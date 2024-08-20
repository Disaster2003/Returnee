/**
* @file GoalEvent.cpp
*/
#include "GoalEvent.h"
#include "../Engine/Engine.h"
#include "../Engine/SphereCollider.h"
#include "TitleScene.h"
#include "../Effect/Stardust.h"
#include "../Engine/EasyAudio/EasyAudio.h"
#include "../AudioSettings.h"

/// <summary>
/// コンポーネントを初期化する
/// </summary>
void GoalEvent::Awake()
{
	auto owner = GetOwner();

	// コンポーネントの設定
	auto collider = owner->AddComponent<SphereCollider>();
	collider->sphere.radius = 1.0f;
	collider->sphere.position.y = 0.5f + 0.9f;
	collider->isStatic = true;
}

/// <summary>
/// 常にゴール画面を更新する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void GoalEvent::Update(float deltaTime)
{
	auto owner = GetOwner();
	auto engine = owner->GetEngine();

	// ゴールに触れたら、キラキラしたパーティクルを生成し続ける
	if (isTriggered)
	{
		time_particle += deltaTime;
		if (time_particle >= 0.1f)
		{
			time_particle -= 0.1f;
			auto light = engine->Create<GameObject>("light", owner->position);
			light->AddComponent<Stardust>();
		}
	}

	// 徐々にフェードアウト
	if (time_fade > 0)
	{
		time_fade -= deltaTime;
		obj_fade->color[3] = 1 - time_fade;
		// フェードアウトが終わったら、タイトル画面に切り替える
		if (time_fade <= 0)
			engine->SetNextScene<TitleScene>();
	} // if fadeTimer
}

/// <summary>
/// ゴール演出における衝突イベント
/// </summary>
/// <param name="self">衝突したコライダーコンポーネント(自分)</param>
/// <param name="other">衝突したコライダーコンポーネント(相手)</param>
void GoalEvent::OnCollision
(
	const ComponentPtr& self,
	const ComponentPtr& other
)
{
	// 一度だけプレイヤーと衝突できる
	if (isTriggered || other->GetOwner()->name != "player")
		return;

	isTriggered = true; // 状態を「衝突済み」にする

	// メッセージオブジェクトの生成
	Engine* engine = GetOwner()->GetEngine();
	engine->CreateUIObject<UILayout>("Res/UI_logo_game_clear.dds", { 0,0 }, 0.1f);

	// 戻るボタンの生成
	auto button =
		engine->CreateUIObject<UIButton>
		(
			"Res/button_return_title.dds",
			{ 0,-0.5f },
			0.1f
		);
	button.second->onClick.push_back
	(
		[this](UIButton* button)
		{
			time_fade = 1;
			button->isInteractable = false;
			EasyAudio::PlayOneShot(SE::click);
		}
	);

	// フェードアウト用画像の生成
	auto fade =
		engine->CreateUIObject<UILayout>
		(
			"Res/white.tga",
			{ 0,0 },
			1
		);
	obj_fade = fade.first;
	const vec2 fbSize = engine->GetFramebufferSize();
	obj_fade->scale = { fbSize.x / fbSize.y,1,1 };
	std::fill_n(&obj_fade->color.x, 4, 0.0f);

	// ゲームクリアBGM/SEを再生
	EasyAudio::Stop(AudioPlayer::bgm);
	EasyAudio::Stop(AudioPlayer::run);
	EasyAudio::Play(AudioPlayer::bgm, BGM::game_clear, 1, true);
	EasyAudio::PlayOneShot(SE::goal);
}