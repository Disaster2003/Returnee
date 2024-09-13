/**
* @file MagicMissile.cpp
*/
#include "MagicMissile.h"
#include "../Player/PlayerComponent.h"

/// <summary>
/// ���@������������
/// </summary>
void MagicMissile::Awake()
{
    Particle::Awake();

    auto owner = GetOwner();
    auto engine = owner->GetEngine();
    owner->materials[0]->texBaseColor = engine->GetTexture("Res/particle_fire.tga");
    owner->materials[0]->baseColor = { 0, 0, 0, 1 }; // ���C�g�̉e�����Ȃ���
    owner->materials[0]->emission = { 2.0f, 1.0f, 0.2f };
    owner->scale = vec3(0.7f + static_cast<float>(rand() % 4) * 0.1f);
    owner->rotation.z = static_cast<float>(rand() % 10) * 0.63f;

    // �R���C�_�[��ݒ肷��
    auto collider = owner->AddComponent<SphereCollider>();
    collider->sphere.radius = 1.0f;
    collider->isTrigger = true;
}

/// <summary>
/// ��ɖ��@�̏�Ԃ��X�V����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void MagicMissile::Update(float deltaTime)
{
    auto owner = GetOwner();
    owner->position += velocity * deltaTime; // ���X�Ƀv���C���[�̕����Ɉړ�
    owner->rotation.z -= 2 * deltaTime;      // ��]

    Particle::Update(deltaTime);

    // �p�[�e�B�N���𐶐�����
    time_particle -= deltaTime;
    if (time_particle <= 0)
    {
        time_particle = 0.05f;
        auto go = owner->GetEngine()->Create<GameObject>("magic missile particle", owner->position);
        go->AddComponent<MagicMissileParticle>();
    }
}

/// <summary>
/// �Փˎ��ɏՓ˂�������ɂ���ď������s��
/// </summary>
/// <param name="self">����</param>
/// <param name="other">�Փ˂�������</param>
void MagicMissile::OnCollision
(
    const ComponentPtr& self,
    const ComponentPtr& other
)
{
    auto otherObject = other->GetOwner();
    // �K�[�h����
    if (otherObject->name == "player")
    {
        auto engine = GetOwner()->GetEngine();
        const auto& player = otherObject->GetComponent<PlayerComponent>();
        if (player->GetStateSword() == PlayerComponent::STATE_SWORD::GUARD)
        {
            // �K�[�h�����Đ�����
            EasyAudio::PlayOneShot(SE::sword_guard);

            // �K�[�h�̒�~
            player->SetAfterGuard();

            auto player_owner = player->GetOwner();
            // �K�[�h�G�t�F�N�g
            for (int i = 0; i < 5; ++i)
            {
                // ���ʂ����肷��
                const vec3 dirFront = { sin(player_owner->rotation.y), 0, cos(player_owner->rotation.y) };

                // �K�[�h�G�t�F�N�g�̐���
                vec3 position_effect = player_owner->position - dirFront;
                auto effect_guard = engine->Create<GameObject>("guard effect", position_effect);
                effect_guard->AddComponent<PlayerActionSuccessParticle>();
                effect_guard->materials[0]->texBaseColor = engine->GetTexture("Res/particle_guard.tga");
                effect_guard->materials[0]->texEmission = engine->GetTexture("Res/particle_guard.tga");
            }
        }
        else
        {
            // �_���[�W��^����
            player->TakeDamage(damage, GetOwner());
        }

        // ���g��j������
        GetOwner()->Destroy();
        return;
    }
    // �h���S���ɂ̓q�b�g���Ȃ�
    else if (strncmp(otherObject->name.c_str(), "Dragon", 6) == 0)
    {
        return;
    }
    // �I�[�N�V���[�}���ɂ̓q�b�g���Ȃ�
    else if (strncmp(otherObject->name.c_str(), "Orc", 3) == 0)
    {
        return;
    }
    // �K�[�h������Ȃ���΁A����Ƀq�b�g���Ȃ�
    else if (otherObject->name == "player.hand")
    {
        return;
    }
    else if (otherObject->name == "weapon")
    {
        return;
    }

    // ���g��j������
    GetOwner()->Destroy();
}

/// <summary>
/// �_���[�W�ʂ�ݒ肷��
/// </summary>
/// <param name="_damage">�G�ɗ^����_���[�W��</param>
void MagicMissile::SetDamage(int _damage)
{
    // ���@�����炷
    EasyAudio::PlayOneShot(SE::enemy_magic);
    damage = _damage;
}
