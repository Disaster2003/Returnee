/**
* @file down_sampling.frag
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location = 1) in vec2 inTexcoord;

// �e�N�X�`��
layout(binding = 0) uniform sampler2D texColor;

// �o�͂���F�f�[�^
out vec4 outColor;

// �G���g���[�|�C���g
void main()
{
	// 3x3�̏k���ڂ����������s��
	vec2 texel = 1.0 / vec2(textureSize(texColor, 0)); // �e�N�Z���T�C�Y���v�Z����
	outColor  = texture(texColor, inTexcoord) * 4;
	outColor += texture(texColor, inTexcoord + vec2(-texel.x, texel.y));
	outColor += texture(texColor, inTexcoord + vec2( texel.x, texel.y));
	outColor += texture(texColor, inTexcoord + vec2(-texel.x,-texel.y));
	outColor += texture(texColor, inTexcoord + vec2( texel.x,-texel.y));
	outColor *= 1.0 / 8.0; // ���ω�������
}