/**
* @file Dragon.cpp
*/
#include "Dragon.h"
#include "../../Engine/Animation.h"
#include "../../Engine/GameObject.h"
#include "../Player/PlayerComponent.h"
#include "../../Effect/Smoke.h"
#include "../../Engine/Random.h"

/// <summary>
/// �h���S���̃R���|�[�l���g������������
/// </summary>
void Dragon::Awake()
{
	auto owner = GetOwner();
	auto engine = owner->GetEngine();

    // �R���|�[�l���g��ݒ肷��
	for (float i = 0; i < 2; ++i)
	{
		auto collider = owner->AddComponent<SphereCollider>();
		collider->sphere.radius = 0.5f + i * 0.2f;
		collider->sphere.position.y = 0.5f + 0.9f * i;
	}

    // �U�������ݒ肷��
	characterMovement = owner->AddComponent<CharacterMovement>();
	auto go = owner->GetEngine()->Create<GameObject>("Dragon Right Arm Attack Collision ", { -1.7f, 1.65f, 0.6f });
	go->SetParentAndJoint(owner, 2); // �E�r�ɐڑ�
	attackCollider_right_arm = go->AddComponent<EnemyAttackCollider>();

	auto go2 = owner->GetEngine()->Create<GameObject>("Dragon Tail Attack Collision ", { 0.00f, 1.00f, -0.6f });
	go2->SetParentAndJoint(owner, 8); // �K���ɐڑ�
	attackCollider_tail = go->AddComponent<EnemyAttackCollider>();

    // ���ʂ̏��������s��
    const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };

    // �ҋ@�ʒu�̏��������s��
    position_wait = owner->position + dirFront * 2;
}

/// <summary>
/// ��Ƀh���S���̏�Ԃɂ���ē�����X�V����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Dragon::Update(float deltaTime)
{
    auto owner = GetOwner();
    auto engine = owner->GetEngine();

    // �v���C���[�����݂�����
    if (GetTarget())
    {
        // �v���C���[������ł����牽�����Ȃ�
        const auto player = GetTarget()->GetComponent<PlayerComponent>();
        if (player->GetStatePlayer() != PlayerComponent::STATE_PLAYER::ALIVE)
        {
            if (ui_hp_frame && ui_hp_gauge)
            {
                // �̗̓Q�[�W��j������
                ui_hp_frame->GetOwner()->Destroy();
                ui_hp_gauge->GetOwner()->Destroy();
            }
            characterMovement->velocity = vec3(0);
            animator->SetSpeed(0);
            return;
        }
    }

    // �̗̓Q�[�W�̕\���E��\���A����
    HpGauge(engine, deltaTime);

    if (!isAngry)
        // �̗͂�1/3��؂��Ă����ꍇ
        if (Dragon::GetHp() <= HP_MAX / 3)
            // �{���Ԃɂ���
            isAngry = true;

    // �{�胂�[�h�ɓ�������
    if (isAngry)
        if (angry_effect_span <= 0)
        {
            angry_effect_span = 1.0f;
            // �^���Ԃ��X�[�p�[�T�C���l
            auto owner = GetOwner();
            const vec3 spawnPosition = { owner->position.x, owner->position.y + 2, owner->position.z };
            auto smoke = owner->GetEngine()->Create<GameObject>("smoke", spawnPosition);
            smoke->AddComponent<Smoke>();
            smoke->materials[0]->emission = { 1.0f, 0.05f, 0.05f };
        }
    angry_effect_span -= deltaTime;

    /*
      �h���S���̍s���ɂ���:

      �̗͂�0�ȉ��ɂȂ�����u���S�v�ɑJ��.

      �ҋ@: �o���n�_�̎���5m�Ƀ����_���ȖړI�n��ݒ肵�Ĉړ�.
           �ړI�n�ɓ���������1�`5�b�Î~.
           5�܂���30m�ȓ��Ƀv���C���[���N�������烉���_�����ԂŃ��C�L���X�g. ���C�����B������u�ǐՁv�ɑJ��.
           ��L���J��Ԃ�.

      �ǐ�: �v���C���[��ǂ�����������Ɉړ�.
           �����_�����ԂŉΉ��e�𔭓����A�u�Ή��e�U���v�ɑJ��.
           �v���C���[�܂ł̋�����3m�ȓ��Ȃ�΁u�U���v�ɑJ��.
           �v���C���[�܂ł̋�����35m�ȏ�Ȃ�΃����_�����ԂŃ��C�L���X�g. ���C�����B���Ȃ�������u�ҋ@�v�ɑJ��.

      �΋�:
      �U��: �A�j���[�V�������I��������u�ǐՁv�ɑJ��.
    */
    switch (state)
    {
    case STATE_DRAGON::WAIT: DoWait(deltaTime); break;
    case STATE_DRAGON::WALK: DoWalk(deltaTime); break;
    case STATE_DRAGON::ATTACK: DoAttack(deltaTime); break;
    case STATE_DRAGON::ATTACK_TAIL: DoAttackTail(deltaTime); break;
    case STATE_DRAGON::ATTACK_FIREBALL: DoAttackFireBall(deltaTime); break;
    case STATE_DRAGON::HOVERING: DoHovering(deltaTime); break;
    case STATE_DRAGON::HOVERING_AND_FIRE: DoHoveringAndFire(deltaTime); break;
    case STATE_DRAGON::DEAD: DoDead(deltaTime); break;
    }
}

/// <summary>
/// �_���[�W���󂯂�
/// </summary>
/// <param name="damage">�_���[�W��</param>
/// <param name="causer">���g</param>
void Dragon::TakeDamage
(
    int damage,
    GameObject* causer
)
{
    // ��Ԃ��u���S�v�Ȃ牽���N����Ȃ�
    if (state == STATE_DRAGON::DEAD)
        return;

    // �a��ꂽ����炷
    EasyAudio::PlayOneShot(SE::sword_hit);

    // �m�b�N�o�b�N
    const vec3 v = normalize(GetOwner()->position - causer->position);
    characterMovement->velocity.x += 4 * v.x;
    characterMovement->velocity.z += 4 * v.z;

    // �_���[�W���󂯂�
    Dragon::SetHp(damage);

    // �̗͂�0�ȉ��Ȃ�
    if (Dragon::GetHp() <= 0)
    {
        // ���S
        StartDead();

        // ���SSE��炷
        EasyAudio::PlayOneShot(SE::dragon_dead);
    }
}

/// <summary>
/// �u�ҋ@�v���J�n����
/// </summary>
void Dragon::StartWait()
{
    // ��Ԃ��u�ҋ@�v�Ȃ牽�����Ȃ�
    if (state == STATE_DRAGON::WAIT)
        return;

    if (animator)
    {
        // �u�ҋ@�v�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("wait");
    }

    // ��Ԃ��u�ҋ@�v�ɂ���
    state = STATE_DRAGON::WAIT;
}

/// <summary>
/// �u�ǐՁv���J�n����
/// </summary>
void Dragon::StartWalk()
{
    // ��Ԃ��u�ǐՁv�Ȃ牽�����Ȃ�
    if (state == STATE_DRAGON::WALK)
        return;

    if (animator)
    {
        // �u�ǐՁv�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION * 2);
        animator->Play("walk");
    }

    // ��Ԃ��u�ǐՁv�ɂ���
    state = STATE_DRAGON::WALK;
}

/// <summary>
/// �u�U���v���J�n����
/// </summary>
void Dragon::StartAttack()
{
    // ��Ԃ��u�U���v�Ȃ牽�����Ȃ�
    if (state == STATE_DRAGON::ATTACK)
        return;

    if (animator)
    {
        // �u�U���v�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("attack");
    }

    // �_���[�W�ʂ�ݒ肷��
    attackCollider_right_arm->SetDamage(ATTAK_NORMAL);

    // �U�������ݒ肷��
    attackCollider_right_arm->Activate(1.0f);

    // �{���ԂȂ�
    if (isAngry)
        // �U���񐔂��c���Ă�����
        if (attack_max != 0)
        {
            // �U���𑱂���
            time_attack = 0;

            // �U���񐔂����炷
            attack_max--;
        }
        else
        {
            // ���ɍU������܂ł̎���(0.5�`1.5�b)
            time_attack = float(std::rand() % 11) * 0.1f + 0.5f;

            // �U���񐔂��[����
            attack_max = 2;
        }
    else
        // ���ɍU������܂ł̎���(0.5�`1.5�b)
        time_attack = float(std::rand() % 11) * 0.1f + 0.5f;

    // ��Ԃ��u�U���v�ɂ���
    state = STATE_DRAGON::ATTACK;
}

/// <summary>
/// �u�K���U���v���J�n����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Dragon::StartAttackTail()
{
    // ��Ԃ��u�K���U���v�Ȃ牽�����Ȃ�
    if (state == STATE_DRAGON::ATTACK_TAIL)
        return;

    if (animator)
    {
        // �u�K���U���v�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("attack_tail");
    }

    // �_���[�W�ʂ�ݒ肷��
    attackCollider_tail->SetDamage(ATTACK_SPECIAL);

    // �U�������ݒ肷��
    attackCollider_tail->Activate(10.0f);

    // ���ɍU������܂ł̎���(0.5�`1.5�b)
    time_attack = float(std::rand() % 11) * 0.1f + 0.5f;

    // �U���񐔂̕�[
    attack_max = 2;

    // ��Ԃ��u�K���U���v�ɂ���
    state = STATE_DRAGON::ATTACK_TAIL;
}

/// <summary>
/// �u�Ή��e�U���v���J�n����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Dragon::StartAttackFireBall()
{
    // ��Ԃ��u�Ή��e�U���v�Ȃ牽�����Ȃ�
    if (state == STATE_DRAGON::ATTACK_FIREBALL)
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

    if (animator)
    {
        // �u�Ή��e�U���v�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("magic");
    }
    smoke->materials[0]->emission = { 0.2f, 0.1f, 0.1f };

    // �{���ԂȂ�
    if (isAngry)
        // �U���񐔂��c���Ă�����
        if (attack_max != 0)
        {
            // �U���𑱂���
            time_attack_fire_ball = 0;

            // �U���񐔂����炷
            attack_max--;
        }
        else
        {
            // ���ɉΉ��e�U������܂ł̎���(0.5�`2.0�b)
            time_attack_fire_ball = float(std::rand() % 16) * 0.1f + 0.5f;

            // �U���񐔂��[����
            attack_max = 2;
        }
    else
        // ���ɉΉ��e�U������܂ł̎���(0.5�`2.0�b)
        time_attack_fire_ball = float(std::rand() % 16) * 0.1f + 0.5f;

    // �Ή��e�U������
    isCasted = false;

    // ��Ԃ��u�Ή��e�U���v�ɂ���
    state = STATE_DRAGON::ATTACK_FIREBALL;
}

/// <summary>
/// �u�󒆑؍݁v���J�n����
/// </summary>
void Dragon::StartHovering()
{
    // ��Ԃ��u�󒆑؍݁v�Ȃ牽�����Ȃ�
    if (state == STATE_DRAGON::HOVERING)
        return;

    if (animator)
    {
        // �u�󒆑؍݁v�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION * 2);
        animator->Play("hovering");
    }

    // �U���񐔂��[����
    attack_max = 3;

    // �ړ�������肷��
    destination = position_hovering[(int)Random::Range(0, 8)];

    // ���ɋ󒆑؍݂���܂ł̎���(8.0f�b)
    time_hovering = 8.0f;

    // ��Ԃ��u�󒆑؍݁v�ɂ���
    state = STATE_DRAGON::HOVERING;
}

/// <summary>
/// �u�󒆑؍ݎ��ɉΉ��e�U���v���J�n����
/// </summary>
void Dragon::StartHoveringAndFire()
{
    // ��Ԃ��u�󒆑؍݁v�Ȃ牽�����Ȃ�
    if (state == STATE_DRAGON::HOVERING_AND_FIRE)
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

    if (animator)
    {
        // �u�Ή��e�U���v�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("magic");
    }
    smoke->materials[0]->emission = { 0.2f, 0.1f, 0.1f };

    // �U���񐔂����炷
    attack_max--;

    // �Ή��e�U������
    isCasted = false;

    // ��Ԃ��u�󒆑؍݁v�ɂ���
    state = STATE_DRAGON::HOVERING_AND_FIRE;
}

/// <summary>
/// �u���v���J�n����
/// </summary>
void Dragon::StartDead()
{
    // ��Ԃ��u���S�v�Ȃ牽�����Ȃ�
    if (state == STATE_DRAGON::DEAD)
        return;

    if (animator)
    {
        // �u���v�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("dead");
    }

    // ��Ԃ��u���S�v�ɂ���
    state = STATE_DRAGON::DEAD;

    // �U�������j������
    attackCollider_right_arm->Deactivate();
    attackCollider_tail->Deactivate();
}

/// <summary>
/// �u�ҋ@�v����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Dragon::DoWait(float deltaTime)
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
            const float sin�� = cross(dirFront, nv).y;
            owner->rotation.y += 5 * sin�� * deltaTime;
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
    const auto targetInfo = GetTargetInfo();
    if (targetInfo)
    {
        float s = DISTANCE_SEARCH;
        if (targetInfo.distance <= s)
        {
            // �G�𔭌������̂ŒǐՊJ�n
            time_wait = 0;
            time_walk = 0;
            //StartWalk();
            StartHovering();
        }
    }
}

/// <summary>
/// �u�ǐՁv����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Dragon::DoWalk(float deltaTime)
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
    time_attack = std::max(time_attack - deltaTime, 0.0f);
    time_attack_fire_ball = std::max(time_attack_fire_ball - deltaTime, 0.0f);
    time_hovering = std::max(time_hovering - deltaTime, 0.0f);

    const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };
    const vec3 dirLeft = { cos(owner->rotation.y), 0, -sin(owner->rotation.y) };

    if (time_hovering <= 0)
    {
        // �u�󒆑؍݁v���J�n����
        StartHovering();
        return;
    }
    // �U���Ώۂ������̑O�ɂ�����U���̃`�F�b�N�A�����łȂ���ΐ��񂷂�
    else if (targetInfo.position_type == TARGET_INFO::POSITON_TYPE::FRONT)
    {
        // �v���C���[���U�����茗���ł����
        if (targetInfo.distance <= DISTANCE_ATTACK)
        {
            if (time_attack <= 0)
            {
                // Health�������ȉ��Ȃ�A�u�K���U���v�������_���őI��ōs��
                if (isAngry)
                    // �����_���ɏo��������0.5�ȉ��Ȃ�
                    if (Random::Range(0, 1) <= 0.5f)
                    {
                        // �u�K���U���v���J�n����
                        StartAttackTail();
                        return;
                    }

                // �u�U���v���J�n����
                StartAttack();
                return;
            }
        }
        else if (time_attack_fire_ball <= 0)
        {
            // �u�Ή��e�U���v���J�n����
            StartAttackFireBall();
            return;
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
    {
        if (isAngry)
            characterMovement->AccelerateXZ(dirFront * SPEED_MOVE_ENEMY * accel * deltaTime, SPEED_MOVE_ENEMY * 0.5f);
        else
            characterMovement->AccelerateXZ(dirFront * SPEED_MOVE_ENEMY * accel * deltaTime, SPEED_MOVE_ENEMY);
    }
    else
        // �t�����ɉ���(����)����
        characterMovement->AccelerateXZ(-dirFront * SPEED_MOVE_ENEMY * accel * deltaTime, SPEED_MOVE_ENEMY);
}

/// <summary>
/// �u�U���v����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Dragon::DoAttack(float deltaTime)
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
/// �u�K���U���v����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Dragon::DoAttackTail(float deltaTime)
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
/// �u�Ή��e�U���v����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Dragon::DoAttackFireBall(float deltaTime)
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
/// �u�󒆑؍݁v����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Dragon::DoHovering(float deltaTime)
{
    auto owner = GetOwner();

    const vec3 v = destination - owner->position;
    const float d2 = dot(v, v);
    if (d2 < 2 * 2)
    {
        // �ړI�n�ɓ���
        if (attack_max <= 0)
            StartWalk();
        else
            for (int i = 11; i >= 0; i--)
                if (destination.x == position_hovering[i].x &&
                    destination.z == position_hovering[i].z)
                {
                    if (i == 11)
                    {
                        float theta = GetTargetInfo().cos��_by_left;
                        owner->rotation.y += 30 * theta * deltaTime;
                        if (GetTargetInfo().position_type == TARGET_INFO::POSITON_TYPE::FRONT)
                            StartHoveringAndFire();
                    }

                    else
                        destination = position_hovering[i + 1];

                    break;
                }
    }
    else
    {
        // �ړI�n�Ɍ������Ĉړ���
        const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };
        const vec3 nv = v * (1 / sqrt(d2));
        const float sin�� = cross(dirFront, nv).y;
        owner->rotation.y += 5 * sin�� * deltaTime;
        characterMovement->AccelerateXZ(30 * nv * deltaTime, 6);
        characterMovement->AccelerateY(30 * nv * deltaTime, 6);
    }
}

/// <summary>
/// �u�󒆑؍ݎ��ɉΉ��e�U���v����
/// </summary>
/// <param name="deltaTime"></param>
void Dragon::DoHoveringAndFire(float deltaTime)
{
    auto owner = GetOwner();

    // �A�j���[�V�������I��������
    if (!animator->IsPlaying())
    {
        // �u�󒆑؍݁v�A�j���[�V�������Đ�����
        animator->SetSpeed(SPEED_ANIMATION * 2);
        animator->Play("walk");

        // ��Ԃ��u�󒆑؍݁v�ɂ���
        state = STATE_DRAGON::HOVERING;

        return;
    }

    // ��~����
    characterMovement->DecelerateXZ(DECELERATION * 10 * deltaTime);

    const vec3 v = destination - owner->position;
    const float d2 = dot(v, v);
    // ���V��
    const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };
    const vec3 nv = v * (1 / sqrt(d2));
    characterMovement->AccelerateY(30 * nv * deltaTime, 3);

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
void Dragon::DoDead(float deltaTime)
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
            float y = float(rand() % 6) * 0.05f + 0.25f;//  0.25�`0.5m
            float z = float(rand() % 5) * 0.25f - 0.5f; // -0.5�`+0.5m
            auto smoke = engine->Create<GameObject>("smoke", owner->position + vec3(x, y, z));
            smoke->AddComponent<Smoke>();
            float b = float(rand() % 5) * 0.25f + 0.25f;
            smoke->materials[0]->emission = vec3{ 0.125f, 0.15f, 0.1f } *b;
        }

        EasyAudio::PlayOneShot(SE::rock_delete);

        mistGenerator->GetOwner()->Destroy();

        // ���g��j������
        owner->Destroy();
    }
}

/// <summary>
/// �̗̓Q�[�W�̕\���E��\���A����
/// </summary>
/// <param name="engine">�Q�[���G���W��</param>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Dragon::HpGauge
(
    Engine* engine,
    float deltaTime
)
{
    // �v���C���[���߂Â�����A�{�X�̗̑͂�\������
    const auto targetInfo = GetTargetInfo();
    if (GetTarget()->GetComponent<PlayerComponent>()->GetStatePlayer() == PlayerComponent::STATE_PLAYER::ALIVE && targetInfo.distance <= DISTANCE_SEARCH)
    {
        // �̗̓Q�[�W�̃t���[���𐶐����Ă��Ȃ�������
        if (!ui_hp_frame)
        {
            // �̗̓Q�[�W�̃t���[���𐶐�����
            ui_hp_frame = engine->CreateUIObject<UILayout>("Res/boss_hp_frame.dds", { 0.00f, -0.80f }, 0.05f).second;
            ui_hp_frame->GetOwner()->render_queue = RENDER_QUEUE_OVERLAY + 1;
        }
        // �̗̓Q�[�W�𐶐����Ă��Ȃ�������
        if (!ui_hp_gauge)
        {
            // �̗̓Q�[�W�𐶐�����
            ui_hp_gauge = engine->CreateUIObject<UILayout>("Res/boss_hp_gauge.dds", { 0.00f, -0.80f }, 0.05f).second;
            ui_hp_gauge->GetOwner()->materials[0]->baseColor = { 1.0f, 0.0f, 0.0f, 1 };
        }

        // �̗̓Q�[�W��\������
        auto gauge = ui_hp_gauge->GetOwner();
        const float aspectRatio = gauge->materials[0]->texBaseColor->GetAspectRatio();
        const float targetRatio = std::clamp(float(Dragon::GetHp()) / float(HP_MAX), 0.0f, 1.0f);

        // �̗̓Q�[�W�̑������v�Z������
        if (current_ratio > targetRatio)
            current_ratio = std::max(current_ratio - deltaTime * 0.5f, targetRatio);
        else if (current_ratio < targetRatio)
            current_ratio = std::min(current_ratio + deltaTime * 0.5f, targetRatio);

        // �̗̓Q�[�W�S�̂̃X�P�[�����O
        switch (mode_scaling)
        {
        case MODE_SCALING::OPEN:
            scale_hp_gauge += deltaTime * 1.5f;
            if (scale_hp_gauge >= 1)
            {
                scale_hp_gauge = 1;
                mode_scaling = MODE_SCALING::STATIONARY;
            }
            break;
        case MODE_SCALING::STATIONARY:
            if (current_ratio <= 0)
                mode_scaling = MODE_SCALING::CLOSE;
            break;

        case MODE_SCALING::CLOSE:
            scale_hp_gauge = std::max(scale_hp_gauge - deltaTime * 1.5f, 0.0f);
            break;
        }

        // �v�Z���ʂ�̗̓Q�[�W�̍��W�A�傫���ɔ��f����
        ui_hp_gauge->position_base.x = -(1 - current_ratio) * aspectRatio * scale_hp_gauge * 0.05f;
        gauge->scale.x = current_ratio * aspectRatio * scale_hp_gauge * 0.05f;
        ui_hp_frame->GetOwner()->scale.x = aspectRatio * scale_hp_gauge * 0.05f;
    }
}
