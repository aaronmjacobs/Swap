#include "Version.glsl"

#include "ForwardCommon.glsl"
#include "MaterialCommon.glsl"

uniform Material uMaterial;

#if VARYING_NORMAL
in vec3 vNormal;
#endif

#if VARYING_TEX_COORD
in vec2 vTexCoord;
#endif

#if VARYING_TBN
in mat3 vTBN;
#endif

layout(location = 0) out vec3 normal;

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

   normal = calcMaterialSurfaceNormal(uMaterial, materialSampleParams);
}
