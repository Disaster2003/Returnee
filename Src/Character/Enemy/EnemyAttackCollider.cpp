/**
* @file EnemyAttackCollider.cpp
*/
#include "EnemyAttackCollider.h"
#include "../Player/PlayerComponent.h"
#include "../../Effect/GuardEffect.h"

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
            // ガードエフェクト
            for (int i = 0; i < 5; ++i)
            {
                vec3 position_effect = (GetOwner()->GetParent()->position - player->GetOwner()->position) / 2 + player->GetOwner()->position;
                position_effect += 1;
                auto effect_guard = GetOwner()->GetEngine()->Create<GameObject>("guard effect", position_effect);
                effect_guard->AddComponent<GuardEffect>();
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
