/**
* @file SwordSwingParticle.h
*/
#ifndef SWORDSWINGPARTICLE_H_INCLUDED
#define SWORDSWINGPARTICLE_H_INCLUDED
#include "Particle.h"

class SwordSwingParticle
	: public Particle
{
public:
	// コンストラクタ
	SwordSwingParticle()
		:Particle(LIFE_SPAN)
	{}
	// デフォルトデストラクタ
	inline virtual ~SwordSwingParticle() = default;

    /// <summary>
    /// パーティクルを初期化する
    /// </summary>
    virtual void Awake() override
    {
        Particle::Awake();

        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        owner->materials[0]->texBaseColor = engine->GetTexture("Res/swing_effect.tga");
        owner->materials[0]->texEmission = engine->GetTexture("Res/swing_effect.tga");
        owner->materials[0]->baseColor = { 0.2f, 0.5f, 1.0f, 1 };
        owner->materials[0]->emission = { 0.2f, 0.5f, 1 };
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
	static constexpr float LIFE_SPAN = 0.25f; // 血しぶきの寿命(s)
};

#endif // !SWORDSWINGPARTICLE_H_INCLUDED
