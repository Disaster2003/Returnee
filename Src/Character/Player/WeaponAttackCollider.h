/**
* @file WeaponAttackCollider.h
*/
#ifndef WEAPONATTACKCOLLIDER_H_INCLUDED
#define WEAPONATTACKCOLLIDER_H_INCLUDED
#include "../AttackCollider.h"
#include "../../Effect/BloodParticle.h"

/// <summary>
/// �ΓG�U���p�̃R���C�_�[����
/// </summary>
class WeaponAttackCollider
	: public AttackCollider
{
public:
	// �R���X�g���N�^
	WeaponAttackCollider()
		: AttackCollider(STATE_ATTACK_COLLIDER::SLEEP)
	{}
	// �f�t�H���g�f�X�g���N�^
	virtual ~WeaponAttackCollider() = default;

    // �Փˎ��ɏՓ˂�������ɂ���ď������s��
    virtual void OnCollision(
        const ComponentPtr& self, const ComponentPtr& other) override;

    /// <summary>
    /// �_���[�W�ʂ�ݒ肷��
    /// </summary>
    /// <param name="_damage">�G�ɗ^����_���[�W��</param>
    void SetDamage(int _damage) { damage = _damage; }

    // �G���ǂ������f����
    int EnemyJudge(const char* s) const;

    // �W�I�Ƀ_���[�W��^����
    template<typename T, typename U>
    void DealDamage(const T player, const U enemy)
    {
        // �U����ԂȂ�
        if (player->GetIsAttacking())
        {
            // �U����ԉ���
            player->SetIsAttacking();

            // �_���[�W��^����
            enemy->TakeDamage(damage, player->GetOwner());

            // �����Ԃ�
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
    int damage = 0; // �_���[�W��
};

#endif // !WEAPONATTACKCOLLIDER_H_INCLUDED
