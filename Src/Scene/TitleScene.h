/**
* @file TitleScene.h
*/
#ifndef TITLESCENE_H_INCLUDED
#define TITLESCENE_H_INCLUDED
#include "../Engine/Scene.h"
#include "../Engine/UIButton.h"

/// <summary>
/// タイトル画面
/// </summary>
class TitleScene
	:public Scene
{
public:
	// デフォルトコンストラクタ
	TitleScene() = default;
	// デフォルトデストラクタ
	virtual ~TitleScene() = default;

	// タイトル画面を初期化する
	virtual bool Initialize(Engine& engine) override;
	// 常にタイトル画面を更新する
	virtual void Update(Engine& engine, float deltaTime) override;
	// タイトル画面を終了する
	virtual void Finalize(Engine& engine) override;

private:
	// フェードアウト用の変数
	GameObjectPtr obj_fade;
	float time_fade = 0;

	std::shared_ptr<UILayout> title_logo; // タイトルロゴ
	std::shared_ptr<UILayout> title_bg;	  // タイトル背景

	// フェードインtrue = した、false = していない
	bool isFadedIn = false;
};

#endif // ! TITLESCENE_H_INCLUDED