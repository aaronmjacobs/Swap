#include "Version.glsl"

#include "ForwardCommon.glsl"
#include "LightingCommon.glsl"
#include "MaterialCommon.glsl"

uniform Material uMaterial;

const int kMaxDirectionalLights = 2;
const int kMaxPointLights = 8;
const int kMaxSpotLights = 8;

uniform DirectionalLight uDirectionalLights[kMaxDirectionalLights];
uniform int uNumDirectionalLights;

uniform PointLight uPointLights[kMaxPointLights];
uniform int uNumPointLights;

uniform SpotLight uSpotLights[kMaxSpotLights];
uniform int uNumSpotLights;

uniform vec3 uCameraPos;
uniform vec4 uViewport;

uniform sampler2D uAmbientOcclusion;

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

layout(location = 0) out vec4 color;

LightingParams calcLightingParams(MaterialSampleParams materialSampleParams)
{
   vec2 texCoord = gl_FragCoord.xy * uViewport.zw;

   LightingParams lightingParams;

   lightingParams.diffuseColor = calcMaterialDiffuseColor(uMaterial, materialSampleParams).rgb;
   lightingParams.specularColor = calcMaterialSpecularColor(uMaterial, materialSampleParams).rgb;
   lightingParams.shininess = calcMaterialShininess(uMaterial, materialSampleParams);
   lightingParams.ambientOcclusion = texture(uAmbientOcclusion, texCoord).r;

   lightingParams.surfacePosition = vPosition;
   lightingParams.surfaceNormal = calcMaterialSurfaceNormal(uMaterial, materialSampleParams);

   lightingParams.cameraPosition = uCameraPos;

   return lightingParams;
}

vec3 calcLighting()
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

   LightingParams lightingParams = calcLightingParams(materialSampleParams);

   vec3 lighting = vec3(0.0);

   for (int i = 0; i < uNumDirectionalLights; ++i)
   {
      lighting += calcDirectionalLighting(uDirectionalLights[i], lightingParams);
   }

   for (int i = 0; i < uNumPointLights; ++i)
   {
      lighting += calcPointLighting(uPointLights[i], lightingParams);
   }

   for (int i = 0; i < uNumSpotLights; ++i)
   {
      lighting += calcSpotLighting(uSpotLights[i], lightingParams);
   }

   lighting += calcMaterialEmissiveColor(uMaterial, materialSampleParams);

   return lighting;
}

void main()
{
   color = vec4(calcLighting(), 1.0);
}
