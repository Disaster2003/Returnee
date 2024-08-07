/**
* @file UIButton.h
*/
#ifndef UIBUTTON_H_INCLUDED
#define UIBUTTON_H_INCLUDED
#include "UILayout.h"
#include <functional>

/// <summary>
/// UI用のボタン制御コンポーネント
/// </summary>
class UIButton
	:public UILayout
{
public:
	// デフォルトコンストラクタ
	UIButton() = default;
	// デフォルトデストラクタ
	virtual ~UIButton() = default;

	// 常にクリックされているかどうか判定する
	virtual void Update(float deltaTime) override;

	bool isInteractable = true;					 // マウスに反応をtrue = する、false = しない
	vec3 color_normal = { 1,1,1 };				 // 通常色
	vec3 color_highlighted = { 1.3f,1.3f,1.3f }; // 選択色
	vec3 color_pressed = { 0.7f,0.7f,0.7f };	 // 押下色

	// デリゲート
	using ClickEvent = std::function<void(UIButton*)>;
	std::vector<ClickEvent> onClick;

private:
	// 前回更新時のマウス左ボタンがtrue = 押されている、false = 押されていない
	bool isPrevButtonDown = false;
};

#endif // !UIBUTTON_H_INCLUDED