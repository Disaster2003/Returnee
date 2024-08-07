/**
* @file GuardEffect
*/
#ifndef GUARDEFFECT_H_INCLUDED
#define GUARDEFFECT_H_INCLUDED
#include "Particle.h"

/// <summary>
/// ガードエフェクト
/// </summary>
class GuardEffect
	: public Particle
{
public:
	// コンストラクタ
	GuardEffect()
		:Particle(LIFE_SPAN)
	{}
	// デフォルトデストラクタ
	inline virtual ~GuardEffect() = default;

    /// <summary>
    /// パーティクルを初期化する
    /// </summary>
    virtual void Awake() override
    {
        Particle::Awake();

        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        owner->materials[0]->texBaseColor = engine->GetTexture("Res/guard_effect.dds");
        owner->materials[0]->texEmission = engine->GetTexture("Res/guard_effect.dds");
        owner->materials[0]->baseColor = { 0.7f, 0.7f, 0.7f, 1 };
        owner->materials[0]->emission = { 0.5f, 0.5f, 0.5f };
        owner->scale = vec3(0.1f + static_cast<float>(rand() % 4) * 0.1f);
        owner->rotation.z = static_cast<float>(rand() % 10) * 0.63f;
        owner->position.x += static_cast<float>(rand() % 10) * 0.02f - 0.1f;
        owner->position.z += static_cast<float>(rand() % 10) * 0.02f - 0.1f;
        velocity.x = static_cast<float>(rand() % 7) - 3;
        velocity.y = static_cast<float>(rand() % 7) - 3;
        velocity.z = static_cast<float>(rand() % 7) - 3;
    }

    /// <summary>
    /// 常に血しぶきの状態を更新する
    /// </summary>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        velocity.y -= 9.81f * deltaTime;
        owner->position += velocity * deltaTime;          // 上に移動
        owner->rotation.z += random_rotation * deltaTime; // 回転
        owner->scale += vec3(1.5f * deltaTime);           // 徐々に拡大
        owner->color.w =
            std::min(GuardEffect::GetLifespan() * 8.0f, 1.0f); // 徐々に透明化

        // 血しぶきの寿命を更新する
        Particle::Update(deltaTime);
    }

private:
    vec3 velocity = vec3(0);
    static constexpr float rotationList[] = { -2.0f, -1.0f, -0.5f, 0.5f, 1.0f, 2.0f };
    float random_rotation = rotationList[rand() % std::size(rotationList)];
    static constexpr float LIFE_SPAN = 0.25f; // 血しぶきの寿命(s)
};

#endif // !GUARDEFFECT_H_INCLUDED
