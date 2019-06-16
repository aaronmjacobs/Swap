#include "Version.glsl"

#include "LightingCommon.glsl"
#include "ViewCommon.glsl"

uniform sampler2D uPosition;
uniform sampler2D uNormalShininess;
uniform sampler2D uAlbedo;
uniform sampler2D uSpecular;
uniform sampler2D uAmbientOcclusion;

#if LIGHT_TYPE == DIRECTIONAL_LIGHT
uniform DirectionalLight uDirectionalLight;
#elif LIGHT_TYPE == POINT_LIGHT
uniform PointLight uPointLight;
#elif LIGHT_TYPE == SPOT_LIGHT
uniform SpotLight uSpotLight;
#endif

layout(location = 0) out vec4 color;

LightingParams sampleLightingParams()
{
   vec2 texCoord = gl_FragCoord.xy * uViewport.zw;
   vec4 normalShininess = texture(uNormalShininess, texCoord);

   LightingParams lightingParams;

   lightingParams.diffuseColor = texture(uAlbedo, texCoord).rgb;
   lightingParams.specularColor = texture(uSpecular, texCoord).rgb;
   lightingParams.shininess = normalShininess.a;
   lightingParams.ambientOcclusion = texture(uAmbientOcclusion, texCoord).r;
   lightingParams.alpha = 1.0;

   lightingParams.surfacePosition = texture(uPosition, texCoord).rgb;
   lightingParams.surfaceNormal = normalShininess.rgb;

   lightingParams.cameraPosition = uCameraPosition;

   return lightingParams;
}

vec4 calcLighting()
{
   LightingParams lightingParams = sampleLightingParams();

   vec3 lighting = vec3(0.0);

#if LIGHT_TYPE == DIRECTIONAL_LIGHT
   lighting += calcDirectionalLighting(uDirectionalLight, lightingParams);
#elif LIGHT_TYPE == POINT_LIGHT
   lighting += calcPointLighting(uPointLight, lightingParams);
#elif LIGHT_TYPE == SPOT_LIGHT
   lighting += calcSpotLighting(uSpotLight, lightingParams);
#endif

   return vec4(lighting, 1.0);
}

void main()
{
   color = calcLighting();
}
