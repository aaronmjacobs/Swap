#include "ForwardCommon.glsl"

struct Material
{
#if WITH_DIFFUSE_TEXTURE
   sampler2D diffuseTexture;
#else
   vec3 diffuseColor;
#endif

#if WITH_SPECULAR_TEXTURE
   sampler2D specularTexture;
#else
   vec3 specularColor;
#endif

   float shininess;

   vec3 emissiveColor;

#if WITH_NORMAL_TEXTURE
   sampler2D normalTexture;
#endif
};

struct DirectionalLight
{
   vec3 color;
   vec3 direction;
};

struct PointLight
{
   vec3 color;
   vec3 position;
   float radius;
};

struct SpotLight
{
   vec3 color;
   vec3 direction;
   vec3 position;
   float radius;
   float beamAngle;
   float cutoffAngle;
};

const int kMaxDirectionalLights = 2;
const int kMaxPointLights = 8;
const int kMaxSpotLights = 8;

uniform Material uMaterial;

uniform DirectionalLight uDirectionalLights[kMaxDirectionalLights];
uniform int uNumDirectionalLights;

uniform PointLight uPointLights[kMaxPointLights];
uniform int uNumPointLights;

uniform SpotLight uSpotLights[kMaxSpotLights];
uniform int uNumSpotLights;

uniform vec3 uCameraPos;

in vec3 vPosition;

#if VARYING_NORMAL
in vec3 vNormal;
#endif

#if VARYING_TEX_COORD
in vec2 vTexCoord;
#endif

#if VARYING_TBN
in mat3 vTBN;
#endif

out vec4 color;

vec3 calcDiffuseColor()
{
#if WITH_DIFFUSE_TEXTURE
   return texture(uMaterial.diffuseTexture, vTexCoord).rgb;
#else
   return uMaterial.diffuseColor;
#endif
}

vec3 calcSpecularColor()
{
#if WITH_SPECULAR_TEXTURE
   return texture(uMaterial.specularTexture, vTexCoord).rgb;
#else
   return uMaterial.specularColor;
#endif
}

vec3 calcEmissiveColor()
{
   return uMaterial.emissiveColor;
}

vec3 calcSurfaceNormal()
{
   vec3 surfaceNormal;

#if WITH_NORMAL_TEXTURE
   vec3 tangentSpaceNormal = texture(uMaterial.normalTexture, vTexCoord).rgb * 2.0 - 1.0;
   surfaceNormal = normalize(vTBN * tangentSpaceNormal);
#else
   surfaceNormal = vNormal;
#endif

   return normalize(surfaceNormal);
}

float calcAttenuation(vec3 toLight, float radius)
{
   const float kMinLightBrightness = 0.01;

   float multiplier = 1.0 / (radius * radius * kMinLightBrightness);
   float squaredDist = dot(toLight, toLight);

   return 1.0 / (1.0 + squaredDist * multiplier);
}

vec3 calcAmbient(vec3 lightColor, vec3 diffuseColor)
{
   return lightColor * diffuseColor * 0.05;
}

vec3 calcDiffuse(vec3 lightColor, vec3 diffuseColor, vec3 surfaceNormal, vec3 toLightDirection)
{
   float diffuseAmount = max(0.0, dot(surfaceNormal, toLightDirection));

   return lightColor * (diffuseColor * diffuseAmount);
}

vec3 calcSpecular(vec3 lightColor, vec3 specularColor, vec3 surfaceNormal, vec3 toLightDirection)
{
   vec3 toCamera = normalize(uCameraPos - vPosition);
   vec3 reflection = reflect(-toLightDirection, surfaceNormal);

   float specularBase = max(0.0, dot(toCamera, reflection));
   float specularAmount = pow(specularBase, max(1.0, uMaterial.shininess));

   return lightColor * (specularColor * specularAmount);
}

vec3 calcDirectionalLighting(DirectionalLight directionalLight, vec3 diffuseColor, vec3 specularColor, vec3 surfaceNormal)
{
   vec3 ambient = calcAmbient(directionalLight.color, diffuseColor);
   vec3 diffuse = calcDiffuse(directionalLight.color, diffuseColor, surfaceNormal, -directionalLight.direction);
   vec3 specular = calcSpecular(directionalLight.color, specularColor, surfaceNormal, -directionalLight.direction);

   return ambient + diffuse + specular;
}

vec3 calcPointLighting(PointLight pointLight, vec3 diffuseColor, vec3 specularColor, vec3 surfaceNormal)
{
   vec3 toLight = pointLight.position - vPosition;
   vec3 toLightDirection = normalize(toLight);

   vec3 ambient = calcAmbient(pointLight.color, diffuseColor);
   vec3 diffuse = calcDiffuse(pointLight.color, diffuseColor, surfaceNormal, toLightDirection);
   vec3 specular = calcSpecular(pointLight.color, specularColor, surfaceNormal, toLightDirection);

   float attenuation = calcAttenuation(toLight, pointLight.radius);

   return (ambient + diffuse + specular) * attenuation;
}

vec3 calcSpotLighting(SpotLight spotLight, vec3 diffuseColor, vec3 specularColor, vec3 surfaceNormal)
{
   vec3 toLight = spotLight.position - vPosition;
   vec3 toLightDirection = normalize(toLight);

   vec3 ambient = calcAmbient(spotLight.color, diffuseColor);
   vec3 diffuse = calcDiffuse(spotLight.color, diffuseColor, surfaceNormal, toLightDirection);
   vec3 specular = calcSpecular(spotLight.color, specularColor, surfaceNormal, toLightDirection);

   float attenuation = calcAttenuation(toLight, spotLight.radius);

   float spotAngle = acos(dot(spotLight.direction, -toLightDirection));
   float clampedAngle = clamp(spotAngle, spotLight.beamAngle, spotLight.cutoffAngle);
   float spotMultiplier = 1.0 - ((clampedAngle - spotLight.beamAngle) / (spotLight.cutoffAngle - spotLight.beamAngle));

   return (ambient + diffuse + specular) * attenuation * spotMultiplier;
}

vec3 calcLighting()
{
   vec3 lighting = vec3(0.0);

   vec3 diffuseColor = calcDiffuseColor();
   vec3 specularColor = calcSpecularColor();
   vec3 surfaceNormal = calcSurfaceNormal();

   for (int i = 0; i < uNumDirectionalLights; ++i)
   {
      lighting += calcDirectionalLighting(uDirectionalLights[i], diffuseColor, specularColor, surfaceNormal);
   }

   for (int i = 0; i < uNumPointLights; ++i)
   {
      lighting += calcPointLighting(uPointLights[i], diffuseColor, specularColor, surfaceNormal);
   }

   for (int i = 0; i < uNumSpotLights; ++i)
   {
      lighting += calcSpotLighting(uSpotLights[i], diffuseColor, specularColor, surfaceNormal);
   }

   lighting += calcEmissiveColor();

   return lighting;
}

void main()
{
   color = vec4(calcLighting(), 1.0);
}
