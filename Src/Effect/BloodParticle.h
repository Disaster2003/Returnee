/**
* @file BloodParticle.h
*/
#ifndef BLOODPARTICLE_H_INCLUDED
#define BLOODPARTICLE_H_INCLUDED
#include "Particle.h"

/// <summary>
/// �����Ԃ�
/// </summary>
class BloodParticle
    : public Particle
{
public:
    // �R���X�g���N�^
    BloodParticle()
        :Particle(LIFE_SPAN)
    {}
    // �f�t�H���g�f�X�g���N�^
    inline virtual ~BloodParticle() = default;

    /// <summary>
    /// �p�[�e�B�N��������������
    /// </summary>
    virtual void Awake() override
    {
        Particle::Awake();

        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        owner->materials[0]->texBaseColor = engine->GetTexture("Res/damage_particle.tga");
        owner->materials[0]->texEmission = engine->GetTexture("Res/damage_particle.tga");
        owner->materials[0]->baseColor = { 0.2f, 0.5f, 1.0f, 1 };
        owner->materials[0]->emission = { 0.2f, 0.5f, 1 };
        owner->scale = vec3(0.1f + static_cast<float>(rand() % 4) * 0.1f);
        owner->rotation.z = static_cast<float>(rand() % 10) * 0.63f;
        owner->position.x += static_cast<float>(rand() % 10) * 0.02f - 0.1f;
        owner->position.z += static_cast<float>(rand() % 10) * 0.02f - 0.1f;
        velocity.x = static_cast<float>(rand() % 7) - 3;
        velocity.y = static_cast<float>(rand() % 7) - 3;
        velocity.z = static_cast<float>(rand() % 7) - 3;
    }

    /// <summary>
    /// ��Ɍ����Ԃ��̏�Ԃ��X�V����
    /// </summary>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        velocity.y -= deltaTime;
        owner->position += velocity * deltaTime;          // ��Ɉړ�
        owner->rotation.z += random_rotation * deltaTime; // ��]
        owner->scale += vec3(deltaTime);           // ���X�Ɋg��
        owner->color.w =
            std::min(BloodParticle::GetLifespan() * 8.0f, 1.0f); // ���X�ɓ�����

        // �����Ԃ��̎������X�V����
        Particle::Update(deltaTime);
    }

private:
    vec3 velocity = vec3(0);
    static constexpr float rotationList[] = { -2.0f, -1.0f, -0.5f, 0.5f, 1.0f, 2.0f };
    float random_rotation = rotationList[rand() % std::size(rotationList)];
    static constexpr float LIFE_SPAN = 0.25f; // �����Ԃ��̎���(s)
};

#endif // !BLOODPARTICLE_H_INCLUDED
