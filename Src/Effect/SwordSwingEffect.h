/**
* @file SwordSwingEffect.h
*/
#ifndef SWORDSWINGEFFECT_H_INCLUDED
#define SWORDSWINGEFFECT_H_INCLUDED
#include "Particle.h"

/// <summary>
/// �a����
/// </summary>
class SwordSwingEffect
	: public Particle
{
public:
	// �R���X�g���N�^
	SwordSwingEffect()
		:Particle(LIFE_SPAN)
	{}
	// �f�t�H���g�f�X�g���N�^
	inline virtual ~SwordSwingEffect() = default;

    /// <summary>
    /// �p�[�e�B�N��������������
    /// </summary>
    virtual void Awake() override
    {
        Particle::Awake();

        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        owner->materials[0]->texBaseColor = engine->GetTexture("Res/effect_swing.tga");
        owner->materials[0]->texEmission = engine->GetTexture("Res/effect_swing.tga");
        owner->materials[0]->baseColor = { 0.2f, 0.5f, 1.5f, 1 };
        owner->materials[0]->emission = { 0.2f, 0.5f, 1.5f };
        owner->scale.x *= 0.1f;
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
	static constexpr float LIFE_SPAN = 0.5f; // �����Ԃ��̎���(s)
};

#endif // !SWORDSWINGEFFECT_H_INCLUDED
