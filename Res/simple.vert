/**
* @file simple.vert
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location = 0) in vec3 inPosition; // ���_���W
layout(location = 1) in vec2 inTexcoord; // �e�N�X�`�����W

// �V�F�[�_����̏o��
layout(location = 1) out vec2 outTexcoord; // �e�N�X�`�����W

// �G���g���[�|�C���g
void main()
{
	outTexcoord = inTexcoord;
	gl_Position = vec4(inPosition, 1);
}