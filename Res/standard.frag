/**
* @file standard.frag
*/
#version 450

// シェーダへの入力
layout(location=0) in vec3 inPosition;		 // ワールド座標
layout(location=1) in vec2 inTexcoord;		 // テクスチャ座標
layout(location=2) in mat3 inTBN;			 // 法線変換行列
layout(location=5) in vec3 inShadowTexcoord; // シャドウテクスチャ座標

// テクスチャサンプラ
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texEmission;
layout(binding=2) uniform sampler2DShadow texShadow; // 影用の深度テクスチャ
layout(binding=3) uniform sampler2D texNormal; // 法線テクスチャ
layout(binding=4) uniform samplerCube texEnvironment; // 環境キューブマップ

layout(location=4) uniform vec3 cameraPosition; // カメラ座標

// プログラムからの入力
layout(location=100) uniform vec4 color;		// 物体の色
layout(location=101) uniform vec4 emission;		// 物体の発光色
layout(location=102) uniform float alphaCutoff; // フラグメントを破棄する境界値

// x = 物体表面の粗さ、y = 金属かどうか(0 = 非金属、1 = 金属)
layout(location=103) uniform vec2 roughnessAndMetallic;

// 環境光
layout(location=107) uniform vec3 ambientLight;

// 平行光源
struct DirectionalLight
{
	vec3 color;		// 色と明るさ
	vec3 direction; // 光の向き
};
layout(location=108) uniform DirectionalLight directionalLight;

// 出力する色データ
out vec4 outColor;

// ライト
struct Light
{
	vec4 colorAndFalloffAngle[16];	// 色と明るさ、減衰開始角度
	vec4 positionAndRadius[16];		// 座標と半径
	vec4 directionAndConeAngle[16]; // 向き、最大照射角度
};
layout(location=110) uniform int lightCount;
layout(location=111) uniform Light pointLight;

// 法線分布項
float NormalDistributionGGX(vec3 N, vec3 H, float roughness)
{
	// 光線を視線方向に反射する微小平面の比率を求める
	float NdotH = max(dot(N, H), 0);
	float NdotH2 = NdotH * NdotH;
	
	// dot(N, H)が影響する比率を求める
	float r2 = roughness * roughness;
	float r4 = r2 * r2;
	float denom = (NdotH2 * (r4 - 1.0) + 1.0);
	denom = 3.14159265 * denom * denom;
	
	return r4 / denom;
}

// 幾何減衰項
float GeometricAttenuationSchlick(float NdotL, float NdotV, float roughness)
{
	float k = (roughness + 1) * (roughness + 1) * 0.125;
	
	// 光源方向から見た幾何学的減衰項を計算する
	float g0 = NdotL / (NdotL * (1 - k) + k);
	
	// 視点方向から見た幾何学的減衰項を計算する
	float g1 = NdotV / (NdotV * (1 - k) + k);
	
	return g0 * g1;
}

// フレネル項
vec3 FresnelSchlick(vec3 f0, float VdotH)
{
	// シュリックの近似式
	float vh1 = 1 - VdotH;
	float vh2 = vh1 * vh1;
	return f0 + (1 - f0) * (vh2 * vh2 * vh1);
}

// cosθの最小値
const float minCosTheta = 0.000001;

// 鏡面反射BRDF
vec3 SpecularBRDF(
  vec3 normal, vec3 H, float roughness, vec3 cameraVector, float NdotL, vec3 F)
{
  // 法線分布項を計算する
  float D = NormalDistributionGGX(normal, H, roughness);

  // 幾何学的減衰項を計算する
  float NdotV = max(dot(normal, cameraVector), minCosTheta);
  float G = GeometricAttenuationSchlick(NdotL, NdotV, roughness);

  // 鏡面反射BRDFを計算する
  float denom = 4 * NdotL * NdotV;
  return (D * G * F) * (1 / denom);
}

// CalcBRDFの計算結果
struct BRDFResult
{
  vec3 diffuse;  // 拡散反射
  vec3 specular; // 鏡面反射
};

// 鏡面反射BRDFと拡散反射BRDFをまとめて計算する
BRDFResult CalcBRDF(vec3 normal, vec3 f0,
  vec3 cameraVector, vec3 direction, vec3 color)
{
	// フレネルを計算する
	vec3 H = normalize(direction + cameraVector);
	vec3 F = FresnelSchlick(f0, max(dot(cameraVector, H), 0));

	// GGXで鏡面反射を計算する
	float NdotL = max(dot(normal, direction), minCosTheta);
	vec3 specularBRDF = SpecularBRDF(
	  normal, H, roughnessAndMetallic.x, cameraVector, NdotL, F);
	vec3 specular = color * specularBRDF * NdotL;

	// 正規化ランバートで拡散反射を計算する
	float diffuseBRDF = NdotL / 3.14159265;
	vec3 diffuse = color * diffuseBRDF * (1 - F) * (1 - roughnessAndMetallic.y);

	return BRDFResult(diffuse, specular);
}

// エントリーポイント
void main()
{
	vec4 c = texture(texColor, inTexcoord);

	// カットオフ値が指定されている場合、アルファがその値未満の場合は描画をキャンセルする
    if (alphaCutoff > 0)
	{
		if (c.a < alphaCutoff)
			// フラグメントを破棄(キャンセル)する
			discard;

		// 破棄されなかったら不透明にする
		c.a = 1;
	}

	// テクスチャのガンマ補正を解除する
	const float crtGamma = 2.2; // CRTモニターのガンマ値
	c.rgb = pow(c.rgb, vec3(crtGamma));

	outColor = c * color;

	// 法線の長さが0でなければ、法線テクスチャが設定されていると判断する
	vec3 normal = texture(texNormal, inTexcoord).xyz;
	if (dot(normal, normal) > 0.0001)
	{
	  // 8bit値であることを考慮しつつ0～1を-1～+1に変換
	  // この式を使うと8bitの128がちょうど0になる
	  normal = normal * (255.0 / 127.0) - (128.0 / 127.0);

	  // XY成分からZ成分を復元(法線の長さは常に1なため)
	  normal.z = sqrt(1 - normal.x * normal.x - normal.y * normal.y);

	  // タンジェント座標系からワールド座標系に変換
	  // 線形補間によってoutTBNの長さが1ではなくなっているので、正規化して長さを1に復元する
	  normal = normalize(inTBN * normal);
	}
	else
	  // 法線テクスチャが未設定の場合は頂点法線を使う
	  normal = normalize(inTBN[2]);

	 // 視線ベクトル
	vec3 cameraVector = normalize(cameraPosition - inPosition);

	// 角度0のフレネル値
	vec3 f0 = mix(vec3(0.04), outColor.rgb, roughnessAndMetallic.y);

	vec3 diffuse=vec3(0); // 拡散光の明るさの合計
	vec3 specular = vec3(0);// 鏡面反射光の明るさの合計
	for(int i = 0; i < lightCount; ++i)
	{
		// ポイントライトの方向を測る
		vec3 direction = pointLight.positionAndRadius[i].xyz - inPosition;

		// 光源までの距離を測る
		float sqrDistance = dot(direction,direction);
		float distance = sqrt(sqrDistance);

		// 方向を正規化して長さを1にする
		direction = normalize(direction);

	    float illuminance = 1;

		// 照射角度が0より大きければスポットライトとみなす
		const float coneAngle = pointLight.directionAndConeAngle[i].w;
		if(coneAngle > 0)
		{
			// 「ライトからフラグメントへ向かうベクトル」と「スポットライトのベクトル」のなす角を計算する
			// 角度がconeAngle以上なら範囲外
			float angle = acos(dot(-direction, pointLight.directionAndConeAngle[i].xyz));
			if(angle >= coneAngle)
				continue; // 照射範囲外

			// 最大照射角度のとき0,減衰角度の時1になるように補間する
			const float falloffAngle = pointLight.colorAndFalloffAngle[i].w;
			const float a = min((coneAngle - angle) / (coneAngle - falloffAngle), 1);
			illuminance *= a;
		}

		// ライトの最大距離を制限する
		const float radius = pointLight.positionAndRadius[i].w;
		const float smoothFactor = clamp(1 - pow(distance / radius, 4), 0, 1);
		illuminance *= smoothFactor * smoothFactor;

		// 逆2乗の法則によって明るさを減衰させる
		// 0除算が起きないように1を足している
		illuminance /= sqrDistance + 1;

		// 拡散反射と鏡面反射を計算する
		vec3 color = pointLight.colorAndFalloffAngle[i].xyz * illuminance;
		BRDFResult result = CalcBRDF(normal, f0, cameraVector, direction, color);
		diffuse += result.diffuse;
		specular += result.specular;
	} // for lightCount

	// 影を計算する
	float shadow = texture(texShadow, inShadowTexcoord).r;

	// 平行光源の明るさを計算する
	BRDFResult result = CalcBRDF(normal, f0, cameraVector,
	  -directionalLight.direction, directionalLight.color);
	specular += result.specular * shadow;
	diffuse += result.diffuse * shadow;

    // アンビエントライトの明るさを計算する
	vec3 Fa = f0 + (1 - f0) * 0.0021555; // 角度45°のフレネル値
	specular += ambientLight * Fa;
	diffuse += ambientLight * (1 - Fa) * (1 - roughnessAndMetallic.y);

	// 環境キューブマップによるライティング
	{
		// カメラベクトルと法線から反射ベクトルを計算する
		vec3 d = dot(cameraVector, normal) * normal;
		vec3 reflectionVector = 2 * d - cameraVector;
	
		// スペキュラ反射率を計算する
		float NdotV = max(dot(normal, cameraVector), 0.01);
		float G = GeometricAttenuationSchlick(NdotV, NdotV, roughnessAndMetallic.x);
		vec3 F = FresnelSchlick(f0, max(dot(cameraVector, normal), 0));

		// きつい角度の反射が強くなりすぎないように制限する値
		const float angleThreshold = cos(radians(60));
		float denom = mix(angleThreshold, 1, NdotV);
		vec3 specularRatio = F * (G / denom);

		// キューブマップから鏡面反射成分を取得する
		float maxMipmapLevel = textureQueryLevels(texEnvironment) - 1;
		float levelCap = textureQueryLod(texEnvironment, reflectionVector).x;
		float level = max(maxMipmapLevel * pow(roughnessAndMetallic.x, 0.5), levelCap);
		vec3 ambientSpecular = textureLod(texEnvironment, reflectionVector, level).rgb;
		ambientSpecular = pow(ambientSpecular, vec3(2.2)); // ガンマ補正を解除
		specular += ambientSpecular * specularRatio;

		// キューブマップから拡散反射成分を取得
		vec3 ambientDiffuse = textureLod(texEnvironment, normal, maxMipmapLevel).rgb;
		ambientDiffuse = pow(ambientDiffuse, vec3(2.2)); // ガンマ補正を解除
		diffuse += ambientDiffuse * (1 - specularRatio) * (1 - roughnessAndMetallic.y);
	}

	// 拡散光の影響を反映する
	outColor.rgb *= diffuse;

	// 鏡面反射の影響を反映する
	outColor.rgb += specular;

	// 発光色を反映する
	if(emission.w > 0)
		outColor.rgb += texture(texEmission, inTexcoord).rgb * emission.rgb;
	else
		outColor.rgb += emission.rgb;

  // ガンマ補正はimage_processing.fragで行うので、ここでは実行しない。
}