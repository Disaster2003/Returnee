/**
* @file standard.vert
*/
#version 450

// シェーダへの入力
layout(location=0) in vec3 inPosition; // 頂点座標
layout(location=1) in vec2 inTexcoord; // テクスチャ座標
layout(location=2) in vec3 inNormal;   // 法線ベクトル
layout(location=3) in vec4 inTangent;  // タンジェントベクトル

// シェーダからの出力
layout(location=0) out vec3 outPosition; // ワールド座標
layout(location=1) out vec2 outTexcoord; // テクスチャ座標
layout(location=2) out mat3 outTBN;      // 法線変換行列
layout(location=5) out vec3 outShadowTexcoord; // シャドウテクスチャ座標

// プログラムからの入力
layout(location=0) uniform mat4 transformMatrix;
layout(location=1) uniform mat3 normalMatrix;

// アスペクト比と視野角による拡大率
layout(location=3) uniform vec2 aspectRatioAndScaleFov;

layout(location=4) uniform vec3 cameraPosition; // カメラの位置
layout(location=5) uniform vec2 cameraSinCosY;	// カメラのY軸回転
layout(location=6) uniform vec2 cameraSinCosX;	// カメラのX軸回転

layout(location=10) uniform mat4 shadowTextureMatrix; // シャドウテクスチャ行列
layout(location=11) uniform float shadowNormalOffset; // 座標を法線方向にずらす量

vec3 RotateXY(vec3 v, vec2 sinCosX, vec2 sinCosY)
{
	// X軸回転
	v.zy = vec2(v.z * sinCosX.y + v.y * sinCosX.x, -v.z * sinCosX.x + v.y * sinCosX.y);

	// Y軸回転
	v.xz = vec2(v.x * sinCosY.y + v.z * sinCosY.x, -v.x * sinCosY.x + v.z * sinCosY.y);

	return v;
}

// エントリーポイント
void main()
{
	outTexcoord = inTexcoord;

	// ローカル座標系からワールド座標系に変換
	gl_Position = transformMatrix * vec4(inPosition, 1);
	outPosition = gl_Position.xyz;

	// ワールド法線を計算する
	outTBN[0] = normalMatrix * inTangent.xyz;
	outTBN[2] = normalMatrix * inNormal;
	outTBN[1] = inTangent.w * cross(outTBN[2], outTBN[0]);

	// シャドウテクスチャ座標を計算する
	outShadowTexcoord = outPosition + outTBN[2] * shadowNormalOffset;
	outShadowTexcoord = vec3(shadowTextureMatrix * vec4(outShadowTexcoord, 1));

	// ワールド座標系からクリップ座標系に変換する
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

	// ビュー座標系からクリップ座標系に変換する
	gl_Position.xy *= aspectRatioAndScaleFov;

	// 深度値の計算結果が-1～+1になるようなパラメータA,Bを計算する
	const float near = 0.35;
	const float far = 1000;
	const float A = -2 * far * near / (far - near);
	const float B = (far + near) / (far - near);

	// 遠近法を有効にする
	gl_Position.w = -gl_Position.z;
	gl_Position.z = -gl_Position.z * B + A; // 深度値を補正
}