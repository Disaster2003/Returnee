/**
* @file Stardust.h
*/
#ifndef STARDUST_H_INCLUDED
#define STARDUST_H_INCLUDED
#include "Particle.h"
#include "../Engine/Random.h"

// キラキラ
class Stardust
	: public Particle
{
public:
	// コンストラクタ
	Stardust()
		:Particle(LIFE_SPAN)
	{}
	// デフォルトデストラクタ
	virtual ~Stardust() = default;

	/// <summary>
	/// パーティクルを初期化する
	/// </summary>
	virtual void Awake() override
	{
		// パーティクルの共通項を初期化する
		Particle::Awake();

		// パーティクルの相違点を初期化する
		auto owner = GetOwner();
		auto engine = owner->GetEngine();
		owner->materials[0]->texBaseColor = engine->GetTexture("Res/particle_star.tga");
		owner->materials[0]->baseColor = { 0, 0, 0, 1 };      // ライトの影響をなくす
		owner->materials[0]->emission = { 3.0f, 3.0f, 0.2f }; // 強い黄
		owner->position.y++; // 上に移動
		owner->scale = vec3(0.25f);
	}

	/// <summary>
	/// 常にパーティクルを更新する
	/// </summary>
	/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
	virtual void Update(float deltaTime) override
	{
		auto owner = GetOwner();
		auto engine = owner->GetEngine();

		owner->position += delta_position * deltaTime;    // 上に移動
		owner->rotation.z += float(sin(120)) * deltaTime; // 回転
		owner->color.w -= deltaTime;					  // 徐々に透明化

		// 寿命を減らす
		Particle::Update(deltaTime);
	}

private:
	// パーティクルの移動量
	vec3 delta_position = { Random::Range(-1.0f,1.0f),2,Random::Range(-1.0f,1.0f) };
	static constexpr float LIFE_SPAN = 1.0f; // キラキラの寿命(s)
};

#endif // !STARDUST_H_INCLUDED
