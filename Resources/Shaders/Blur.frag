#include "Version.glsl"

#include "FramebufferCommon.glsl"

uniform sampler2D uTexture;

out vec4 blurredColor;

void main()
{
   const float kOffsets[3] = float[](0.0, 1.3846153846, 3.2307692308);
   const float kWeights[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);
   const float kOffsetScale = 1.0;

   vec2 pixelToUV = uFramebufferSize.zw;

   blurredColor = texture(uTexture, gl_FragCoord.xy  * pixelToUV) * kWeights[0];

   for (int i = 1; i < 3; ++i)
   {
#if HORIZONTAL
      vec2 offset = vec2(kOffsets[i] * kOffsetScale, 0.0);
#else
      vec2 offset = vec2(0.0, kOffsets[i] * kOffsetScale);
#endif

      blurredColor += texture(uTexture, (gl_FragCoord.xy + offset) * pixelToUV) * kWeights[i];
      blurredColor += texture(uTexture, (gl_FragCoord.xy - offset) * pixelToUV) * kWeights[i];
   }
}
