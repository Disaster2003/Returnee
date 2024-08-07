/**
* @file high_pass_filter.frag
*/
#version 450

// シェーダへの入力
layout(location = 1) in vec2 inTexcoord;

// テクスチャ
layout(binding = 0) uniform sampler2D texColor;

// プログラムからの入力
// x: 高輝度とみなす明るさ(しきい値)
// y: ブルームの強さ
layout(location = 202) uniform vec2 highPassFilter;

// 出力する色データ
out vec4 outColor;

// 明るい成分を抽出する
vec3 GetBrighterPart(in vec2 texcoord)
{
	// RGBのうちもっとも明るい成分をピクセルの明るさとする
	vec3 color = texture(texColor, texcoord).rgb;
	float brightness = max(color.r, max(color.g, color.b));
	
	// 明るい成分の比率を計算する
	float ratio = max(brightness - highPassFilter.x, 0) / max(brightness, 0.001);
	return color * ratio; // 明るい成分を計算する
}

// エントリーポイント
void main()
{
	// テクセルサイズを計算する
	vec2 oneTexel = 1.0 / vec2(textureSize(texColor, 0));
	
	// 明るい成分を計算し、4x4ピクセルの縮小処理を行う
	outColor.rgb  = GetBrighterPart(inTexcoord + vec2(-oneTexel.x, oneTexel.y));
	outColor.rgb += GetBrighterPart(inTexcoord + vec2( oneTexel.x, oneTexel.y));
	outColor.rgb += GetBrighterPart(inTexcoord + vec2(-oneTexel.x,-oneTexel.y));
	outColor.rgb += GetBrighterPart(inTexcoord + vec2( oneTexel.x,-oneTexel.y));
	
	// ブルームの強さを乗算する
	outColor.rgb *= (1.0 / 4.0) * highPassFilter.y;
	outColor.a = 1;
}