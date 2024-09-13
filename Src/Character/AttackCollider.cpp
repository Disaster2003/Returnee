/**
* @file AttackCollider.cpp
*/
#include "AttackCollider.h"
#include "../Engine/Engine.h"
#include "../Engine/Debug.h"

/// <summary>
/// �R���C�_�[������������
/// </summary>
void AttackCollider::Awake()
{
    // �R���C�_�[�̐ݒ�
    collider = GetOwner()->AddComponent<SphereCollider>();
    collider->isTrigger = true;
    collider->isStatic = true;
}

/// <summary>
/// �U���p�̃R���C�_�[��Ԃ��X�V����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void AttackCollider::Update(float deltaTime)
{
    // �N�����Ă��Ȃ���Ή������Ȃ�
    if (state == STATE_ATTACK_COLLIDER::SLEEP)
    {
        return;
    }

    // �o�ߎ��Ԃ��X�V����
    timer += deltaTime;

    switch (state)
    {
    default:
    case STATE_ATTACK_COLLIDER::SLEEP:
        // �R���C�_�[�N��
        if (timer >= 0.3f)
        {
            state = STATE_ATTACK_COLLIDER::COLLISION_ENABLE;
        }
        break;
    case STATE_ATTACK_COLLIDER::COLLISION_ENABLE:
        // �R���C�_�[��~
        if (timer >= 0.5f)
        {
            state = STATE_ATTACK_COLLIDER::COLLISION_DISABLE;
        }
        break;
    case STATE_ATTACK_COLLIDER::COLLISION_DISABLE:
        // �R���C�_�[Off
        if (timer >= 0.6f)
        {
            Deactivate();
        }
        break;
    }
}

/// <summary>
/// �U���������N������
/// </summary>
/// <param name="collider_sphere_radius">�U���͈�</param>
void AttackCollider::Activate(float collider_sphere_radius)
{
    // null�`�F�b�N
    if (!collider)
    {
        LOG_WARNING("�R���C�_�[�����݂��܂���");
        return;
    }

    if (state == STATE_ATTACK_COLLIDER::SLEEP)
    {
        state = STATE_ATTACK_COLLIDER::COLLISION_ENABLE;
        timer = 0;

        // �U���͈͂̐ݒ�
        collider->sphere.radius = collider_sphere_radius;
    }
}
