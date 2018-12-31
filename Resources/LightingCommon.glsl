#include "Version.glsl"

#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

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

vec3 calcSpecular(vec3 lightColor, vec3 specularColor, float shininess, vec3 surfacePosition, vec3 surfaceNormal, vec3 toLightDirection, vec3 cameraPosition)
{
   vec3 toCamera = normalize(cameraPosition - surfacePosition);
   vec3 reflection = reflect(-toLightDirection, surfaceNormal);

   float specularBase = max(0.0, dot(toCamera, reflection));
   float specularAmount = pow(specularBase, max(1.0, shininess));

   return lightColor * (specularColor * specularAmount);
}

vec3 calcDirectionalLighting(DirectionalLight directionalLight, vec3 diffuseColor, vec3 specularColor, float shininess, vec3 surfacePosition, vec3 surfaceNormal, vec3 cameraPosition)
{
   vec3 ambient = calcAmbient(directionalLight.color, diffuseColor);
   vec3 diffuse = calcDiffuse(directionalLight.color, diffuseColor, surfaceNormal, -directionalLight.direction);
   vec3 specular = calcSpecular(directionalLight.color, specularColor, shininess, surfacePosition, surfaceNormal, -directionalLight.direction, cameraPosition);

   return ambient + diffuse + specular;
}

vec3 calcPointLighting(PointLight pointLight, vec3 diffuseColor, vec3 specularColor, float shininess, vec3 surfacePosition, vec3 surfaceNormal, vec3 cameraPosition)
{
   vec3 toLight = pointLight.position - surfacePosition;
   vec3 toLightDirection = normalize(toLight);

   vec3 ambient = calcAmbient(pointLight.color, diffuseColor);
   vec3 diffuse = calcDiffuse(pointLight.color, diffuseColor, surfaceNormal, toLightDirection);
   vec3 specular = calcSpecular(pointLight.color, specularColor, shininess, surfacePosition, surfaceNormal, toLightDirection, cameraPosition);

   float attenuation = calcAttenuation(toLight, pointLight.radius);

   return (ambient + diffuse + specular) * attenuation;
}

vec3 calcSpotLighting(SpotLight spotLight, vec3 diffuseColor, vec3 specularColor, float shininess, vec3 surfacePosition, vec3 surfaceNormal, vec3 cameraPosition)
{
   vec3 toLight = spotLight.position - surfacePosition;
   vec3 toLightDirection = normalize(toLight);

   vec3 ambient = calcAmbient(spotLight.color, diffuseColor);
   vec3 diffuse = calcDiffuse(spotLight.color, diffuseColor, surfaceNormal, toLightDirection);
   vec3 specular = calcSpecular(spotLight.color, specularColor, shininess, surfacePosition, surfaceNormal, toLightDirection, cameraPosition);

   float attenuation = calcAttenuation(toLight, spotLight.radius);

   float spotAngle = acos(dot(spotLight.direction, -toLightDirection));
   float clampedAngle = clamp(spotAngle, spotLight.beamAngle, spotLight.cutoffAngle);
   float spotMultiplier = 1.0 - ((clampedAngle - spotLight.beamAngle) / (spotLight.cutoffAngle - spotLight.beamAngle));

   return (ambient + diffuse + specular) * attenuation * spotMultiplier;
}
