/**
* @file Light.h
*/
#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED
#include "Component.h"
#include "VecMath.h"

/// <summary>
/// ライト・コンポーネント
/// </summary>
class Light
	: public Component
{
public:
	// デフォルトコンストラクタ
	Light() = default;
	// デストラクタ
	virtual ~Light();

	// ライトを初期化する
	virtual void Awake() override;
	// 常にライトの状態を更新する
	virtual void Update(float deltaTime) override;

	// ライトの種類
	enum class TYPE_LIGHT
	{
		POINT, // 点光源
		SPOT,  // スポット光源
	};
	TYPE_LIGHT type = TYPE_LIGHT::POINT; // ライトの種類

	vec3 color = { 1,1,1 };	// 色
	float intensity = 1;	// 明るさ
	float radius = 1;		// ライトが届く最大半径

	// スポットライト用のパラメータ
	// 方向はGameObject::rotationから計算
	float angle_cone = radians(30);	  // スポットライトが照らす角度
	float angle_falloff = radians(20); // スポットライトの減衰開始角度

private:
	int index_light = -1; // ライトインデックス
};

#endif // !LIGHT_H_INCLUDED