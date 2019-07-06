#include "Version.glsl"

uniform sampler2D uAmbientOcclusion;

in vec2 vTexCoord;

out float blurredAmbientOcclusion;

void main()
{
	vec2 texelSize = 1.0 / textureSize(uAmbientOcclusion, 0);

	float sum = 0.0;
	for (int y = -2; y < 2; ++y)
	{
		for (int x = -2; x < 2; ++x)
		{
			vec2 offset = vec2(x, y) * texelSize;
			sum += texture(uAmbientOcclusion, vTexCoord + offset).r;
		}
	}

	blurredAmbientOcclusion = sum / 16.0;
}
