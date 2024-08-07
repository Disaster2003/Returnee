#version 450

layout(binding=0) uniform sampler2D texColor; // �Օ���
layout(binding=1) uniform sampler2D texDepth; // ���`�[�x�l

out vec4 outColor;

// �ڂ����W���̔z��
const float gaussian[5] = 
  { 0.153170, 0.444893, 0.422649, 0.392902, 0.362970 };

const int blurRadius = 4;        // �ڂ������a
const int blurScale = 2;         // �ڂ������a�̊g�嗦
const float edgeDistance = 0.15; // �������̂Ƃ݂Ȃ��[�x��(�P��=m)
const float edgeSharpness = 1 / edgeDistance; // ���̗̂֊s���ڂ����Ȃ����߂̌W��
const float far = 1000;          // �t�@�[���ʂ̐[�x�l

// �G���g���[�|�C���g
void main()
{
    // �s�N�Z���̎Օ����Ɛ[�x�l���擾����
    ivec2 uv = ivec2(gl_FragCoord.xy);
    outColor = texelFetch(texColor, uv, 0);
    float depth = texelFetch(texDepth, uv, 0).x;

    // �[�x�l���t�@�[���ʈȏ�̏ꍇ�A���̃s�N�Z���͔w�i�Ȃ̂łڂ����Ȃ�
    if (depth >= far)
        return;

    // �o�C���e�����E�t�B���^�����s����
    outColor *= gaussian[0] * gaussian[0];
    float totalWeight = gaussian[0] * gaussian[0];
    for (int y = -blurRadius; y <= blurRadius; ++y)
        for (int x = -blurRadius; x <= blurRadius; ++x)
        {
            // ���S�͎擾�ς݂Ȃ̂Ŗ�������
            if (y == 0 && x == 0)
              continue;

            // �T���v���_�̃E�F�C�g(�d�v�x)���擾����
            float weight = gaussian[abs(x)] * gaussian[abs(y)];

            // ���S�Ƃ̐[�x�l�̍��ɂ���ĕ��̂̋��E�𔻒肵�A�E�F�C�g�𒲐�����
            ivec2 sampleUV = uv + ivec2(x, y) * blurScale;
            float sampleDepth = texelFetch(texDepth, sampleUV, 0).x;
            weight *= max(0.0, 1.0 - edgeSharpness * abs(sampleDepth - depth));

            // �Օ����ƍ��v�E�F�C�g���X�V����
            outColor += texelFetch(texColor, sampleUV, 0) * weight;
            totalWeight += weight;
        }

    outColor /= totalWeight;
}