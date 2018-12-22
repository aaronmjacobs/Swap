#version 410 core

uniform sampler2D uSceneColorTexture;
uniform sampler2D uSceneDepthTexture;

in vec2 vTexCoord;

out vec4 color;

void main()
{
	vec4 sceneColor = texture(uSceneColorTexture, vTexCoord);
	float sceneDepth = texture(uSceneDepthTexture, vTexCoord).r;

	float depthAmount = 1.0 - ((sceneDepth * 100.0) - 99.0);

	color = vec4(sceneColor.rgb * depthAmount, 1.0);
}
