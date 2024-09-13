/**
* @file PlayerActionSuccessParticle.h
*/
#ifndef PLAYERACTIONSUCCESSPARTICLE_H_INCLUDED
#define PLAYERACTIONSUCCESSPARTICLE_H_INCLUDED
#include "Particle.h"
#include "../Engine/Debug.h"

/// <summary>
/// �v���C���[�̍s�������������ɏo���p�[�e�B�N��
/// </summary>
class PlayerActionSuccessParticle
    : public Particle
{
public:
    // �R���X�g���N�^
    PlayerActionSuccessParticle()
        :Particle(LIFE_SPAN)
    {}
    // �f�t�H���g�f�X�g���N�^
    inline virtual ~PlayerActionSuccessParticle() = default;

    /// <summary>
    /// �p�[�e�B�N��������������
    /// </summary>
    virtual void Awake() override
    {
        Particle::Awake();

        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        // null�`�F�b�N
        if (!owner || !engine)
        {
            LOG_WARNING("�v���C���[�p�̉��o�����݂��܂���");
            return;
        }
        owner->materials[0]->baseColor = { 0.2f, 0.5f, 1.5f, 1 };
        owner->materials[0]->emission = { 0.2f, 0.5f, 1.5f };
        owner->scale = vec3(0.1f + static_cast<float>(rand() % 4) * 0.1f);
        owner->rotation.z = static_cast<float>(rand() % 10) * 0.63f;
        owner->position.x += static_cast<float>(rand() % 10) * 0.02f - 0.1f;
        owner->position.z += static_cast<float>(rand() % 10) * 0.02f - 0.1f;
        velocity.x = static_cast<float>(rand() % 7) - 3;
        velocity.y = static_cast<float>(rand() % 7) - 3;
        velocity.z = static_cast<float>(rand() % 7) - 3;
    }

    /// <summary>
    /// ��Ƀp�[�e�B�N���̏�Ԃ��X�V����
    /// </summary>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        // null�`�F�b�N
        if (!owner)
        {
            LOG_WARNING("�v���C���[�p�̉��o�����݂��܂���");
            return;
        }
        velocity.y -= 9.81f * deltaTime;
        owner->position += velocity * deltaTime;          // ��Ɉړ�
        owner->rotation.z += random_rotation * deltaTime; // ��]
        owner->scale += vec3(0.25f * deltaTime);          // ���X�Ɋg��
        owner->color.w =
            std::min(PlayerActionSuccessParticle::GetLifespan() * 8.0f, 1.0f); // ���X�ɓ�����

        // �p�[�e�B�N���̎������X�V����
        Particle::Update(deltaTime);
    }

private:
    vec3 velocity = vec3(0);
    static constexpr float rotationList[] = { -2.0f, -1.0f, -0.5f, 0.5f, 1.0f, 2.0f };
    float random_rotation = rotationList[rand() % std::size(rotationList)];
    static constexpr float LIFE_SPAN = 0.25f; // �p�[�e�B�N���̎���(s)
};

#endif // !PLAYERACTIONSUCCESSPARTICLE_H_INCLUDED
