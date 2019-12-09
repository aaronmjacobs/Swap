#include "Version.glsl"

#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct DirectionalLight
{
   vec3 color;
   vec3 direction;

   bool castShadows;
   mat4 worldToShadow;
   sampler2DShadow shadowMap;
};

struct PointLight
{
   vec3 color;
   vec3 position;
   float radius;

   bool castShadows;
   vec2 nearFar;
   samplerCubeShadow shadowMap;
};

struct SpotLight
{
   vec3 color;
   vec3 direction;
   vec3 position;
   float radius;
   float beamAngle;
   float cutoffAngle;

   bool castShadows;
   mat4 worldToShadow;
   sampler2DShadow shadowMap;
};

struct LightingParams
{
   vec3 diffuseColor;
   vec3 specularColor;
   float shininess;
   float ambientOcclusion;
   float alpha;

   vec3 surfacePosition;
   vec3 surfaceNormal;

   vec3 cameraPosition;
};

float calcAttenuation(vec3 toLight, float radius)
{
   const float kMinLightBrightness = 0.01;

   float multiplier = 1.0 / (radius * radius * kMinLightBrightness);
   float squaredDist = dot(toLight, toLight);

   return 1.0 / (1.0 + squaredDist * multiplier);
}

float linearizeDepth(float normalizedDepth, float near, float far)
{
   float depthNDC = normalizedDepth * 2.0 - 1.0;
   return (2.0 * near * far) / (near + far - depthNDC * (far - near));
}

float normalizeDepth(float linearDepth, float near, float far)
{
   float depthNDC = ((near + far) - (2.0 * near * far) / linearDepth) / (far - near);
   return (depthNDC + 1.0) * 0.5;
}

float sampleShadowMap(sampler2DShadow shadowMap, vec3 shadowCoords)
{
   const float kBias = 0.001;

   float visibility = 0.0;
   vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

   for (int x = -1; x <= 1; ++x)
   {
       for (int y = -1; y <= 1; ++y)
       {
           visibility += texture(shadowMap, vec3(shadowCoords.xy + vec2(x, y) * texelSize, shadowCoords.z - kBias));
       }
   }

   return visibility / 9.0;
}

vec3 calcAmbient(vec3 lightColor, vec3 diffuseColor, float ambientOcclusion)
{
   return lightColor * diffuseColor * ambientOcclusion * 0.1;
}

vec3 calcDiffuse(vec3 lightColor, vec3 diffuseColor, float ambientOcclusion, vec3 surfaceNormal, vec3 toLightDirection)
{
   float diffuseAmount = max(0.0, dot(surfaceNormal, toLightDirection));

   return lightColor * (diffuseColor * diffuseAmount) * ambientOcclusion * 0.9;
}

vec3 calcSpecular(vec3 lightColor, vec3 specularColor, float shininess, vec3 surfacePosition, vec3 surfaceNormal, vec3 toLightDirection, vec3 cameraPosition)
{
   vec3 toCamera = normalize(cameraPosition - surfacePosition);
   vec3 reflection = reflect(-toLightDirection, surfaceNormal);

   float specularBase = max(0.0, dot(toCamera, reflection));
   float specularAmount = pow(specularBase, max(1.0, shininess));

   return lightColor * (specularColor * specularAmount);
}

vec3 calcDirectionalLighting(DirectionalLight directionalLight, LightingParams lightingParams)
{
   vec3 ambient = calcAmbient(directionalLight.color, lightingParams.diffuseColor, lightingParams.ambientOcclusion);
   vec3 diffuse = calcDiffuse(directionalLight.color, lightingParams.diffuseColor, lightingParams.ambientOcclusion, lightingParams.surfaceNormal, -directionalLight.direction);
   vec3 specular = calcSpecular(directionalLight.color, lightingParams.specularColor, lightingParams.shininess, lightingParams.surfacePosition, lightingParams.surfaceNormal, -directionalLight.direction, lightingParams.cameraPosition);

   float visibility = 1.0;
   if (directionalLight.castShadows)
   {
      vec4 shadowPosition = directionalLight.worldToShadow * vec4(lightingParams.surfacePosition, 1.0);
      vec3 shadowCoords = (shadowPosition.xyz / shadowPosition.w) * 0.5 + 0.5;

      visibility = sampleShadowMap(directionalLight.shadowMap, shadowCoords);
   }

   return ambient + (diffuse + specular) * visibility;
}

vec3 calcPointLighting(PointLight pointLight, LightingParams lightingParams)
{
   vec3 toLight = pointLight.position - lightingParams.surfacePosition;
   vec3 toLightDirection = normalize(toLight);

   vec3 ambient = calcAmbient(pointLight.color, lightingParams.diffuseColor, lightingParams.ambientOcclusion);
   vec3 diffuse = calcDiffuse(pointLight.color, lightingParams.diffuseColor, lightingParams.ambientOcclusion, lightingParams.surfaceNormal, toLightDirection);
   vec3 specular = calcSpecular(pointLight.color, lightingParams.specularColor, lightingParams.shininess, lightingParams.surfacePosition, lightingParams.surfaceNormal, toLightDirection, lightingParams.cameraPosition);

   float attenuation = calcAttenuation(toLight, pointLight.radius);

   // TODO PCF
   float visibility = 1.0;
   if (pointLight.castShadows)
   {
      const float kBias = 0.001;

      float computedDepth = normalizeDepth(length(toLight), pointLight.nearFar.x, pointLight.nearFar.y);
      visibility = texture(pointLight.shadowMap, vec4(toLightDirection, computedDepth - kBias));
   }

   return (ambient + (diffuse + specular) * visibility) * attenuation;
}

vec3 calcSpotLighting(SpotLight spotLight, LightingParams lightingParams)
{
   vec3 toLight = spotLight.position - lightingParams.surfacePosition;
   vec3 toLightDirection = normalize(toLight);

   vec3 ambient = calcAmbient(spotLight.color, lightingParams.diffuseColor, lightingParams.ambientOcclusion);
   vec3 diffuse = calcDiffuse(spotLight.color, lightingParams.diffuseColor, lightingParams.ambientOcclusion, lightingParams.surfaceNormal, toLightDirection);
   vec3 specular = calcSpecular(spotLight.color, lightingParams.specularColor, lightingParams.shininess, lightingParams.surfacePosition, lightingParams.surfaceNormal, toLightDirection, lightingParams.cameraPosition);

   float attenuation = calcAttenuation(toLight, spotLight.radius);

   float spotAngle = acos(dot(spotLight.direction, -toLightDirection));
   float clampedAngle = clamp(spotAngle, spotLight.beamAngle, spotLight.cutoffAngle);
   float spotMultiplier = 1.0 - ((clampedAngle - spotLight.beamAngle) / (spotLight.cutoffAngle - spotLight.beamAngle));

   float visibility = 1.0;
   if (spotLight.castShadows)
   {
      vec4 shadowPosition = spotLight.worldToShadow * vec4(lightingParams.surfacePosition, 1.0);
      vec3 shadowCoords = (shadowPosition.xyz / shadowPosition.w) * 0.5 + 0.5;

      visibility = sampleShadowMap(spotLight.shadowMap, shadowCoords);
   }

   return (ambient + (diffuse + specular) * visibility) * attenuation * spotMultiplier;
}
