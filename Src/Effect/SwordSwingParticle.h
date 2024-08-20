/**
* @file SwordSwingParticle.h
*/
#ifndef SWORDSWINGPARTICLE_H_INCLUDED
#define SWORDSWINGPARTICLE_H_INCLUDED
#include "Particle.h"

class SwordSwingParticle
	: public Particle
{
public:
	// �R���X�g���N�^
	SwordSwingParticle()
		:Particle(LIFE_SPAN)
	{}
	// �f�t�H���g�f�X�g���N�^
	inline virtual ~SwordSwingParticle() = default;

    /// <summary>
    /// �p�[�e�B�N��������������
    /// </summary>
    virtual void Awake() override
    {
        Particle::Awake();

        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        owner->materials[0]->texBaseColor = engine->GetTexture("Res/swing_effect.tga");
        owner->materials[0]->texEmission = engine->GetTexture("Res/swing_effect.tga");
        owner->materials[0]->baseColor = { 0.2f, 0.5f, 1.0f, 1 };
        owner->materials[0]->emission = { 0.2f, 0.5f, 1 };
    }

    /// <summary>
    /// ��Ɍ����Ԃ��̏�Ԃ��X�V����
    /// </summary>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
    virtual void Update(float deltaTime) override
    {
        // �����Ԃ��̎������X�V����
        Particle::Update(deltaTime);
    }

private:
	static constexpr float LIFE_SPAN = 0.25f; // �����Ԃ��̎���(s)
};

#endif // !SWORDSWINGPARTICLE_H_INCLUDED
