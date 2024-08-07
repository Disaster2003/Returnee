/**
* @file image_processing.frag
*/
#version 450

layout(location = 1) in vec2 inTexcoord;

out vec4 outColor;

layout(binding = 0) uniform sampler2D texColor;

// �v���O��������̓���
layout(location = 200) uniform mat3 colorMatrix;

// ���ˏ�u���[�̃p�����[�^
// xy=���S���W(NDC), z=����, w=�J�n����
layout(location = 201) uniform vec4 radialBlur;

// �G���g���[�|�C���g
void main()
{
    outColor = texture(texColor, inTexcoord);
    
    // ���ˏ�u���[
    if (radialBlur.z > 0)
    {
        // ���S�֌������x�N�g�����v�Z����
        vec2 v = radialBlur.xy - inTexcoord;
        
        // �u���[�J�n�����ȉ��̏ꍇ�͕����x�N�g����0�ɂ���
        float lenA = length(v);
        float lenB = max(lenA - radialBlur.w, 0);
        v *= lenB / lenA;
        
        v *= radialBlur.z; // �x�N�g���̒����𒲐�
        
        // �x�N�g�������̃s�N�Z���𕡐��T���v�����O���ĉ摜���ڂ���
        outColor.rgb *= 1.0 / 8.0;
        float ratio = 1.0 / 8.0;
        for (float i = 1; i < 8; ++i)
          outColor.rgb += texture(texColor, inTexcoord + v * i).rgb * ratio;
    } // if radialBlur.z
    
    // �F�ϊ��s��ŐF��ς���
    outColor.rgb = colorMatrix * outColor.rgb;
    
    // �K���}�␳���s��
    outColor.rgb = pow(outColor.rgb, vec3(1 / 2.2));
}