#version 450

layout(location=1) in vec2 inTexcoord;

out float outColor;

layout(binding=0) uniform sampler2D texDepth; // 深度テクスチャ

layout(location=200) uniform vec4 saoInfo;
#define RADIUS2             (saoInfo.x) // サンプリング半径(m)の2乗
#define RADIUS_IN_TEXCOORDS (saoInfo.y) // サンプリング半径(テクスチャ座標)
#define BIAS                (saoInfo.z) // 平面とみなすコサイン値
#define INTENSITY           (saoInfo.w) // AOの強度

// NDC座標をビュー座標に変換するためのパラメータ
layout(location=201) uniform vec2 ndcToView;

const float twoPi = 3.14159265 * 2; // 2π(360度)
const float samplingCount = 11;     // サンプル数
const float spiralTurns = 7;        // サンプル点の回転回数
const int maxMipLevel = 3;          // 最大ミップレベル

// ミップレベル0になるサンプル点の距離(2^logMipOffsetピクセル)
const int logMipOffset = 3;

// ビュー座標系の座標を計算する
vec3 GetViewSpacePosition(vec2 uv, float linearDepth)
{
	// テクスチャ座標からNDC座標に変換する
	vec2 ndc = uv * 2 - 1;

	// NDC座標から視点からの距離1mの場合の視点座標に変換する
	vec2 viewOneMeter = ndc * ndcToView;

	// 距離が深度値の場合の視点座標に変換する
	return vec3(viewOneMeter * linearDepth, -linearDepth);
}

// SAO(Scalable Ambient Obscurance)により遮蔽率を求める
void main()
{
	// ピクセルの視点座標と法線を求める
	float depth = textureLod(texDepth, inTexcoord, 0).x;
	vec3 positionVS = GetViewSpacePosition(inTexcoord, depth);
	vec3 normalVS = normalize(cross(dFdx(positionVS), dFdy(positionVS)));

	// フラグメントごとに回転の開始角度をずらすことで見た目を改善する
	const ivec2 iuv = ivec2(gl_FragCoord.xy);
	const float startAngle = mod((3 * iuv.x ^ iuv.y + iuv.x * iuv.y) * 10, twoPi);

	// ワールド座標系とスクリーン座標系のサンプリング半径
	float radiusTS = RADIUS_IN_TEXCOORDS / depth;
	float pixelsSS = radiusTS * textureSize(texDepth, 0).y;

	float occlusion = 0; // 遮蔽率
	for (int i = 0; i < samplingCount; ++i)
	{
		// サンプル点の角度と距離を求める
		float ratio = (float(i) + 0.5) * (1.0 / samplingCount);
		float angle = ratio * (spiralTurns * twoPi) + startAngle;
		vec2 unitOffset = vec2(cos(angle), sin(angle)); 

		// サンプル点の視点座標を求める
		vec2 uv = inTexcoord + ratio * radiusTS * unitOffset;

		// 距離が遠いほど高いミップレベルを選択する
		int mipLevel = clamp(findMSB(int(ratio * pixelsSS)) - logMipOffset, 0, maxMipLevel);

		// サンプル点の視点座標を求める
		float sampleDepth = textureLod(texDepth, uv, mipLevel).x;
		vec3 samplePositionVS = GetViewSpacePosition(uv, sampleDepth);

		// サンプル点へのベクトルと法線のコサインを求める
		// コサインが1(垂直)に近いほど遮蔽率が上がる
		vec3 v = samplePositionVS - positionVS;
		float vn = dot(v, normalVS); // 角度による遮蔽率

		// サンプル点が中心に近いほど遮蔽率が上がる
		float vv = dot(v, v);
		float f = max(RADIUS2 - vv, 0); // 距離による遮蔽率

		// サンプル点までの距離とコサインからAOを求める
		occlusion += f * f * f * max((vn - BIAS) / (vv + 0.001), 0);

		// 水平または垂直に近い面のオクルージョンを平均化する。
		if (abs(dFdx(positionVS.z)) < 0.02)
			occlusion -= dFdx(occlusion) * ((iuv.x & 1) - 0.5);
		if (abs(dFdy(positionVS.z)) < 0.02)
			occlusion -= dFdy(occlusion) * ((iuv.y & 1) - 0.5);

		outColor = occlusion;
	}

	// 平均値を求め、AOの強さを乗算する
	occlusion = min(1.0, occlusion / samplingCount * INTENSITY);
}