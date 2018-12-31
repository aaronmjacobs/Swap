#include "Version.glsl"

#include "MaterialDefines.glsl"

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

struct MaterialSampleParams
{
#if WITH_DIFFUSE_TEXTURE || WITH_SPECULAR_TEXTURE || WITH_NORMAL_TEXTURE
   vec2 texCoord;
#endif

#if WITH_NORMAL_TEXTURE
   mat3 tbn;
#else
   vec3 normal;
#endif
};

vec4 calcMaterialDiffuseColor(Material material, MaterialSampleParams params)
{
#if WITH_DIFFUSE_TEXTURE
   return texture(material.diffuseTexture, params.texCoord);
#else
   return vec4(material.diffuseColor, 1.0);
#endif
}

vec4 calcMaterialSpecularColor(Material material, MaterialSampleParams params)
{
#if WITH_SPECULAR_TEXTURE
   return texture(material.specularTexture, params.texCoord);
#else
   return vec4(material.specularColor, 1.0);
#endif
}

vec3 calcMaterialEmissiveColor(Material material, MaterialSampleParams params)
{
   return material.emissiveColor;
}

float calcMaterialShininess(Material material, MaterialSampleParams params)
{
   return material.shininess;
}

vec3 calcMaterialSurfaceNormal(Material material, MaterialSampleParams params)
{
   vec3 surfaceNormal;

#if WITH_NORMAL_TEXTURE
   vec3 tangentSpaceNormal = texture(material.normalTexture, params.texCoord).rgb * 2.0 - 1.0;
   surfaceNormal = normalize(params.tbn * tangentSpaceNormal);
#else
   surfaceNormal = params.normal;
#endif

   return normalize(surfaceNormal);
}
