#include "Version.glsl"

#include "FramebufferCommon.glsl"
#include "ViewCommon.glsl"

#default WITH_POSITION_BUFFER 1

#if WITH_POSITION_BUFFER
uniform sampler2D uPosition;
#else
uniform sampler2D uDepth;
#endif // WITH_POSITION_BUFFER

uniform sampler2D uNormal;
uniform sampler2D uNoise;

uniform vec3 uSamples[SSAO_NUM_SAMPLES];

in vec2 vTexCoord;

out float ambientOcclusion;

vec3 loadPosition(vec2 texCoord)
{
#if WITH_POSITION_BUFFER
   return texture(uPosition, texCoord).xyz;
#else
   float depth = texture(uDepth, texCoord).r;
   float z = depth * 2.0 - 1.0;

   vec4 clipPosition = vec4(texCoord * 2.0 - 1.0, z, 1.0);
   vec4 viewPosition = uClipToView * clipPosition;
   viewPosition /= viewPosition.w;

   vec4 worldPosition = uViewToWorld * viewPosition;
   return worldPosition.xyz;
#endif
}

void main()
{
   vec2 noiseScale = uFramebufferSize.xy / textureSize(uNoise, 0);

   vec3 position = (uWorldToView * vec4(loadPosition(vTexCoord), 1.0)).xyz;
   vec3 normal = (uWorldToView * vec4(texture(uNormal, vTexCoord).xyz, 0.0)).xyz;
   vec3 randomVec = texture(uNoise, vTexCoord * noiseScale).xyz;

   vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
   vec3 bitangent = cross(normal, tangent);
   mat3 tbn = mat3(tangent, bitangent, normal);

   ambientOcclusion = 0.0;
   for (int i = 0; i < SSAO_NUM_SAMPLES; ++i)
   {
      float radius = 1.0;
      vec3 samplePosition = position + (tbn * uSamples[i]) * radius;

      vec4 offset = uViewToClip * vec4(samplePosition, 1.0);
      offset.xyz = (offset.xyz / offset.w) * 0.5 + 0.5;
      float sampleDepth = (uWorldToView * vec4(loadPosition(offset.xy), 1.0)).z;

      float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - sampleDepth));
      float bias = 0.025;
      ambientOcclusion += (sampleDepth >= samplePosition.z + bias ? 1.0 : 0.0) * rangeCheck;
   }

   ambientOcclusion = pow(1.0 - (ambientOcclusion / SSAO_NUM_SAMPLES), 2.0);
}
