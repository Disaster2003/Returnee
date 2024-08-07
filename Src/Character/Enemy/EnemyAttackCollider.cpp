/**
* @file EnemyAttackCollider.cpp
*/
#include "EnemyAttackCollider.h"
#include "../Player/PlayerComponent.h"
#include "../../Effect/GuardEffect.h"

/// <summary>
/// �Փˎ��ɏՓ˂�������ɂ���ď������s��
/// </summary>
/// <param name="self">���g</param>
/// <param name="other">�Փ˂�������</param>
void EnemyAttackCollider::OnCollision
(
    const ComponentPtr& self,
    const ComponentPtr& other
)
{
    // �U�����肪�N�����ĂȂ�������A�����N�����Ȃ�
    if (EnemyAttackCollider::GetState() != STATE_ATTACK_COLLIDER::COLLISION_DISABLE)
        return;

    auto otherObject = other->GetOwner();
    // �v���C���[�ƏՓ˂����Ȃ�
    if (otherObject->name == "player")
    {
        const auto& player = otherObject->GetComponent<PlayerComponent>();
        // �K�[�h���Ă�����
        if (player->GetStateSword() == PlayerComponent::STATE_SWORD::GUARD)
        {
            // �K�[�h����
            EasyAudio::PlayOneShot(SE::sword_guard);
            player->SetAfterGuard();
            // �K�[�h�G�t�F�N�g
            for (int i = 0; i < 5; ++i)
            {
                vec3 position_effect = (GetOwner()->GetParent()->position - player->GetOwner()->position) / 2 + player->GetOwner()->position;
                position_effect += 1;
                auto effect_guard = GetOwner()->GetEngine()->Create<GameObject>("guard effect", position_effect);
                effect_guard->AddComponent<GuardEffect>();
            }
        }
        else
            // �_���[�W��^����
            player->TakeDamage(damage, GetOwner()->GetParent());

        // �U���������~����
        Deactivate();
    }
}

/// <summary>
/// �_���[�W�ʂ�ݒ肷��
/// </summary>
/// <param name="_damage">�G�ɗ^����_���[�W��</param>
void EnemyAttackCollider::SetDamage(float _damage)
{
    // �U�������炷
    EasyAudio::PlayOneShot(SE::enemy_attack);
    damage = _damage;
}
