#version 450

layout(location=1) in vec2 inTexcoord;

layout(binding=0) uniform sampler2D texColor;

out vec4 outColor;

void main()
{
	outColor = texture(texColor, inTexcoord);
}