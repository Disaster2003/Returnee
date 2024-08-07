/**
* @file AttackCollider.cpp
*/
#include "AttackCollider.h"
#include "../Engine/Engine.h"

/// <summary>
/// コライダーを初期化する
/// </summary>
void AttackCollider::Awake()
{
    // コライダーを設定する
    collider = GetOwner()->AddComponent<SphereCollider>();
    collider->isTrigger = true;
    collider->isStatic = true;
}

/// <summary>
/// 攻撃用のコライダー状態を更新する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void AttackCollider::Update(float deltaTime)
{
    // 起動していなければ何もしない
    if (state == STATE_ATTACK_COLLIDER::SLEEP)
        return;

    // 経過時間を更新する
    timer += deltaTime;

    switch (state)
    {
    default:
    case STATE_ATTACK_COLLIDER::SLEEP:
        if (timer >= 0.3f)
            state = STATE_ATTACK_COLLIDER::COLLISION_ENABLE;
        break;
    case STATE_ATTACK_COLLIDER::COLLISION_ENABLE:
        if (timer >= 0.5f)
            state = STATE_ATTACK_COLLIDER::COLLISION_DISABLE;
        break;
    case STATE_ATTACK_COLLIDER::COLLISION_DISABLE:
        if (timer >= 0.6f)
            Deactivate();
        break;
    }
}

/// <summary>
/// 攻撃処理を起動する
/// </summary>
/// <param name="collider_sphere_radius">攻撃範囲</param>
void AttackCollider::Activate(float collider_sphere_radius)
{
    if (state == STATE_ATTACK_COLLIDER::SLEEP)
    {
        state = STATE_ATTACK_COLLIDER::COLLISION_ENABLE;
        timer = 0;
        collider->sphere.radius = collider_sphere_radius;
    }
}
