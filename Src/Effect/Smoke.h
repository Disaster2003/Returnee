/**
* @file Smoke.h
*/
#ifndef SMOKE_H_INCLUDED
#define SMOKE_H_INCLUDED
#include "Particle.h"

/// <summary>
/// ���̗��q
/// </summary>
class Smoke
    : public Particle
{
public:
    // �R���X�g���N�^
    Smoke()
        :Particle(LIFE_SPAN)
    {}
    // �f�t�H���g�f�X�g���N�^
    virtual ~Smoke() = default;

    /// <summary>
    /// �p�[�e�B�N��������������
    /// </summary>
    virtual void Awake() override
    {
        Particle::Awake();

        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        owner->materials[0]->texBaseColor = engine->GetTexture("Res/particle_fire.tga");
        owner->materials[0]->baseColor = { 0.05f, 0.025f, 0.05f, 1 }; // ���C�g�̉e�����Ȃ���
        owner->materials[0]->emission = { 0.05f, 0.025f, 0.05f };
        owner->scale = vec3(0.7f + static_cast<float>(rand() % 4) * 0.1f);
        owner->rotation.z = static_cast<float>(rand() % 10) * 0.63f;
        owner->position.x += static_cast<float>(rand() % 10) * 0.02f - 0.1f;
        owner->position.z += static_cast<float>(rand() % 10) * 0.02f - 0.1f;
    }

    /// <summary>
    /// ��ɉ��̗��q�̏�Ԃ��X�V����
    /// </summary>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        owner->position.y += 0.5f * deltaTime;        // ��Ɉړ�
        owner->rotation.z += 0.57f * deltaTime;       // ��]
        owner->scale += vec3(0.75f * deltaTime);      // ���X�Ɋg��
        owner->color.w =
            std::min(Smoke::GetLifespan() * 2, 1.0f); // ���X�ɓ�����

        Particle::Update(deltaTime);
    }

private:
    static constexpr float LIFE_SPAN = 1.0f; // ���̎���(s)
};

#endif // !SMOKE_H_INCLUDED
