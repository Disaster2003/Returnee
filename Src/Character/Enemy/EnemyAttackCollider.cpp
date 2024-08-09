/**
* @file EnemyAttackCollider.cpp
*/
#include "EnemyAttackCollider.h"
#include "../Player/PlayerComponent.h"

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

    auto engine = GetOwner()->GetEngine();
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
            auto player_owner = player->GetOwner();
            // �K�[�h�G�t�F�N�g
            for (int i = 0; i < 5; ++i)
            {
                // ���ʂ����肷��
                const vec3 dirFront = { sin(player_owner->rotation.y), 0, cos(player_owner->rotation.y) };

                // ������\������\������
                vec3 position_effect = player_owner->position - dirFront;
                auto effect_guard = engine->Create<GameObject>("guard effect", position_effect);
                effect_guard->AddComponent<BloodParticle>();
                effect_guard->materials[0]->texBaseColor = engine->GetTexture("Res/guard_effect.tga");
                effect_guard->materials[0]->texEmission = engine->GetTexture("Res/guard_effect.tga");
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
