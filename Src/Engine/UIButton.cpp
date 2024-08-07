/**
* @file UIButton.cpp
*/
#include "UIButton.h"
#include "Engine.h"

/// <summary>
/// 常にクリックされているかどうか判定する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void UIButton::Update(float deltaTime)
{
	// 基底クラスのメンバ関数を実行
	UILayout::Update(deltaTime);

	auto owner = GetOwner();
	auto engine = owner->GetEngine();

	// ボタンの有効・無効判定
	if (!isInteractable)
	{
		std::copy_n(&color_normal.x, 3, &owner->color.x);
		owner->color[3] = 0.75f; // 無効ぽさを出すため少し透明にする
		return;
	}

	const vec2 mouse = engine->GetMousePosition();
	if (abs(mouse.x - position_base.x) < owner->scale.x &&
		abs(mouse.y - position_base.y) < owner->scale.y)
	{
		// カーソルがボタン領域内にある
		const bool buttonDown = engine->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT);
		if (buttonDown)
			// 左ボタンが押されている
			std::copy_n(&color_pressed.x, 3, &owner->color.x);
		else
		{
			// 左ボタンが離されている
			std::copy_n(&color_highlighted.x, 3, &owner->color.x);
			// ボタンクリック判定
			if (isPrevButtonDown)
				// デリゲートを実行
				for (auto& e : onClick)
					e(this);
		}	//if buttonDown

		// マウスボタンの状態を更新
		isPrevButtonDown = buttonDown;
	}
	else
	{
		// カーソルがボタン領域外にある
		std::copy_n(&color_normal.x, 3, &owner->color.x);
		isPrevButtonDown = false;
	}
}