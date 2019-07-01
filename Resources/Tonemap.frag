#include "Version.glsl"

uniform sampler2D uColorHDR;
uniform sampler2D uBloom;

in vec2 vTexCoord;

out vec4 colorLDR;

vec3 tonemap(vec3 colorHDR)
{
   const float curve = 8.0;

   vec3 clampedColorHDR = max(vec3(0.0), colorHDR);
   vec3 colorLDR = clampedColorHDR - (pow(pow(clampedColorHDR, vec3(curve)) + 1, vec3(1.0 / curve)) - 1.0);

   return colorLDR;
}

void main()
{
   vec3 colorHDR = texture(uColorHDR, vTexCoord).rgb;
   vec3 bloom = texture(uBloom, vTexCoord).rgb * 0.5;

   colorLDR = vec4(tonemap(colorHDR + bloom), 1.0);
}
