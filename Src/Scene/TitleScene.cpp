/**
* @file TitleScene.cpp
*/
#include "TitleScene.h"
#include "ExplorationScene.h"
#include "../Engine/Light.h"
#include "../Engine/Engine.h"
#include "../Engine/EasyAudio/EasyAudio.h"
#include "../AudioSettings.h"

/// <summary>
/// タイトル画面を初期化する
/// </summary>
/// <return>true = 初期化成功、false = 初期化失敗</return>
bool TitleScene::Initialize(Engine& engine)
{
	// ライトを配置
	auto lightObject =
		engine.Create<GameObject>
		(
			"light",
			engine.GetMainCamera().position
		);
	auto light = lightObject->AddComponent<Light>();
	light->color = { 1.0f,0.9f,0.8f };
	light->intensity = 10;
	light->radius = 5;

	title_bg = engine.CreateUIObject<UILayout>("Res/title_bg.dds", { 0,0 }, 1.3f).second;
	title_bg->GetOwner()->materials[0]->baseColor = { 0, 0, 0, 1 };
	title_logo = engine.CreateUIObject<UILayout>("Res/title_logo.dds", { 0,0.3f }, 0.3f).second;
	title_logo->GetOwner()->materials[0]->baseColor = { 0, 0, 0, 1 };
	auto startButton = engine.CreateUIObject<UIButton>("Res/start_button.dds", { 0,-0.6f }, 0.1f);

	// ボタンが押されたらメインゲームシーンに切り替えるラムダ式を設定
	startButton.second->onClick.push_back
	(
		[this](UIButton* button)
		{
			time_fade = 1;
			button->isInteractable = false;	//ボタンを無効化
			EasyAudio::PlayOneShot(SE::click);
		}
	);

	// フェードアウト用UIオブジェクト
	auto fade = engine.CreateUIObject<UILayout>("Res/white.tga", { 0,0 }, 1);
	obj_fade = fade.first;
	// 画面全体を覆うサイズに設定
	const vec2 fbSize = engine.GetFramebufferSize();
	obj_fade->scale = { fbSize.x / fbSize.y,1,1 };
	// カラーを「黒、透明」に設定
	std::fill_n(&obj_fade->color.x, 4, 0.0f);

	// タイトルBGMを再生
	EasyAudio::Stop(AudioPlayer::bgm);
	EasyAudio::Play(AudioPlayer::bgm, BGM::title, 1, true);

	return true; // 初期化成功
}

/// <summary>
/// 常にタイトル画面を更新する
/// </summary>
/// <param name="engine">ゲームエンジン</param>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void TitleScene::Update
(
	Engine& engine,
	float deltaTime
)
{
	if (!isFadedIn)
	{
		// タイトルロゴをフェードインさせる
		float c = title_logo->GetOwner()->materials[0]->baseColor.x;
		c = std::min(c + deltaTime * 0.5f, 1.0f);
		title_logo->GetOwner()->materials[0]->baseColor = { c, c, c, 1 };
		if (c >= 1)
		{
			c = 1;
			isFadedIn = true;
		}
	}
	else
	{
		// タイトル背景をフェードインさせる
		float c = title_bg->GetOwner()->materials[0]->baseColor.x;
		c = std::min(c + deltaTime * 0.5f, 1.0f);
		title_bg->GetOwner()->materials[0]->baseColor = { c, c, c, 1 };
		if (c >= 1)
			c = 1;
	}

	if (time_fade > 0)
	{
		// タイマーに合わせて徐々にフェードアウトさせる
		time_fade -= deltaTime;
		obj_fade->color[3] = 1 - time_fade;

		// タイマーが0以下になったらメインゲームシーンに切り替える
		if (time_fade <= 0)
			engine.SetNextScene<ExplorationScene>();
	}//if fadeTimer
}

/// <summary>
/// タイトル画面を終了する
/// </summary>
void TitleScene::Finalize(Engine& engine)
{
	engine.ClearGameObjectAll();
}