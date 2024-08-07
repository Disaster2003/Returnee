#version 450

layout(binding=0) uniform sampler2D texDepth;

out float outColor;

layout(location=200) uniform int mipLevel; // コピー元のミップレベル

void main()
{
	ivec2 uv = ivec2(gl_FragCoord.xy) * 2;
	float d1 = texelFetch(texDepth, uv + ivec2(0, 0), mipLevel).x;
	float d2 = texelFetch(texDepth, uv + ivec2(0, 1), mipLevel).x;
	float d3 = texelFetch(texDepth, uv + ivec2(1, 1), mipLevel).x;
	float d4 = texelFetch(texDepth, uv + ivec2(1, 0), mipLevel).x;
	outColor = min(min(d1, d2), min(d3, d4));
}