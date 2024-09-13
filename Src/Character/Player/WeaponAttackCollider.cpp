/**
* @file WeaponAttackCollider.cpp
*/
#include "WeaponAttackCollider.h"
#include "PlayerComponent.h"
#include "../Enemy/Dragon.h"
#include "../Enemy/Orc.h"
#include "../../Engine/Debug.h"

/// <summary>
/// 衝突時に衝突した相手によって処理を行う
/// </summary>
/// <param name="self">自身</param>
/// <param name="other">衝突した相手</param>
void WeaponAttackCollider::OnCollision
(
    const ComponentPtr& self,
    const ComponentPtr& other
)
{
    // nullチェック
    if (!self)
    {
        LOG_WARNING("武器のコライダーが存在しません");
        return;
    }
    if (!other)
    {
        LOG_WARNING("敵が存在しません");
        return;
    }

    // 攻撃判定が起動してなかったら、何も起こさない
    if (WeaponAttackCollider::GetState() != STATE_ATTACK_COLLIDER::COLLISION_ENABLE)
    {
        return;
    }

    // 攻撃処理
    auto obj_target = other->GetOwner();
    int num_enemy = EnemyJudge(obj_target->name.c_str());
    if (num_enemy)
    {
        // 敵にダメージを与える
        auto player = GetOwner()->GetParent()->GetParent()->GetComponent<PlayerComponent>();
        switch (num_enemy)
        {
        case 1:
            DealDamage(player, obj_target->GetComponent<Dragon>());
            break;
        case 2:
            DealDamage(player, obj_target->GetComponent<Orc>());
            break;
        default:
            break;
        }

        // 攻撃処理を停止する
        Deactivate();
    }
}

/// <summary>
/// 敵かどうか判断する
/// </summary>
/// <param name="s">衝突したオブジェクト名</param>
/// <returns>0 : 敵じゃない、1 : オーク、2 : ドラゴン</returns>
int WeaponAttackCollider::EnemyJudge(const char* s) const
{
    // ドラゴンやオークに攻撃が当たっていたらtrue
    if (strncmp(s, "Dragon", 6) == 0)
    {
        return 1;
    }
    if (strncmp(s, "Orc", 3) == 0)
    {
        return 2;
    }

    // 異なっていたらfalse
    return 0;
}