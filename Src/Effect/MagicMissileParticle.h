/**
* @file MagicMissileParticle.h
*/
#ifndef MAGICMISSILEPARTICLE_H_INCLUDED
#define MAGICMISSILEPARTICLE_H_INCLUDED
#include "Particle.h"

/// <summary>
/// 火炎弾の軌跡パーティクル
/// </summary>
class MagicMissileParticle
    : public Particle
{
public:
    // コンストラクタ
    MagicMissileParticle()
        :Particle(LIFE_SPAN)
    {}
    // デフォルトデストラクタ
    virtual ~MagicMissileParticle() = default;

    /// <summary>
    /// パーティクルを初期化する
    /// </summary>
    virtual void Awake() override
    {
        Particle::Awake();

        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        owner->materials[0]->texBaseColor = engine->GetTexture("Res/particle_fire.tga");
        owner->materials[0]->baseColor = { 0, 0, 0, 1 }; // ライトの影響をなくす
        owner->materials[0]->emission = { 2.0f, 1.0f, 0.5f };
        owner->scale = vec3(0.2f + static_cast<float>(rand() % 4) * 0.1f);
        owner->rotation.z = static_cast<float>(rand() % 10) * 0.63f;
    }

    /// <summary>
    /// 常に火炎弾の軌跡を更新する
    /// </summary>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        owner->materials[0]->baseColor.w -= deltaTime; // 徐々に透明化
        // 徐々に発光
        owner->materials[0]->emission =
            vec3(2.0f, 1.0f, 0.5f) * (MagicMissileParticle::GetLifespan() * 4 + 0.2f);
        // X軸方向に縮小する
        owner->scale =
            vec3(std::max(owner->scale.x - deltaTime, 0.0f));
        owner->position.y += deltaTime * 5;

        Particle::Update(deltaTime);
    }

private:
    static constexpr float LIFE_SPAN = 0.25f; // パーティクルの寿命(s)
};

#endif // !MAGICMISSILEPARTICLE_H_INCLUDED
