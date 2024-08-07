/**
* @file UILayout.h
*/
#ifndef UILAYOUT_H_INCLUDED
#define UILAYOUT_H_INCLUDED
#include "Component.h"
#include "VecMath.h"

/// <summary>
/// UIを画面空間で配置するためのコンポーネント
/// </summary>
class UILayout
	:public Component
{
public:
	// デフォルトコンストラクタ
	UILayout() = default;
	// デフォルトデストラクタ
	virtual ~UILayout() = default;

	// UIレイアウトの実行を開始する
	void Start()override;
	// 常にカメラの方を向くようにUIを調整する
	void Update(float deltaTime)override;

	vec3 position_base = vec3(0); // カメラから見た座標
};

#endif UILAYOUT_H_INCLUDED