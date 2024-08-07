/**
* @file high_pass_filter.frag
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location = 1) in vec2 inTexcoord;

// �e�N�X�`��
layout(binding = 0) uniform sampler2D texColor;

// �v���O��������̓���
// x: ���P�x�Ƃ݂Ȃ����邳(�������l)
// y: �u���[���̋���
layout(location = 202) uniform vec2 highPassFilter;

// �o�͂���F�f�[�^
out vec4 outColor;

// ���邢�����𒊏o����
vec3 GetBrighterPart(in vec2 texcoord)
{
	// RGB�̂��������Ƃ����邢�������s�N�Z���̖��邳�Ƃ���
	vec3 color = texture(texColor, texcoord).rgb;
	float brightness = max(color.r, max(color.g, color.b));
	
	// ���邢�����̔䗦���v�Z����
	float ratio = max(brightness - highPassFilter.x, 0) / max(brightness, 0.001);
	return color * ratio; // ���邢�������v�Z����
}

// �G���g���[�|�C���g
void main()
{
	// �e�N�Z���T�C�Y���v�Z����
	vec2 oneTexel = 1.0 / vec2(textureSize(texColor, 0));
	
	// ���邢�������v�Z���A4x4�s�N�Z���̏k���������s��
	outColor.rgb  = GetBrighterPart(inTexcoord + vec2(-oneTexel.x, oneTexel.y));
	outColor.rgb += GetBrighterPart(inTexcoord + vec2( oneTexel.x, oneTexel.y));
	outColor.rgb += GetBrighterPart(inTexcoord + vec2(-oneTexel.x,-oneTexel.y));
	outColor.rgb += GetBrighterPart(inTexcoord + vec2( oneTexel.x,-oneTexel.y));
	
	// �u���[���̋�������Z����
	outColor.rgb *= (1.0 / 4.0) * highPassFilter.y;
	outColor.a = 1;
}