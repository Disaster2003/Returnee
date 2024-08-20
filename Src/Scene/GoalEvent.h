/**
* @file GoalEvent.h
*/
#ifndef GOALEVENT_H_INCLUDED
#define GOALEVENT_H_INCLUDED
#include "../Engine/Component.h"

/// <summary>
/// ゴール演出を行うコンポーネント
/// </summary>
class GoalEvent
	:public Component
{
public:
	// デフォルトコンストラクタ
	GoalEvent() = default;
	// デフォルトデストラクタ
	virtual ~GoalEvent() = default;

	// コンポーネントを初期化する
	virtual void Awake() override;
	// 常にゴール画面を更新する
	virtual void Update(float deltaTime) override;

private:
	// ゴール演出における衝突イベント
	virtual void OnCollision(const ComponentPtr& self, const ComponentPtr& other)override;
	
	GameObjectPtr obj_fade;	// フェードアウトオブジェクト
	float time_fade = 0,	// フェードアウトまでの時間
		time_particle = 0;	// パーティクル生成までの時間

	bool isTriggered = false; // ゴール判定がtrue = 起動、false = 停止
};
#endif // !GOALEVENT_H_INCLUDED