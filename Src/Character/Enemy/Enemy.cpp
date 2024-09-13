/**
* @file Enemy.cpp
*/
#include "Enemy.h"
#include "../Player/PlayerComponent.h"
#include "MagicMissile.h"

/// <summary>
/// ���@�U�����s��
/// </summary>
/// <param name="_owner">�U����</param>
void Enemy::AttackMagicMissile(GameObject& _owner)
{
    const vec3 direction_front_of_head = { sin(_owner.rotation.y), 1.5f, cos(_owner.rotation.y) };
    const vec3 position_spawn = _owner.position + direction_front_of_head;

    // ���ˊp�x��O�����E60�x�ɐ�������
    constexpr vec3 targetOffset = { 0, 1, 0 }; // �v���C���[�̋��̍�����_��
    vec3 v = normalize(GetTarget()->position + targetOffset - position_spawn);
    const float sin�� = cross(vec3(direction_front_of_head.x, 0, direction_front_of_head.z), vec3(v.x, 0, v.z)).y;
    static const float maxSin�� = sin(radians(60));
    if (sin�� > maxSin��)
    {
        const float s = sin(radians(60));
        const float c = cos(radians(60));
        v.x =  direction_front_of_head.x * c + direction_front_of_head.z * s;
        v.z = -direction_front_of_head.x * s + direction_front_of_head.z * c;
    }
    else if (sin�� < -maxSin��)
    {
        const float s = sin(radians(-60));
        const float c = cos(radians(-60));
        v.x =  direction_front_of_head.x * c + direction_front_of_head.z * s;
        v.z = -direction_front_of_head.x * s + direction_front_of_head.z * c;
    }
    const float a = atan2(-v.z, v.x);

    // ���@�e�𔭎˂���
    const float angles[] = { 0, 0.4f, -0.4f };
    auto go = _owner.GetEngine()->Create<GameObject>("Magic Missile", position_spawn);
    auto missile = go->AddComponent<MagicMissile>();
    float c = cos(a + angles[0]);
    float s = sin(a + angles[0]);
    v.x = c;
    v.z = -s;
    missile->velocity = v * SPEED_MAGICMISSILE;

    // �_���[�W�ʂ�ݒ肷��
    missile->SetDamage(ATTAK_NORMAL);
}

/// <summary>
/// �U���Ώۂ܂ł̋����ƌ������擾����
/// </summary>
/// <returns>�v���C���[�܂ł̋����ƌ���</returns>
Enemy::TARGET_INFO Enemy::GetTargetInfo() const
{
    // �U���Ώۂ����Ȃ���΁A�������Ȃ�
    if (!target)
    {
        return {};
    }

    TARGET_INFO targetInfo;
    const auto owner = GetOwner();

    // �U���ΏۂƂ̋������v������
    vec3 v = target->position - owner->position;
    targetInfo.distance = length(v);
    v *= 1 / targetInfo.distance;

    // ���ʂƍ��̌������m�F����
    const float s = sin(owner->rotation.y);
    const float c = cos(owner->rotation.y);
    targetInfo.cos��_by_front = dot({ s, 0, c }, v);
    targetInfo.cos��_by_left = dot({ c, 0, -s }, v);
    if (targetInfo.cos��_by_front < 0)
        targetInfo.position_type = TARGET_INFO::POSITON_TYPE::BACK;
    else
    {
        constexpr float frontRange = 0.17365f; // cos(radians(80))
        if (targetInfo.cos��_by_left > frontRange)
        {
            targetInfo.position_type = TARGET_INFO::POSITON_TYPE::RIGHT;
        }
        else if (targetInfo.cos��_by_left < -frontRange)
        {
            targetInfo.position_type = TARGET_INFO::POSITON_TYPE::LEFT;
        }
        else
        {
            targetInfo.position_type = TARGET_INFO::POSITON_TYPE::FRONT;
        }
    }

    return targetInfo;
}