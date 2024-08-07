/**
* @file down_sampling.frag
*/
#version 450

// シェーダへの入力
layout(location = 1) in vec2 inTexcoord;

// テクスチャ
layout(binding = 0) uniform sampler2D texColor;

// 出力する色データ
out vec4 outColor;

// エントリーポイント
void main()
{
	// 3x3の縮小ぼかし処理を行う
	vec2 texel = 1.0 / vec2(textureSize(texColor, 0)); // テクセルサイズを計算する
	outColor  = texture(texColor, inTexcoord) * 4;
	outColor += texture(texColor, inTexcoord + vec2(-texel.x, texel.y));
	outColor += texture(texColor, inTexcoord + vec2( texel.x, texel.y));
	outColor += texture(texColor, inTexcoord + vec2(-texel.x,-texel.y));
	outColor += texture(texColor, inTexcoord + vec2( texel.x,-texel.y));
	outColor *= 1.0 / 8.0; // 平均化させる
}