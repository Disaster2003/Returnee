/**
* @file ExplorationScene.h
*/
#ifndef EXPLORATIONSCENE_H_INCLUDED
#define EXPLORATIONSCENE_H_INCLUDED
#include "../Engine/Scene.h"

// 先行宣言
class PlayerComponent;
class Dragon;

/// <summary>
/// フィールド探検シーン
/// </summary>
class ExplorationScene
	: public Scene
{
public:
	// デフォルトコンストラクタ
	ExplorationScene() = default;
	// デフォルトデストラクタ
	virtual ~ExplorationScene() = default;

	// シーンを初期化する
	virtual bool Initialize(Engine& engine) override;
	// 常にゲームオーバーになってないか監視する
	virtual void Update(Engine& engine, float deltaTime)override;
	// シーンを終了する
	virtual void Finalize(Engine& engine) override;

private:
	std::shared_ptr<PlayerComponent> playerComponent; // プレイヤー
	std::shared_ptr<Dragon> boss;					  // ボス
};

#endif // EXPLORATIONSCENE_H_INCLUDED