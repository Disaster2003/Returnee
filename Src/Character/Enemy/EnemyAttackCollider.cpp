/**
* @file EnemyAttackCollider.cpp
*/
#include "EnemyAttackCollider.h"
#include "../Player/PlayerComponent.h"

/// <summary>
/// 衝突時に衝突した相手によって処理を行う
/// </summary>
/// <param name="self">自身</param>
/// <param name="other">衝突した相手</param>
void EnemyAttackCollider::OnCollision
(
    const ComponentPtr& self,
    const ComponentPtr& other
)
{
    // 攻撃判定が起動してなかったら、何も起こさない
    if (EnemyAttackCollider::GetState() != STATE_ATTACK_COLLIDER::COLLISION_DISABLE)
        return;

    auto engine = GetOwner()->GetEngine();
    auto otherObject = other->GetOwner();
    // プレイヤーと衝突したなら
    if (otherObject->name == "player")
    {
        const auto& player = otherObject->GetComponent<PlayerComponent>();
        // ガードしていたら
        if (player->GetStateSword() == PlayerComponent::STATE_SWORD::GUARD)
        {
            // ガード成功
            EasyAudio::PlayOneShot(SE::sword_guard);
            player->SetAfterGuard();
            auto player_owner = player->GetOwner();
            // ガードエフェクト
            for (int i = 0; i < 5; ++i)
            {
                // 正面を決定する
                const vec3 dirFront = { sin(player_owner->rotation.y), 0, cos(player_owner->rotation.y) };

                // 発動を表す煙を表示する
                vec3 position_effect = player_owner->position - dirFront;
                auto effect_guard = engine->Create<GameObject>("guard effect", position_effect);
                effect_guard->AddComponent<BloodParticle>();
                effect_guard->materials[0]->texBaseColor = engine->GetTexture("Res/guard_effect.tga");
                effect_guard->materials[0]->texEmission = engine->GetTexture("Res/guard_effect.tga");
            }
        }
        else
            // ダメージを与える
            player->TakeDamage(damage, GetOwner()->GetParent());

        // 攻撃処理を停止する
        Deactivate();
    }
}

/// <summary>
/// ダメージ量を設定する
/// </summary>
/// <param name="_damage">敵に与えるダメージ量</param>
void EnemyAttackCollider::SetDamage(float _damage)
{
    // 攻撃音も鳴らす
    EasyAudio::PlayOneShot(SE::enemy_attack);
    damage = _damage;
}
