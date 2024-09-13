/**
* @file Enemy.cpp
*/
#include "Enemy.h"
#include "../Player/PlayerComponent.h"
#include "MagicMissile.h"

/// <summary>
/// 魔法攻撃を行う
/// </summary>
/// <param name="_owner">攻撃者</param>
void Enemy::AttackMagicMissile(GameObject& _owner)
{
    const vec3 direction_front_of_head = { sin(_owner.rotation.y), 1.5f, cos(_owner.rotation.y) };
    const vec3 position_spawn = _owner.position + direction_front_of_head;

    // 発射角度を前方左右60度に制限する
    constexpr vec3 targetOffset = { 0, 1, 0 }; // プレイヤーの胸の高さを狙う
    vec3 v = normalize(GetTarget()->position + targetOffset - position_spawn);
    const float sinθ = cross(vec3(direction_front_of_head.x, 0, direction_front_of_head.z), vec3(v.x, 0, v.z)).y;
    static const float maxSinθ = sin(radians(60));
    if (sinθ > maxSinθ)
    {
        const float s = sin(radians(60));
        const float c = cos(radians(60));
        v.x =  direction_front_of_head.x * c + direction_front_of_head.z * s;
        v.z = -direction_front_of_head.x * s + direction_front_of_head.z * c;
    }
    else if (sinθ < -maxSinθ)
    {
        const float s = sin(radians(-60));
        const float c = cos(radians(-60));
        v.x =  direction_front_of_head.x * c + direction_front_of_head.z * s;
        v.z = -direction_front_of_head.x * s + direction_front_of_head.z * c;
    }
    const float a = atan2(-v.z, v.x);

    // 魔法弾を発射する
    const float angles[] = { 0, 0.4f, -0.4f };
    auto go = _owner.GetEngine()->Create<GameObject>("Magic Missile", position_spawn);
    auto missile = go->AddComponent<MagicMissile>();
    float c = cos(a + angles[0]);
    float s = sin(a + angles[0]);
    v.x = c;
    v.z = -s;
    missile->velocity = v * SPEED_MAGICMISSILE;

    // ダメージ量を設定する
    missile->SetDamage(ATTAK_NORMAL);
}

/// <summary>
/// 攻撃対象までの距離と向きを取得する
/// </summary>
/// <returns>プレイヤーまでの距離と向き</returns>
Enemy::TARGET_INFO Enemy::GetTargetInfo() const
{
    // 攻撃対象がいなければ、何もしない
    if (!target)
    {
        return {};
    }

    TARGET_INFO targetInfo;
    const auto owner = GetOwner();

    // 攻撃対象との距離を計測する
    vec3 v = target->position - owner->position;
    targetInfo.distance = length(v);
    v *= 1 / targetInfo.distance;

    // 正面と左の向きを確認する
    const float s = sin(owner->rotation.y);
    const float c = cos(owner->rotation.y);
    targetInfo.cosθ_by_front = dot({ s, 0, c }, v);
    targetInfo.cosθ_by_left = dot({ c, 0, -s }, v);
    if (targetInfo.cosθ_by_front < 0)
        targetInfo.position_type = TARGET_INFO::POSITON_TYPE::BACK;
    else
    {
        constexpr float frontRange = 0.17365f; // cos(radians(80))
        if (targetInfo.cosθ_by_left > frontRange)
        {
            targetInfo.position_type = TARGET_INFO::POSITON_TYPE::RIGHT;
        }
        else if (targetInfo.cosθ_by_left < -frontRange)
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