#version 450

out float outColor;

layout(binding=0) uniform sampler2D texDepth;

/**
* �[�x�o�b�t�@�̒l����r���[��Ԃ�Z�l�𕜌�
*/
void main()
{
	// 2x2�e�N�Z���̂����A�ł����_�ɋ߂��l��I������
	ivec2 uv = ivec2(gl_FragCoord.xy) * 2;
	float d1 = texelFetch(texDepth, uv + ivec2(0, 0), 0).x;
	float d2 = texelFetch(texDepth, uv + ivec2(0, 1), 0).x;
	float d3 = texelFetch(texDepth, uv + ivec2(1, 1), 0).x;
	float d4 = texelFetch(texDepth, uv + ivec2(1, 0), 0).x;
	float depth = min(min(d1, d2), min(d3, d4));
	
	// �[�x�l����`�ɖ߂�
	const float near = 0.35;
	const float far = 1000;
	const float A = -2 * far * near / (far - near);
	const float B = (far + near) / (far - near);
	depth = 2 * depth - 1;
	outColor = A / (depth - B);
}