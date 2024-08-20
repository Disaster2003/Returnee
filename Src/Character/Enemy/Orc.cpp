/**
* @file Orc.cpp
*/
#include "Orc.h"
#include "../../Engine/Animation.h"
#include "../../Engine/GameObject.h"
#include "../Player/PlayerComponent.h"
#include "../../Effect/Smoke.h"
#include "../../Engine/Random.h"

/// <summary>
/// �I�[�N�̃R���|�[�l���g������������
/// </summary>
void Orc::Awake()
{
	auto owner = GetOwner();
	auto engine = owner->GetEngine();

	// �R���|�[�l���g��ݒ肷��
	for (float i = 0; i < 2; ++i)
	{
		auto collider = owner->AddComponent<SphereCollider>();
		collider->sphere.radius = 0.5f + i * 0.2f;
		collider->sphere.position.y = 1.0f + 0.9f * i;
	}

	// �U�������ݒ肷��
	characterMovement = owner->AddComponent<CharacterMovement>();
    if (job == JOB::FIGHTER)
    {
        auto go = owner->GetEngine()->Create<GameObject>("OrcShaman Right Arm Attack Collision ", { -1.7f, 1.65f, 0.6f });
        go->SetParentAndJoint(owner, 2); // �E�r�ɐڑ�
        attackCollider_right_arm = go->AddComponent<EnemyAttackCollider>();
    }

	// ���ʂ̏��������s��
	const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };

	// �ҋ@�ʒu�̏��������s��
	position_wait = owner->position + dirFront * 2;
}

/// <summary>
/// ��ɃI�[�N�̏�Ԃɂ���ē�����X�V����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Orc::Update(float deltaTime)
{
    // �v���C���[�����݂�����
    if (GetTarget())
    {
        // �v���C���[������ł����牽�����Ȃ�
        const auto player = GetTarget()->GetComponent<PlayerComponent>();
        if (player->GetStatePlayer() == PlayerComponent::STATE_PLAYER::DEAD)
        {
            characterMovement->velocity = vec3(0);
            animator->SetSpeed(0);
            return;
        }
    }

    /*
      �I�[�N�̍s���ɂ���:

      �̗͂�0�ȉ��ɂȂ�����u���S�v�ɑJ��.

      �ҋ@: �o���n�_�̎���5m�Ƀ����_���ȖړI�n��ݒ肵�Ĉړ�.
           �ړI�n�ɓ���������1�`5�b�Î~.
           10m�ȓ��Ƀv���C���[���N�������烉���_�����ԂŃ��C�L���X�g. ���C�����B������u�ǐՁv�ɑJ��.
           ��L���J��Ԃ�.

      �ǐ�: �v���C���[��ǂ�����������Ɉړ�.
           �����_�����ԂŁu���@�v�ɑJ��.
           �v���C���[�܂ł̋�����3�܂���5m�ȓ��Ȃ�΁u�U���v�ɑJ��.
           �v���C���[�܂ł̋�����10m�ȏ�Ȃ�΃����_�����ԂŃ��C�L���X�g. ���C�����B���Ȃ�������u�ҋ@�v�ɑJ��.

      ���@:
      �U��: �A�j���[�V�������I��������u�ǐՁv�ɑJ��.
    */
    switch (state)
    {
    case STATE_ORC::WAIT: DoWait(deltaTime); break;
    case STATE_ORC::WALK: DoWalk(deltaTime); break;
    case STATE_ORC::ATTACK: DoAttack(deltaTime); break;
    case STATE_ORC::ATTACK_MAGICMISSILE: DoAttackMagicMissile(deltaTime); break;
    case STATE_ORC::DEAD: DoDead(deltaTime); break;
    }
}

/// <summary>
/// �_���[�W���󂯂�
/// </summary>
/// <param name="damage">�_���[�W��</param>
/// <param name="causer">���g</param>
void Orc::TakeDamage
(
    int damage,
    GameObject* causer
)
{
    // ��Ԃ��u���S�v�Ȃ牽���N����Ȃ�
    if (state == STATE_ORC::DEAD)
        return;

    // �a��ꂽ����炷
    EasyAudio::PlayOneShot(SE::sword_hit);

    // �m�b�N�o�b�N
    const vec3 v = normalize(GetOwner()->position - causer->position);
    characterMovement->velocity.x += 4 * v.x;
    characterMovement->velocity.z += 4 * v.z;

    // �_���[�W���󂯂�
    Orc::SetHp(damage);

    // �̗͂�0�ȉ��Ȃ�
    if (Orc::GetHp() <= 0)
    {
        // ���S
        StartDead();

        // ���SSE��炷
        EasyAudio::PlayOneShot(SE::orc_dead);
    }
}

/// <summary>
/// �u�ҋ@�v���J�n����
/// </summary>
void Orc::StartWait()
{
    // ��Ԃ��u�ҋ@�v�Ȃ牽�����Ȃ�
    if (state == STATE_ORC::WAIT)
        return;

    if (animator)
    {
        // �u�ҋ@�v�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("wait");
    }

    // ��Ԃ��u�ҋ@�v�ɂ���
    state = STATE_ORC::WAIT;
}

/// <summary>
/// �u�ǐՁv���J�n����
/// </summary>
void Orc::StartWalk()
{
    // ��Ԃ��u�ǐՁv�Ȃ牽�����Ȃ�
    if (state == STATE_ORC::WALK)
        return;

    if (animator)
    {
        // �u�ǐՁv�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION * 2);
        animator->Play("walk");
    }

    // ��Ԃ��u�ǐՁv�ɂ���
    state = STATE_ORC::WALK;
}

/// <summary>
/// �u�U���v���J�n����
/// </summary>
void Orc::StartAttack()
{
    // ��Ԃ��u�U���v�Ȃ牽�����Ȃ�
    if (state == STATE_ORC::ATTACK)
        return;

    if (animator)
    {
        // �u�U���v�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION);
        // �u�U���v�A�u���U���v�������_���őI��ōs��
        if (Random::Range(0, 1) <= 0.5f)
            animator->Play("attack");
        else
            animator->Play("attack.right");
    }

    // �_���[�W�ʂ�ݒ肷��
    attackCollider_right_arm->SetDamage(ATTAK_NORMAL);

    // �U�������ݒ肷��
    attackCollider_right_arm->Activate(1.0f);

    // ���ɍU������܂ł̎���(0.5�`1.5�b)
    time_attack = float(std::rand() % 11) * 0.1f + 0.5f;

    // ��Ԃ��u�U���v�ɂ���
    state = STATE_ORC::ATTACK;
}

/// <summary>
/// �u���@�U���v���J�n����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Orc::StartAttackMagicMissile()
{
    // ��Ԃ��u���@�U���v�Ȃ牽�����Ȃ�
    if (state == STATE_ORC::ATTACK_MAGICMISSILE)
        return;

    auto owner = GetOwner();
    auto engine = owner->GetEngine();

    // ���ʂ����肷��
    const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };

    // ������\������\������
    constexpr vec3 spawnOffset = { 0, 3.0f, 0 };
    const vec3 spawnPosition = owner->position + dirFront + spawnOffset;
    auto smoke = engine->Create<GameObject>("smoke", spawnPosition);
    smoke->AddComponent<Smoke>();
    smoke->materials[0]->texBaseColor = engine->GetTexture("Res/effect_magic_circle.tga");
    smoke->materials[0]->texEmission = engine->GetTexture("Res/effect_magic_circle.tga");
    smoke->materials[0]->baseColor = { 2,2,0.2f,1 };
    smoke->materials[0]->emission = { 2,2,0.2f };

    if (animator)
    {
        // �u���@�U���v�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("magic");
    }
    smoke->materials[0]->emission = { 0.2f, 0.1f, 0.1f };

    // ���ɖ��@���r������܂ł̎���(0.5�`2.0�b)
    time_magic = float(std::rand() % 16) * 0.1f + 0.5f;

    isCasted = false;

    // ��Ԃ��u���@�U���v�ɂ���
    state = STATE_ORC::ATTACK_MAGICMISSILE;
}

/// <summary>
/// �u���v���J�n����
/// </summary>
void Orc::StartDead()
{
    // ��Ԃ��u���S�v�Ȃ牽�����Ȃ�
    if (state == STATE_ORC::DEAD)
        return;

    if (animator)
    {
        // �u���v�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION);
        if (Random::Range(0, 1) <= 0.5f)
            animator->Play("down.0");
        else
            animator->Play("down.1");
    }

    // ��Ԃ��u���S�v�ɂ���
    state = STATE_ORC::DEAD;

    // �U�������j������
    attackCollider_right_arm->Deactivate();
}

/// <summary>
/// �u�ҋ@�v����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Orc::DoWait(float deltaTime)
{
    auto owner = GetOwner();
    auto engine = owner->GetEngine();
    if (time_walk > 0)
    {
        time_walk -= deltaTime;

        const vec3 v = destination - owner->position;
        const float d2 = dot(v, v);
        if (d2 < 0.5f * 0.5f)
        {
            // �ړI�n�ɓ���
            time_walk = 0;
            time_wait = float(rand() % 5) + 1;
            animator->Play("wait");
            animator->SetSpeed(SPEED_ANIMATION);
            characterMovement->DecelerateXZ(DECELERATION * deltaTime);
        }
        else
        {
            // �ړI�n�Ɍ������Ĉړ���
            const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };
            const vec3 nv = v * (1 / sqrt(d2));
            const float sinTheta = cross(dirFront, nv).y;
            owner->rotation.y += 5 * sinTheta * deltaTime;
            characterMovement->AccelerateXZ(5 * nv * deltaTime, 1);
        }
    }
    else if (time_wait > 0)
    {
        // �ړI�n�ɂđҋ@��
        time_wait -= deltaTime;
        characterMovement->DecelerateXZ(DECELERATION * deltaTime);
    }
    else
    {
        // ���̖ړI�n��I������
        float r = radians(float(rand() % 18) * 20);
        constexpr float distanceList[] = { 5, 1, 3, 4, 2, 5, 4, 5, 3, 4 };
        float s = sin(r);
        float c = cos(r);
        destination = position_wait + vec3(c, 0, -s) * distanceList[index_random_walk_distance];
        destination.y = owner->position.y;
        time_walk = 5;
        index_random_walk_distance = (index_random_walk_distance + 1) % std::size(distanceList);
        if (animator->GetCurrentClipName() != "walk")
        {
            animator->Play("walk");
            animator->SetSpeed(SPEED_ANIMATION * 2);
        }
        // ��~����
        characterMovement->DecelerateXZ(DECELERATION * deltaTime);
    }

    // ���G����
    const auto target_info = GetTargetInfo();
    if (target_info)
    {
        float s = DISTANCE_SEARCH;
        if (target_info.distance <= s)
        {
            // �G�𔭌������̂ŒǐՊJ�n
            time_wait = 0;
            time_walk = 0;
            StartWalk();
        }
    }
}

/// <summary>
/// �u�ǐՁv����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Orc::DoWalk(float deltaTime)
{
    auto owner = GetOwner();
    auto engine = owner->GetEngine();

    // ���F�����Ƀv���C���[�����Ȃ�������
    const auto targetInfo = GetTargetInfo();
    if (!targetInfo || targetInfo.distance > DISTANCE_TRACK_MAX)
    {
        // �u�ҋ@�v���J�n����
        StartWait();
        return;
    }

    // �U���J�n�^�C�}�[���X�V����
    if (job == JOB::FIGHTER)
        time_attack = std::max(time_attack - deltaTime, 0.0f);
    else if (job == JOB::MAGICIAN)
        time_magic = std::max(time_magic - deltaTime, 0.0f);

    const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };
    const vec3 dirLeft = { cos(owner->rotation.y), 0, -sin(owner->rotation.y) };

    // �U���Ώۂ������̑O�ɂ�����U���̃`�F�b�N�A�����łȂ���ΐ��񂷂�
    if (targetInfo.position_type == TARGET_INFO::POSITON_TYPE::FRONT)
    {
        // �v���C���[���U�����茗���ł����
        if (targetInfo.distance <= DISTANCE_ATTACK)
        {
            if (time_attack <= 0)
            {
                // �u�U���v���J�n����
                StartAttack();
                return;
            }
            if (time_magic <= 0)
            {
                // �u�Ή��e�U���v���J�n����
                StartAttackMagicMissile();
                return;
            }
        }
    }
    else
    {
        float theta = targetInfo.cos��_by_left;
        owner->rotation.y += 30 * theta * deltaTime;
    }

    // �G�܂ł̋������U���͈͂̋��E�ɗ���悤�ɉ���������؂�ւ���
    constexpr float accel = 4; // ��{�����x
    if (targetInfo.distance > DISTANCE_ATTACK)
        characterMovement->AccelerateXZ(dirFront * SPEED_MOVE_ENEMY * accel * deltaTime, SPEED_MOVE_ENEMY);
    else
        // �t�����ɉ���(����)����
        characterMovement->AccelerateXZ(-dirFront * SPEED_MOVE_ENEMY * accel * deltaTime, SPEED_MOVE_ENEMY);
}

/// <summary>
/// �u�U���v����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Orc::DoAttack(float deltaTime)
{
    // �A�j���[�V�������I��������
    if (!animator->IsPlaying())
    {
        // �u�ǐՁv���J�n����
        StartWalk();
        return;
    }

    // ��~����
    characterMovement->DecelerateXZ(DECELERATION * deltaTime);
}

/// <summary>
/// �u���@�v����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Orc::DoAttackMagicMissile(float deltaTime)
{
    auto owner = GetOwner();

    // �A�j���[�V�������I��������
    if (!animator->IsPlaying())
    {
        // �u�ǐՁv���J�n����
        StartWalk();
        return;
    }

    // ��~����
    characterMovement->DecelerateXZ(DECELERATION * deltaTime);

    // �r���J�n����0.8�b��ɖ��@�𔭓�����
    const float t = animator->GetTimer();
    if (isCasted || t < 0.8f)
        return;

    // ����
    isCasted = true;
    AttackMagicMissile(*owner);
}

/// <summary>
/// �u���v����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Orc::DoDead(float deltaTime)
{
    auto owner = GetOwner();
    auto engine = owner->GetEngine();

    // ��~����
    characterMovement->DecelerateXZ(DECELERATION * deltaTime);

    if (!animator->IsPlaying())
    {
        // �ˑR������̂����܂������߁A���G�t�F�N�g��\������
        for (int i = 0; i < 5; ++i)
        {
            float x = float(rand() % 5) * 0.25f - 0.5f; // -0.5�`+0.5m
            float y = float(rand() % 6) * 0.05f + 0.25f;// 0.25�`0.5m
            float z = float(rand() % 5) * 0.25f - 0.5f; // -0.5�`+0.5m
            auto smoke = engine->Create<GameObject>("smoke", owner->position + vec3(x, y, z));
            smoke->AddComponent<Smoke>();
            float b = float(rand() % 5) * 0.25f + 0.25f;
            smoke->materials[0]->emission = vec3{ 0.125f, 0.15f, 0.1f } *b;
        }

        // ���g��j������
        owner->Destroy();
    }
}
