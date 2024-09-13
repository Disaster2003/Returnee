/**
* @file WeaponAttackCollider.cpp
*/
#include "WeaponAttackCollider.h"
#include "PlayerComponent.h"
#include "../Enemy/Dragon.h"
#include "../Enemy/Orc.h"
#include "../../Engine/Debug.h"

/// <summary>
/// �Փˎ��ɏՓ˂�������ɂ���ď������s��
/// </summary>
/// <param name="self">���g</param>
/// <param name="other">�Փ˂�������</param>
void WeaponAttackCollider::OnCollision
(
    const ComponentPtr& self,
    const ComponentPtr& other
)
{
    // null�`�F�b�N
    if (!self)
    {
        LOG_WARNING("����̃R���C�_�[�����݂��܂���");
        return;
    }
    if (!other)
    {
        LOG_WARNING("�G�����݂��܂���");
        return;
    }

    // �U�����肪�N�����ĂȂ�������A�����N�����Ȃ�
    if (WeaponAttackCollider::GetState() != STATE_ATTACK_COLLIDER::COLLISION_ENABLE)
    {
        return;
    }

    // �U������
    auto obj_target = other->GetOwner();
    int num_enemy = EnemyJudge(obj_target->name.c_str());
    if (num_enemy)
    {
        // �G�Ƀ_���[�W��^����
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

        // �U���������~����
        Deactivate();
    }
}

/// <summary>
/// �G���ǂ������f����
/// </summary>
/// <param name="s">�Փ˂����I�u�W�F�N�g��</param>
/// <returns>0 : �G����Ȃ��A1 : �I�[�N�A2 : �h���S��</returns>
int WeaponAttackCollider::EnemyJudge(const char* s) const
{
    // �h���S����I�[�N�ɍU�����������Ă�����true
    if (strncmp(s, "Dragon", 6) == 0)
    {
        return 1;
    }
    if (strncmp(s, "Orc", 3) == 0)
    {
        return 2;
    }

    // �قȂ��Ă�����false
    return 0;
}