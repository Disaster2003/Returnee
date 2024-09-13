/**
* @file CharacterMovement.h
*/
#ifndef CHARACTERMOVEMENT_H_INCLUDED
#define CHARACTERMOVEMENT_H_INCLUDED
#include "GameObject.h"

/// <summary>
/// �Q�[���I�u�W�F�N�g�̑��x�Əd�͂������R���|�[�l���g
/// </summary>
class CharacterMovement
    : public Component
{
public:
    // �f�t�H���g�R���X�g���N�^
    inline CharacterMovement() = default;
    // �f�t�H���g�f�X�g���N�^
    inline virtual ~CharacterMovement() = default;

    /// <summary>
    /// ��ɏd�͂ɂ��e�����X�V����
    /// </summary>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();

        // �ڒn���Ă����ꍇ
        if (owner->isGrounded)
        {
            // �������ւ̉����x��0�ɂ���
            velocity.y = std::max(velocity.y, 0.0f);
        }
        else
        {
            // �d�͉����x�ɂ���đ��x���X�V����
            velocity.y -= gravity * gravity_scale * deltaTime;
        }

        // ���x�����W�ɔ��f����
        owner->position += velocity * deltaTime;
    }

    /// <summary>
    /// ��������
    /// </summary>
    /// <param name="accel">�����x</param>
    /// <param name="speed_max">MAX�̑��x</param>
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
    /// ��������
    /// </summary>
    /// <param name="accel">�����x</param>
    /// <param name="speed_max">MAX�̑��x</param>
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
    /// ��������
    /// </summary>
    /// <param name="speed_deceleration">�����x</param>
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
    static constexpr float gravity = 9.81f;      // �d�͉����x
    static constexpr float gravity_scale = 1.0f; // �d�͂̉e���𐧌䂷��W��
};
using CharacterMovementPtr = std::shared_ptr<CharacterMovement>;

#endif // CHARACTERMOVEMENT_H_INCLUDED