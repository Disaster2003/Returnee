/**
* @file EnemyAttackCollider.h
*/
#ifndef ENEMYATTACKCOLLIDER_H_INCLUDED
#define ENEMYATTACKCOLLIDER_H_INCLUDED
#include "../AttackCollider.h"

/// <summary>
/// 対プレイヤー攻撃用のコライダー制御
/// </summary>
class EnemyAttackCollider
	: public AttackCollider
{
public:
	// コンストラクタ
	EnemyAttackCollider()
		:AttackCollider(STATE_ATTACK_COLLIDER::SLEEP)
	{}
	// デフォルトデストラクタ
	virtual ~EnemyAttackCollider() = default;

    // 衝突時に衝突した相手によって処理を行う
    virtual void OnCollision(
        const ComponentPtr& self, const ComponentPtr& other) override;

	// ダメージ量を設定する
	void SetDamage(int _damage);

private:
    int damage = 0; // ダメージ量
};

#endif // !ENEMYATTACKCOLLIDER_H_INCLUDED
