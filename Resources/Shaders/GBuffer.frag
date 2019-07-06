#include "Version.glsl"

#include "GBufferCommon.glsl"
#include "MaterialCommon.glsl"

uniform Material uMaterial;

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

layout(location = 0) out vec3 position;
layout(location = 1) out vec4 normalShininess;
layout(location = 2) out vec4 albedo;
layout(location = 3) out vec4 specular;
layout(location = 4) out vec3 emissive;

void main()
{
   MaterialSampleParams materialSampleParams;
#if VARYING_TEX_COORD
   materialSampleParams.texCoord = vTexCoord;
#endif
#if WITH_NORMAL_TEXTURE
   materialSampleParams.tbn = vTBN;
#else
   materialSampleParams.normal = vNormal;
#endif

   position = vPosition;
   normalShininess = vec4(calcMaterialSurfaceNormal(uMaterial, materialSampleParams), calcMaterialShininess(uMaterial, materialSampleParams));
   albedo = calcMaterialDiffuseColor(uMaterial, materialSampleParams);
   specular = calcMaterialSpecularColor(uMaterial, materialSampleParams);
   emissive = calcMaterialEmissiveColor(uMaterial, materialSampleParams);
}
