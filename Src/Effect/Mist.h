/**
* @file Mist.h
*/
#ifndef MIST_H_INCLUDED
#define MIST_H_INCLUDED
#include "../Engine/Component.h"
#include "../Engine/GameObject.h"
#include "../Engine/Billboard.h"
#include "../Engine/Random.h"
#include "../Engine/Engine.h"
#include "../Engine/Debug.h"

/// <summary>
/// 霧コンポーネント
/// </summary>
class Mist
	:public Component
{
	friend class MistGenerator;

public:
	// デフォルトコンストラクタ
	inline Mist() = default;
	// デフォルトデストラクタ
	inline virtual ~Mist() = default;

	/// <summary>
	/// 常に霧の状態を更新する
	/// </summary>
	/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
	virtual void Update(float deltaTime) override
	{
		// 一定時間経ったら
		timer += deltaTime;
		if (timer >= life_span)
		{
			// 自身を破棄する
			GetOwner()->Destroy();
		}

		// 寿命が半分以下なら
		if (timer >= life_span / 2)
		{
			// 透明にしていく
			GetOwner()->color[3] = (life_span - timer) / (life_span / 2);
		}
		else
		{
			// 不透明にしていく
			GetOwner()->color[3] = timer / (life_span / 2);
		}
	}

private:
	float timer = 0;	// 経過時間タイマー
	float life_span = 7;	// 寿命(秒)
};

/// <summary>
/// 霧を発生させるコンポーネント
/// </summary>
class MistGenerator
	:public Component
{
public:
	// デフォルトコンストラクタ
	inline MistGenerator() = default;
	// デフォルトデストラクタ
	inline virtual ~MistGenerator() = default;

	/// <summary>
	/// 霧マテリアルを初期化する
	/// </summary>
	inline virtual void Awake() override
	{
		// 霧マテリアルを作成する
		mistMaterial = CloneMaterialList(
			GetOwner()->GetEngine()->GetStaticMesh("plane_xy"));
		mistMaterial[0]->texBaseColor = GetOwner()->GetEngine()->GetTexture("Res/effect_mist.tga");
	}

	/// <summary>
	/// 霧発生装置の動作
	/// </summary>
	/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
	virtual void Update(float deltaTime) override
	{
		// インターバル中
		timer += deltaTime;
		if (timer < time_particle)
		{
			return;
		}

		// インターバル中にする
		timer -= time_particle;

		auto owner = GetOwner();
		auto engine = owner->GetEngine();
		// nullチェック
		if (!owner || !engine)
		{
			LOG_WARNING("霧が存在しません");
			return;
		}

		// 霧を発生させる位置をランダムに選択
		vec3 pos = owner->position;
		pos.x += Random::Range(-range.x, range.x);
		pos.y += 0.15f;	// いい感じに表示されるように高さを調整
		pos.z += Random::Range(-range.y, range.y);

		// 霧を生成
		auto mist = engine->Create<GameObject>("mist", pos);
		mist->scale = { Random::Range(0.7f,1.5f),Random::Range(0.7f,1.5f) };
		mist->staticMesh = engine->GetStaticMesh("plane_xy");
		mist->materials = mistMaterial;
		mist->render_queue = RENDER_QUEUE_TRANSPARENT;
		mist->AddComponent<Billboard>();
		mist->AddComponent<Mist>();
	}

	vec2 range = { 1,1 };		// XZ方向の発生半径
	float time_particle = 1.0f;	// 霧を発生する間隔

private:
	MaterialList mistMaterial;	// 霧マテリアル
	float timer = 0;			// 霧発生タイマー
};

#endif // !MIST_H_INCLUDED