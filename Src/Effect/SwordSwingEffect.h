/**
* @file SwordSwingEffect.h
*/
#ifndef SWORDSWINGEFFECT_H_INCLUDED
#define SWORDSWINGEFFECT_H_INCLUDED
#include "Particle.h"

/// <summary>
/// 斬撃跡
/// </summary>
class SwordSwingEffect
	: public Particle
{
public:
	// コンストラクタ
	SwordSwingEffect()
		:Particle(LIFE_SPAN)
	{}
	// デフォルトデストラクタ
	inline virtual ~SwordSwingEffect() = default;

    /// <summary>
    /// パーティクルを初期化する
    /// </summary>
    virtual void Awake() override
    {
        Particle::Awake();

        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        owner->materials[0]->texBaseColor = engine->GetTexture("Res/effect_swing.tga");
        owner->materials[0]->texEmission = engine->GetTexture("Res/effect_swing.tga");
        owner->materials[0]->baseColor = { 0.2f, 0.5f, 1.5f, 1 };
        owner->materials[0]->emission = { 0.2f, 0.5f, 1.5f };
        owner->scale.x *= 0.1f;
    }

    /// <summary>
    /// 常に血しぶきの状態を更新する
    /// </summary>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    virtual void Update(float deltaTime) override
    {
        // 血しぶきの寿命を更新する
        Particle::Update(deltaTime);
    }

private:
	static constexpr float LIFE_SPAN = 0.5f; // 血しぶきの寿命(s)
};

#endif // !SWORDSWINGEFFECT_H_INCLUDED
