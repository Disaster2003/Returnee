/**
* @file shadow_skeletal.vert
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location = 0) in vec3 inPosition; // ���_���W
layout(location = 1) in vec2 inTexcoord; // �e�N�X�`�����W
layout(location = 4) in uvec4 inJoints;  // ���W�ϊ��s��̔ԍ�
layout(location = 5) in vec4 inWeights;  // ���W�ϊ��s��̉e���x

// �V�F�[�_����̏o��
layout(location = 1) out vec2 outTexcoord; // �e�N�X�`�����W

// �v���O��������̓���
layout(location = 0) uniform mat4 transformMatrix;      // ���W�ϊ��s��
layout(location = 2) uniform mat4 viewProjectionMatrix; // �r���[�v���W�F�N�V�����s��

// �֐߃f�[�^�p��SSBO
layout(std430, binding = 0) buffer JointDataBlock
{
    mat4 jointMatrices[]; // �֐߂̍��W�ϊ��s��
};

// �G���g���[�|�C���g
void main()
{
    // �e�N�X�`�����W��ݒ肷��
    outTexcoord = inTexcoord;
    
    // �p���s��ɃE�F�C�g���|���ĉ��Z�������A���f���s����쐬����
    mat4 jointMatrix =
      jointMatrices[inJoints.x] * inWeights.x +
      jointMatrices[inJoints.y] * inWeights.y +
      jointMatrices[inJoints.z] * inWeights.z +
      jointMatrices[inJoints.w] * inWeights.w;
    
    // ���[�J�����W�n���烏�[���h���W�n�ɕϊ�����
    gl_Position = transformMatrix * (jointMatrix * vec4(inPosition, 1));
    
    // ���[���h���W�n����N���b�v���W�n�ɕϊ�����
    gl_Position = viewProjectionMatrix * gl_Position;
}