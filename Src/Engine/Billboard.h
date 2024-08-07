/**
* @file Billboard.h
*/
#ifndef BILLBOARD_H_INCLUDED
#define BILLBOARD_H_INCLUDED
#include "Component.h"
#include "Engine.h"

/// <summary>
/// ビルボード化コンポーネント
/// </summary>
class Billboard
	:public Component
{
public:
	// デフォルトコンストラクタ
	inline Billboard() = default;
	// デフォルトデストラクタ
	inline virtual ~Billboard() = default;

	/// <summary>
	/// ビルボード化
	/// </summary>
	/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
	virtual void Update(float deltaTime)override
	{
		// 常にカメラの方を向くようにメッセージオブジェクトの向きを調整
		auto owner = GetOwner();
		const auto engine = owner->GetEngine();
		const auto& camera = engine->GetMainCamera();
		owner->rotation.y = camera.rotation.y;
	}
};

#endif // !BILLBOARD_H_INCLUDED