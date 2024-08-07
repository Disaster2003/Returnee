/**
* @file MagicMissile.cpp
*/
#include "MagicMissile.h"
#include "../Player/PlayerComponent.h"
#include "../../Effect/GuardEffect.h"

/// <summary>
/// 魔法を初期化する
/// </summary>
void MagicMissile::Awake()
{
    Particle::Awake();

    auto owner = GetOwner();
    auto engine = owner->GetEngine();
    owner->materials[0]->texBaseColor = engine->GetTexture("Res/particle_fire.tga");
    owner->materials[0]->baseColor = { 0, 0, 0, 1 }; // ライトの影響をなくす
    owner->materials[0]->emission = { 2.0f, 1.0f, 0.5f };
    owner->scale = vec3(0.7f + static_cast<float>(rand() % 4) * 0.1f);
    owner->rotation.z = static_cast<float>(rand() % 10) * 0.63f;

    // コライダーを設定する
    auto collider = owner->AddComponent<SphereCollider>();
    collider->sphere.radius = 1.0f;
    collider->isTrigger = true;
}

/// <summary>
/// 常に魔法の状態を更新する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void MagicMissile::Update(float deltaTime)
{
    auto owner = GetOwner();
    owner->position += velocity * deltaTime; // 徐々にプレイヤーの方向に移動
    owner->rotation.z -= 2 * deltaTime;      // 回転

    Particle::Update(deltaTime);

    // パーティクルを生成する
    time_particle -= deltaTime;
    if (time_particle <= 0)
    {
        time_particle = 0.05f;
        auto go = owner->GetEngine()->Create<GameObject>("magic missile particle", owner->position);
        go->AddComponent<MagicMissileParticle>();
    }
}

/// <summary>
/// 衝突時に衝突した相手によって処理を行う
/// </summary>
/// <param name="self">自分</param>
/// <param name="other">衝突した相手</param>
void MagicMissile::OnCollision
(
    const ComponentPtr& self,
    const ComponentPtr& other
)
{
    auto otherObject = other->GetOwner();
    // プレイヤーと衝突したら
    if (otherObject->name == "player")
    {
        const auto& player = otherObject->GetComponent<PlayerComponent>();
        // ガードしていなかったら
        if (player->GetStateSword() == PlayerComponent::STATE_SWORD::GUARD)
        {
            // ガード成功
            EasyAudio::PlayOneShot(SE::sword_guard);
            player->SetAfterGuard();
            // ガードエフェクト
            for (int i = 0; i < 5; ++i)
            {
                vec3 position_effect = GetOwner()->position;
                auto effect_guard = GetOwner()->GetEngine()->Create<GameObject>("guard effect", position_effect);
                effect_guard->AddComponent<GuardEffect>();
            }
        }
        else
            // ダメージを与える
            player->TakeDamage(damage, GetOwner());

        // 自身を破棄する
        GetOwner()->Destroy();
        return;
    }
    // ドラゴンにはヒットしない
    else if (strncmp(otherObject->name.c_str(), "Dragon", 6) == 0)
        return;
    // オークシャーマンにはヒットしない
    else if (strncmp(otherObject->name.c_str(), "Orc", 3) == 0)
        return;
    // ガード中じゃなければ、武器にヒットしない
    else if (otherObject->name == "player.hand")
        return;
    else if (otherObject->name == "weapon")
        return;
    // 自身を破棄する
    GetOwner()->Destroy();
}

/// <summary>
/// ダメージ量を設定する
/// </summary>
/// <param name="_damage">敵に与えるダメージ量</param>
void MagicMissile::SetDamage(int _damage)
{
    // 魔法音も鳴らす
    EasyAudio::PlayOneShot(SE::enemy_magic);
    damage = _damage;
}
