/**
* @file up_sampling.frag
*/
#version 450

// シェーダへの入力
layout(location=1) in vec2 inTexcoord;

// テクスチャ
layout(binding=0) uniform sampler2D texColor;

// 出力する色データ
out vec4 outColor;

// エントリーポイント
void main()
{
  // 拡大ぼかし処理を行う
  vec2 texel = 1.0 / vec2(textureSize(texColor, 0)); // テクセルサイズを計算
  outColor = texture(texColor, inTexcoord) * 3;
  outColor += texture(texColor, inTexcoord + vec2(-texel.x, texel.y)) * 2;
  outColor += texture(texColor, inTexcoord + vec2( texel.x, texel.y)) * 2;
  outColor += texture(texColor, inTexcoord + vec2( texel.x,-texel.y)) * 2;
  outColor += texture(texColor, inTexcoord + vec2(-texel.x,-texel.y)) * 2;

  outColor += texture(texColor, inTexcoord + vec2(-texel.x * 2, 0));
  outColor += texture(texColor, inTexcoord + vec2( texel.x * 2, 0));
  outColor += texture(texColor, inTexcoord + vec2( 0,-texel.y * 2));
  outColor += texture(texColor, inTexcoord + vec2( 0, texel.y * 2));
  outColor *= 1.0 / 15.0; // 平均化
}