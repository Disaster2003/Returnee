/**
* @file EnemyAttackCollider.h
*/
#ifndef ENEMYATTACKCOLLIDER_H_INCLUDED
#define ENEMYATTACKCOLLIDER_H_INCLUDED
#include "../AttackCollider.h"

/// <summary>
/// �΃v���C���[�U���p�̃R���C�_�[����
/// </summary>
class EnemyAttackCollider
	: public AttackCollider
{
public:
	// �R���X�g���N�^
	EnemyAttackCollider()
		:AttackCollider(STATE_ATTACK_COLLIDER::SLEEP)
	{}
	// �f�t�H���g�f�X�g���N�^
	virtual ~EnemyAttackCollider() = default;

    // �Փˎ��ɏՓ˂�������ɂ���ď������s��
    virtual void OnCollision(
        const ComponentPtr& self, const ComponentPtr& other) override;

	// �_���[�W�ʂ�ݒ肷��
	void SetDamage(float _damage);

private:
    float damage = 0.0f; // �_���[�W��
};

#endif // !ENEMYATTACKCOLLIDER_H_INCLUDED
