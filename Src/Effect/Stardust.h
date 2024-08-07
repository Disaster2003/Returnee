/**
* @file Stardust.h
*/
#ifndef STARDUST_H_INCLUDED
#define STARDUST_H_INCLUDED
#include "Particle.h"
#include "../Engine/Random.h"

// �L���L��
class Stardust
	: public Particle
{
public:
	// �R���X�g���N�^
	Stardust()
		:Particle(LIFE_SPAN)
	{}
	// �f�t�H���g�f�X�g���N�^
	virtual ~Stardust() = default;

	/// <summary>
	/// �p�[�e�B�N��������������
	/// </summary>
	virtual void Awake() override
	{
		// �p�[�e�B�N���̋��ʍ�������������
		Particle::Awake();

		// �p�[�e�B�N���̑���_������������
		auto owner = GetOwner();
		auto engine = owner->GetEngine();
		owner->materials[0]->texBaseColor = engine->GetTexture("Res/particle_star.tga");
		owner->materials[0]->baseColor = { 0, 0, 0, 1 };      // ���C�g�̉e�����Ȃ���
		owner->materials[0]->emission = { 3.0f, 3.0f, 0.2f }; // ������
		owner->position.y++; // ��Ɉړ�
		owner->scale = vec3(0.25f);
	}

	/// <summary>
	/// ��Ƀp�[�e�B�N�����X�V����
	/// </summary>
	/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
	virtual void Update(float deltaTime) override
	{
		auto owner = GetOwner();
		auto engine = owner->GetEngine();

		owner->position += delta_position * deltaTime;    // ��Ɉړ�
		owner->rotation.z += float(sin(120)) * deltaTime; // ��]
		owner->color.w -= deltaTime;					  // ���X�ɓ�����

		// ���������炷
		Particle::Update(deltaTime);
	}

private:
	// �p�[�e�B�N���̈ړ���
	vec3 delta_position = { Random::Range(-1.0f,1.0f),2,Random::Range(-1.0f,1.0f) };
	static constexpr float LIFE_SPAN = 1.0f; // �L���L���̎���(s)
};

#endif // !STARDUST_H_INCLUDED
