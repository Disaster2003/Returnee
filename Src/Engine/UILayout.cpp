/**
* @file UILayout.cpp
*/
#include "UILayout.h"
#include "Engine.h"

/// <summary>
/// UIレイアウトの実行を開始する
/// </summary>
void UILayout::Start()
{
	// 最初に指定された座標を「カメラから見た座標」とする
	position_base = GetOwner()->position;
}

/// <summary>
/// 常にカメラの方を向くようにUIを調整する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void UILayout::Update(float deltaTime)
{
	// 常にカメラの方を向くようにゲームオブジェクトの向きを調整(ビルボードと同じ)
	auto owner = GetOwner();
	const auto engine = owner->GetEngine();
	const auto& camera = engine->GetMainCamera();
	owner->rotation.y = camera.rotation.y;
	owner->rotation.x = camera.rotation.x;

	// 常にカメラの正面に位置するようにゲームオブジェクトの座標を調整
	// 回転の公式によってカメラとの相対座標を求める
	vec3 pos = position_base; // 角度0度の場合の座標

	// y座標の-1~+1が、カメラに写る範囲の上端と下端になるように、Z座標を調整
	pos.z = -engine->GetFovScale();

	const float sx = sin(camera.rotation.x);
	const float cx = cos(camera.rotation.x);
	const float sy = sin(camera.rotation.y);
	const float cy = cos(camera.rotation.y);
	pos =
	{
	  pos.x,
	  pos.z * -sx + pos.y * cx,
	  pos.z * cx + pos.y * sx
	};
	pos =
	{
	  pos.x * cy + pos.z * sy,
	  pos.y,
	  pos.x * -sy + pos.z * cy
	};
	owner->position = camera.position + pos;
}