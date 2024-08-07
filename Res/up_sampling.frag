/**
* @file up_sampling.frag
*/
#version 450

// �V�F�[�_�ւ̓���
layout(location=1) in vec2 inTexcoord;

// �e�N�X�`��
layout(binding=0) uniform sampler2D texColor;

// �o�͂���F�f�[�^
out vec4 outColor;

// �G���g���[�|�C���g
void main()
{
  // �g��ڂ����������s��
  vec2 texel = 1.0 / vec2(textureSize(texColor, 0)); // �e�N�Z���T�C�Y���v�Z
  outColor = texture(texColor, inTexcoord) * 3;
  outColor += texture(texColor, inTexcoord + vec2(-texel.x, texel.y)) * 2;
  outColor += texture(texColor, inTexcoord + vec2( texel.x, texel.y)) * 2;
  outColor += texture(texColor, inTexcoord + vec2( texel.x,-texel.y)) * 2;
  outColor += texture(texColor, inTexcoord + vec2(-texel.x,-texel.y)) * 2;

  outColor += texture(texColor, inTexcoord + vec2(-texel.x * 2, 0));
  outColor += texture(texColor, inTexcoord + vec2( texel.x * 2, 0));
  outColor += texture(texColor, inTexcoord + vec2( 0,-texel.y * 2));
  outColor += texture(texColor, inTexcoord + vec2( 0, texel.y * 2));
  outColor *= 1.0 / 15.0; // ���ω�
}