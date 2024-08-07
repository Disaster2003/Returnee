/**
* @file WeaponAttackCollider.h
*/
#ifndef WEAPONATTACKCOLLIDER_H_INCLUDED
#define WEAPONATTACKCOLLIDER_H_INCLUDED
#include "../AttackCollider.h"
#include "../../Effect/BloodParticle.h"

/// <summary>
/// 対敵攻撃用のコライダー制御
/// </summary>
class WeaponAttackCollider
	: public AttackCollider
{
public:
	// コンストラクタ
	WeaponAttackCollider()
		: AttackCollider(STATE_ATTACK_COLLIDER::SLEEP)
	{}
	// デフォルトデストラクタ
	virtual ~WeaponAttackCollider() = default;

    // 衝突時に衝突した相手によって処理を行う
    virtual void OnCollision(
        const ComponentPtr& self, const ComponentPtr& other) override;

    /// <summary>
    /// ダメージ量を設定する
    /// </summary>
    /// <param name="_damage">敵に与えるダメージ量</param>
    void SetDamage(int _damage) { damage = _damage; }

    // 敵かどうか判断する
    int EnemyJudge(const char* s) const;

    // 標的にダメージを与える
    template<typename T, typename U>
    void DealDamage(const T player, const U enemy)
    {
        // 攻撃状態なら
        if (player->GetIsAttacking())
        {
            // 攻撃状態解除
            player->SetIsAttacking();

            // ダメージを与える
            enemy->TakeDamage(damage, player->GetOwner());

            // 血しぶき
            for (int i = 0; i < 5; ++i)
            {
                vec3 position_blood = enemy->GetOwner()->position;
                position_blood.y += 1;
                auto hitEffect = enemy->GetOwner()->GetEngine()->Create<GameObject>("hit effect", position_blood);
                hitEffect->AddComponent<BloodParticle>();
            }
        }
    }


private:
    int damage = 0; // ダメージ量
};

#endif // !WEAPONATTACKCOLLIDER_H_INCLUDED
