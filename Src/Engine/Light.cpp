/**
* @file Light.cpp
*/
#include "Light.h"
#include "Engine.h"

/// <summary>
/// デストラクタ
/// </summary>
Light::~Light()
{
	// ライトを解放する
	GetOwner()->GetEngine()->DeallocateLight(index_light);
}

/// <summary>
/// ライトを初期化する
/// </summary>
void Light::Awake()
{
	// ライトを取得していなければ、ライトを取得する
	if (index_light < 0)
	{
		index_light = GetOwner()->GetEngine()->AllocateLight();
	}
}

/// <summary>
/// 常にライトの状態を更新する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Light::Update(float deltaTime)
{
	// コンポーネントの値をライトに反映する
	auto owner = GetOwner();
	auto light = owner->GetEngine()->GetLight(index_light);
	if (light)
	{
		light->position = owner->position;
		light->intensity = intensity;
		light->color = color;
		light->radius = radius;
		light->coneAngle = 0; // 照射角度が0以下ならポイントライト扱い

		// スポットライトの場合、スポットライトのパラメータを反映する
		if (type == TYPE_LIGHT::SPOT)
		{
			// ライトの向きを計算
			vec3 direction = { 0,0,-1 }; // 回転なしの向きベクトル

			// X軸回転
			const float sinX = sin(owner->rotation.x);
			const float cosX = cos(owner->rotation.x);
			direction =
			{
				direction.x,
				direction.y * cosX - direction.z * sinX,
				direction.y * sinX + direction.z * cosX
			};

			// Y軸回転
			const float sinY = sin(owner->rotation.y);
			const float cosY = cos(owner->rotation.y);
			direction =
			{
				direction.x * cosY + direction.z * sinY,
				direction.y,
				-direction.x * sinY + direction.z * cosY
			};

			// パラメータを反映する
			light->direction = direction;
			light->coneAngle = angle_cone;
			light->falloffAngle = angle_falloff;
		}
	}
}
