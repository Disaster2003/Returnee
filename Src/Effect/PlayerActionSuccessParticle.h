/**
* @file PlayerActionSuccessParticle.h
*/
#ifndef PLAYERACTIONSUCCESSPARTICLE_H_INCLUDED
#define PLAYERACTIONSUCCESSPARTICLE_H_INCLUDED
#include "Particle.h"
#include "../Engine/Debug.h"

/// <summary>
/// プレイヤーの行動が実った時に出すパーティクル
/// </summary>
class PlayerActionSuccessParticle
    : public Particle
{
public:
    // コンストラクタ
    PlayerActionSuccessParticle()
        :Particle(LIFE_SPAN)
    {}
    // デフォルトデストラクタ
    inline virtual ~PlayerActionSuccessParticle() = default;

    /// <summary>
    /// パーティクルを初期化する
    /// </summary>
    virtual void Awake() override
    {
        Particle::Awake();

        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        // nullチェック
        if (!owner || !engine)
        {
            LOG_WARNING("プレイヤー用の演出が存在しません");
            return;
        }
        owner->materials[0]->baseColor = { 0.2f, 0.5f, 1.5f, 1 };
        owner->materials[0]->emission = { 0.2f, 0.5f, 1.5f };
        owner->scale = vec3(0.1f + static_cast<float>(rand() % 4) * 0.1f);
        owner->rotation.z = static_cast<float>(rand() % 10) * 0.63f;
        owner->position.x += static_cast<float>(rand() % 10) * 0.02f - 0.1f;
        owner->position.z += static_cast<float>(rand() % 10) * 0.02f - 0.1f;
        velocity.x = static_cast<float>(rand() % 7) - 3;
        velocity.y = static_cast<float>(rand() % 7) - 3;
        velocity.z = static_cast<float>(rand() % 7) - 3;
    }

    /// <summary>
    /// 常にパーティクルの状態を更新する
    /// </summary>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        // nullチェック
        if (!owner)
        {
            LOG_WARNING("プレイヤー用の演出が存在しません");
            return;
        }
        velocity.y -= 9.81f * deltaTime;
        owner->position += velocity * deltaTime;          // 上に移動
        owner->rotation.z += random_rotation * deltaTime; // 回転
        owner->scale += vec3(0.25f * deltaTime);          // 徐々に拡大
        owner->color.w =
            std::min(PlayerActionSuccessParticle::GetLifespan() * 8.0f, 1.0f); // 徐々に透明化

        // パーティクルの寿命を更新する
        Particle::Update(deltaTime);
    }

private:
    vec3 velocity = vec3(0);
    static constexpr float rotationList[] = { -2.0f, -1.0f, -0.5f, 0.5f, 1.0f, 2.0f };
    float random_rotation = rotationList[rand() % std::size(rotationList)];
    static constexpr float LIFE_SPAN = 0.25f; // パーティクルの寿命(s)
};

#endif // !PLAYERACTIONSUCCESSPARTICLE_H_INCLUDED
