/**
* @file standard.vert
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=0) in vec3 inPosition; // ���_���W
layout(location=1) in vec2 inTexcoord; // �e�N�X�`�����W
layout(location=2) in vec3 inNormal;   // �@���x�N�g��
layout(location=3) in vec4 inTangent;  // �^���W�F���g�x�N�g��

// �V�F�[�_����̏o��
layout(location=0) out vec3 outPosition; // ���[���h���W
layout(location=1) out vec2 outTexcoord; // �e�N�X�`�����W
layout(location=2) out mat3 outTBN;      // �@���ϊ��s��
layout(location=5) out vec3 outShadowTexcoord; // �V���h�E�e�N�X�`�����W

// �v���O��������̓���
layout(location=0) uniform mat4 transformMatrix;
layout(location=1) uniform mat3 normalMatrix;

// �A�X�y�N�g��Ǝ���p�ɂ��g�嗦
layout(location=3) uniform vec2 aspectRatioAndScaleFov;

layout(location=4) uniform vec3 cameraPosition; // �J�����̈ʒu
layout(location=5) uniform vec2 cameraSinCosY;	// �J������Y����]
layout(location=6) uniform vec2 cameraSinCosX;	// �J������X����]

layout(location=10) uniform mat4 shadowTextureMatrix; // �V���h�E�e�N�X�`���s��
layout(location=11) uniform float shadowNormalOffset; // ���W��@�������ɂ��炷��

vec3 RotateXY(vec3 v, vec2 sinCosX, vec2 sinCosY)
{
	// X����]
	v.zy = vec2(v.z * sinCosX.y + v.y * sinCosX.x, -v.z * sinCosX.x + v.y * sinCosX.y);

	// Y����]
	v.xz = vec2(v.x * sinCosY.y + v.z * sinCosY.x, -v.x * sinCosY.x + v.z * sinCosY.y);

	return v;
}

// �G���g���[�|�C���g
void main()
{
	outTexcoord = inTexcoord;

	// ���[�J�����W�n���烏�[���h���W�n�ɕϊ�
	gl_Position = transformMatrix * vec4(inPosition, 1);
	outPosition = gl_Position.xyz;

	// ���[���h�@�����v�Z����
	outTBN[0] = normalMatrix * inTangent.xyz;
	outTBN[2] = normalMatrix * inNormal;
	outTBN[1] = inTangent.w * cross(outTBN[2], outTBN[0]);

	// �V���h�E�e�N�X�`�����W���v�Z����
	outShadowTexcoord = outPosition + outTBN[2] * shadowNormalOffset;
	outShadowTexcoord = vec3(shadowTextureMatrix * vec4(outShadowTexcoord, 1));

	// ���[���h���W�n����N���b�v���W�n�ɕϊ�����
	// x = x * cos + z * sin
	// z = x * -sin + z * cos
	vec3 pos = gl_Position.xyz - cameraPosition;
	float cameraSinY = cameraSinCosY.x;
	float cameraCosY = cameraSinCosY.y;
	gl_Position.x = pos.x * cameraCosY + pos.z * cameraSinY;
	gl_Position.y = pos.y;
	gl_Position.z = pos.x * -cameraSinY + pos.z * cameraCosY;

	// y = z * -sin + y * cos
	// z = z *  cos + y * sin
	pos = gl_Position.xyz;
	float cameraSinX = cameraSinCosX.x;
	float cameraCosX = cameraSinCosX.y;
	gl_Position.x = pos.x;
	gl_Position.y = pos.y * cameraCosX + pos.z * -cameraSinX;
	gl_Position.z = pos.z * cameraCosX + pos.y * cameraSinX;

	// �r���[���W�n����N���b�v���W�n�ɕϊ�����
	gl_Position.xy *= aspectRatioAndScaleFov;

	// �[�x�l�̌v�Z���ʂ�-1�`+1�ɂȂ�悤�ȃp�����[�^A,B���v�Z����
	const float near = 0.35;
	const float far = 1000;
	const float A = -2 * far * near / (far - near);
	const float B = (far + near) / (far - near);

	// ���ߖ@��L���ɂ���
	gl_Position.w = -gl_Position.z;
	gl_Position.z = -gl_Position.z * B + A; // �[�x�l��␳
}