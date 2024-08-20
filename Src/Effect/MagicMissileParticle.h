/**
* @file MagicMissileParticle.h
*/
#ifndef MAGICMISSILEPARTICLE_H_INCLUDED
#define MAGICMISSILEPARTICLE_H_INCLUDED
#include "Particle.h"

/// <summary>
/// �Ή��e�̋O�Ճp�[�e�B�N��
/// </summary>
class MagicMissileParticle
    : public Particle
{
public:
    // �R���X�g���N�^
    MagicMissileParticle()
        :Particle(LIFE_SPAN)
    {}
    // �f�t�H���g�f�X�g���N�^
    virtual ~MagicMissileParticle() = default;

    /// <summary>
    /// �p�[�e�B�N��������������
    /// </summary>
    virtual void Awake() override
    {
        Particle::Awake();

        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        owner->materials[0]->texBaseColor = engine->GetTexture("Res/particle_fire.tga");
        owner->materials[0]->baseColor = { 0, 0, 0, 1 }; // ���C�g�̉e�����Ȃ���
        owner->materials[0]->emission = { 2.0f, 1.0f, 0.5f };
        owner->scale = vec3(0.2f + static_cast<float>(rand() % 4) * 0.1f);
        owner->rotation.z = static_cast<float>(rand() % 10) * 0.63f;
    }

    /// <summary>
    /// ��ɉΉ��e�̋O�Ղ��X�V����
    /// </summary>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        owner->materials[0]->baseColor.w -= deltaTime; // ���X�ɓ�����
        // ���X�ɔ���
        owner->materials[0]->emission =
            vec3(2.0f, 1.0f, 0.5f) * (MagicMissileParticle::GetLifespan() * 4 + 0.2f);
        // X�������ɏk������
        owner->scale =
            vec3(std::max(owner->scale.x - deltaTime, 0.0f));
        owner->position.y += deltaTime * 5;

        Particle::Update(deltaTime);
    }

private:
    static constexpr float LIFE_SPAN = 0.25f; // �p�[�e�B�N���̎���(s)
};

#endif // !MAGICMISSILEPARTICLE_H_INCLUDED
