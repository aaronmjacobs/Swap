#include "Version.glsl"

#include "LightingCommon.glsl"

uniform sampler2D uPosition;
uniform sampler2D uNormalShininess;
uniform sampler2D uAlbedo;
uniform sampler2D uSpecular;
uniform sampler2D uEmissive;

uniform vec3 uCameraPos;
uniform vec4 uViewport;

#if LIGHT_TYPE == DIRECTIONAL_LIGHT
uniform DirectionalLight uDirectionalLight;
#elif LIGHT_TYPE == POINT_LIGHT
uniform PointLight uPointLight;
#elif LIGHT_TYPE == SPOT_LIGHT
uniform SpotLight uSpotLight;
#endif

layout(location = 0) out vec4 color;

vec3 calcLighting()
{
   vec2 texCoord = gl_FragCoord.xy * uViewport.zw;

   vec3 position = texture(uPosition, texCoord).rgb;
   vec4 normalShininess = texture(uNormalShininess, texCoord);
   vec4 albedo = texture(uAlbedo, texCoord);
   vec4 specular = texture(uSpecular, texCoord);
   vec3 emissive = texture(uEmissive, texCoord).rgb;

   vec3 lighting = vec3(0.0);

#if LIGHT_TYPE == DIRECTIONAL_LIGHT
   lighting += calcDirectionalLighting(uDirectionalLight, albedo.rgb, specular.rgb, normalShininess.a, position, normalShininess.rgb, uCameraPos);
#elif LIGHT_TYPE == POINT_LIGHT
   lighting += calcPointLighting(uPointLight, albedo.rgb, specular.rgb, normalShininess.a, position, normalShininess.rgb, uCameraPos);
#elif LIGHT_TYPE == SPOT_LIGHT
   lighting += calcSpotLighting(uSpotLight, albedo.rgb, specular.rgb, normalShininess.a, position, normalShininess.rgb, uCameraPos);
#endif

   lighting += emissive;

   return lighting;
}

void main()
{
   color = vec4(calcLighting(), 1.0);
}
