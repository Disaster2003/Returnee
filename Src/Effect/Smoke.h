/**
* @file Smoke.h
*/
#ifndef SMOKE_H_INCLUDED
#define SMOKE_H_INCLUDED
#include "Particle.h"

/// <summary>
/// 煙の粒子
/// </summary>
class Smoke
    : public Particle
{
public:
    // コンストラクタ
    Smoke()
        :Particle(LIFE_SPAN)
    {}
    // デフォルトデストラクタ
    virtual ~Smoke() = default;

    /// <summary>
    /// パーティクルを初期化する
    /// </summary>
    virtual void Awake() override
    {
        Particle::Awake();

        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        owner->materials[0]->texBaseColor = engine->GetTexture("Res/particle_fire.tga");
        owner->materials[0]->baseColor = { 0.05f, 0.025f, 0.05f, 1 }; // ライトの影響をなくす
        owner->materials[0]->emission = { 0.05f, 0.025f, 0.05f };
        owner->scale = vec3(0.7f + static_cast<float>(rand() % 4) * 0.1f);
        owner->rotation.z = static_cast<float>(rand() % 10) * 0.63f;
        owner->position.x += static_cast<float>(rand() % 10) * 0.02f - 0.1f;
        owner->position.z += static_cast<float>(rand() % 10) * 0.02f - 0.1f;
    }

    /// <summary>
    /// 常に煙の粒子の状態を更新する
    /// </summary>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        owner->position.y += 0.5f * deltaTime;        // 上に移動
        owner->rotation.z += 0.57f * deltaTime;       // 回転
        owner->scale += vec3(0.75f * deltaTime);      // 徐々に拡大
        owner->color.w =
            std::min(Smoke::GetLifespan() * 2, 1.0f); // 徐々に透明化

        Particle::Update(deltaTime);
    }

private:
    static constexpr float LIFE_SPAN = 1.0f; // 煙の寿命(s)
};

#endif // !SMOKE_H_INCLUDED
