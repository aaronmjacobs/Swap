#include "Version.glsl"

uniform sampler2D uTexture;

in vec2 vTexCoord;

out vec4 thresholdedColor;

vec3 threshold(vec3 value)
{
   return max(vec3(0.0), smoothstep(vec3(0.75), vec3(1.25), value) * value);
}

void main()
{
   vec3 color = texture(uTexture, vTexCoord).rgb;

   thresholdedColor = vec4(threshold(color), 1.0);
}
