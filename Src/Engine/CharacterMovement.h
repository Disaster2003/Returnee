/**
* @file CharacterMovement.h
*/
#ifndef CHARACTERMOVEMENT_H_INCLUDED
#define CHARACTERMOVEMENT_H_INCLUDED
#include "GameObject.h"

/// <summary>
/// ゲームオブジェクトの速度と重力を扱うコンポーネント
/// </summary>
class CharacterMovement
    : public Component
{
public:
    // デフォルトコンストラクタ
    inline CharacterMovement() = default;
    // デフォルトデストラクタ
    inline virtual ~CharacterMovement() = default;

    /// <summary>
    /// 常に重力による影響を更新する
    /// </summary>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();

        // 接地していた場合
        if (owner->isGrounded)
        {
            // 下方向への加速度を0にする
            velocity.y = std::max(velocity.y, 0.0f);
        }
        else
        {
            // 重力加速度によって速度を更新する
            velocity.y -= gravity * gravity_scale * deltaTime;
        }

        // 速度を座標に反映する
        owner->position += velocity * deltaTime;
    }

    /// <summary>
    /// 加速する
    /// </summary>
    /// <param name="accel">加速度</param>
    /// <param name="speed_max">MAXの速度</param>
    void AccelerateY
    (
        const vec3& accel,
        float speed_max
    )
    {
        float vy = velocity.y + accel.y;
        float vd = sqrt(vy * vy);
        if (vd > speed_max)
        {
            vy *= speed_max / vd;
        }
        velocity.y = vy;
    }

    /// <summary>
    /// 加速する
    /// </summary>
    /// <param name="accel">加速度</param>
    /// <param name="speed_max">MAXの速度</param>
    void AccelerateXZ
    (
        const vec3& accel,
        float speed_max
    )
    {
        float vx = velocity.x + accel.x;
        float vz = velocity.z + accel.z;
        float vd = sqrt(vx * vx + vz * vz);
        if (vd > speed_max)
        {
            vx *= speed_max / vd;
            vz *= speed_max / vd;
        }
        velocity.x = vx;
        velocity.z = vz;
    }

    /// <summary>
    /// 減速する
    /// </summary>
    /// <param name="speed_deceleration">減速度</param>
    void DecelerateXZ(float speed_deceleration)
    {
        float vd = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
        if (vd <= speed_deceleration)
        {
            velocity.x = 0;
            velocity.z = 0;
        }
        else
        {
            const float a = (vd - speed_deceleration) / vd;
            velocity.x *= a;
            velocity.z *= a;
        }
    }

    vec3 velocity = vec3(0);
    static constexpr float gravity = 9.81f;      // 重力加速度
    static constexpr float gravity_scale = 1.0f; // 重力の影響を制御する係数
};
using CharacterMovementPtr = std::shared_ptr<CharacterMovement>;

#endif // CHARACTERMOVEMENT_H_INCLUDED