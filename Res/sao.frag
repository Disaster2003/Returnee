#version 450

layout(location=1) in vec2 inTexcoord;

out float outColor;

layout(binding=0) uniform sampler2D texDepth; // �[�x�e�N�X�`��

layout(location=200) uniform vec4 saoInfo;
#define RADIUS2             (saoInfo.x) // �T���v�����O���a(m)��2��
#define RADIUS_IN_TEXCOORDS (saoInfo.y) // �T���v�����O���a(�e�N�X�`�����W)
#define BIAS                (saoInfo.z) // ���ʂƂ݂Ȃ��R�T�C���l
#define INTENSITY           (saoInfo.w) // AO�̋��x

// NDC���W���r���[���W�ɕϊ����邽�߂̃p�����[�^
layout(location=201) uniform vec2 ndcToView;

const float twoPi = 3.14159265 * 2; // 2��(360�x)
const float samplingCount = 11;     // �T���v����
const float spiralTurns = 7;        // �T���v���_�̉�]��
const int maxMipLevel = 3;          // �ő�~�b�v���x��

// �~�b�v���x��0�ɂȂ�T���v���_�̋���(2^logMipOffset�s�N�Z��)
const int logMipOffset = 3;

// �r���[���W�n�̍��W���v�Z����
vec3 GetViewSpacePosition(vec2 uv, float linearDepth)
{
	// �e�N�X�`�����W����NDC���W�ɕϊ�����
	vec2 ndc = uv * 2 - 1;

	// NDC���W���王�_����̋���1m�̏ꍇ�̎��_���W�ɕϊ�����
	vec2 viewOneMeter = ndc * ndcToView;

	// �������[�x�l�̏ꍇ�̎��_���W�ɕϊ�����
	return vec3(viewOneMeter * linearDepth, -linearDepth);
}

// SAO(Scalable Ambient Obscurance)�ɂ��Օ��������߂�
void main()
{
	// �s�N�Z���̎��_���W�Ɩ@�������߂�
	float depth = textureLod(texDepth, inTexcoord, 0).x;
	vec3 positionVS = GetViewSpacePosition(inTexcoord, depth);
	vec3 normalVS = normalize(cross(dFdx(positionVS), dFdy(positionVS)));

	// �t���O�����g���Ƃɉ�]�̊J�n�p�x�����炷���ƂŌ����ڂ����P����
	const ivec2 iuv = ivec2(gl_FragCoord.xy);
	const float startAngle = mod((3 * iuv.x ^ iuv.y + iuv.x * iuv.y) * 10, twoPi);

	// ���[���h���W�n�ƃX�N���[�����W�n�̃T���v�����O���a
	float radiusTS = RADIUS_IN_TEXCOORDS / depth;
	float pixelsSS = radiusTS * textureSize(texDepth, 0).y;

	float occlusion = 0; // �Օ���
	for (int i = 0; i < samplingCount; ++i)
	{
		// �T���v���_�̊p�x�Ƌ��������߂�
		float ratio = (float(i) + 0.5) * (1.0 / samplingCount);
		float angle = ratio * (spiralTurns * twoPi) + startAngle;
		vec2 unitOffset = vec2(cos(angle), sin(angle)); 

		// �T���v���_�̎��_���W�����߂�
		vec2 uv = inTexcoord + ratio * radiusTS * unitOffset;

		// �����������قǍ����~�b�v���x����I������
		int mipLevel = clamp(findMSB(int(ratio * pixelsSS)) - logMipOffset, 0, maxMipLevel);

		// �T���v���_�̎��_���W�����߂�
		float sampleDepth = textureLod(texDepth, uv, mipLevel).x;
		vec3 samplePositionVS = GetViewSpacePosition(uv, sampleDepth);

		// �T���v���_�ւ̃x�N�g���Ɩ@���̃R�T�C�������߂�
		// �R�T�C����1(����)�ɋ߂��قǎՕ������オ��
		vec3 v = samplePositionVS - positionVS;
		float vn = dot(v, normalVS); // �p�x�ɂ��Օ���

		// �T���v���_�����S�ɋ߂��قǎՕ������オ��
		float vv = dot(v, v);
		float f = max(RADIUS2 - vv, 0); // �����ɂ��Օ���

		// �T���v���_�܂ł̋����ƃR�T�C������AO�����߂�
		occlusion += f * f * f * max((vn - BIAS) / (vv + 0.001), 0);

		// �����܂��͐����ɋ߂��ʂ̃I�N���[�W�����𕽋ω�����B
		if (abs(dFdx(positionVS.z)) < 0.02)
			occlusion -= dFdx(occlusion) * ((iuv.x & 1) - 0.5);
		if (abs(dFdy(positionVS.z)) < 0.02)
			occlusion -= dFdy(occlusion) * ((iuv.y & 1) - 0.5);

		outColor = occlusion;
	}

	// ���ϒl�����߁AAO�̋�������Z����
	occlusion = min(1.0, occlusion / samplingCount * INTENSITY);
}