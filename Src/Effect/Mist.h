/**
* @file Mist.h
*/
#ifndef MIST_H_INCLUDED
#define MIST_H_INCLUDED
#include "../Engine/Component.h"
#include "../Engine/GameObject.h"
#include "../Engine/Billboard.h"
#include "../Engine/Random.h"
#include "../Engine/Engine.h"
#include "../Engine/Debug.h"

/// <summary>
/// ���R���|�[�l���g
/// </summary>
class Mist
	:public Component
{
	friend class MistGenerator;

public:
	// �f�t�H���g�R���X�g���N�^
	inline Mist() = default;
	// �f�t�H���g�f�X�g���N�^
	inline virtual ~Mist() = default;

	/// <summary>
	/// ��ɖ��̏�Ԃ��X�V����
	/// </summary>
	/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
	virtual void Update(float deltaTime) override
	{
		// ��莞�Ԍo������
		timer += deltaTime;
		if (timer >= life_span)
		{
			// ���g��j������
			GetOwner()->Destroy();
		}

		// �����������ȉ��Ȃ�
		if (timer >= life_span / 2)
		{
			// �����ɂ��Ă���
			GetOwner()->color[3] = (life_span - timer) / (life_span / 2);
		}
		else
		{
			// �s�����ɂ��Ă���
			GetOwner()->color[3] = timer / (life_span / 2);
		}
	}

private:
	float timer = 0;	// �o�ߎ��ԃ^�C�}�[
	float life_span = 7;	// ����(�b)
};

/// <summary>
/// ���𔭐�������R���|�[�l���g
/// </summary>
class MistGenerator
	:public Component
{
public:
	// �f�t�H���g�R���X�g���N�^
	inline MistGenerator() = default;
	// �f�t�H���g�f�X�g���N�^
	inline virtual ~MistGenerator() = default;

	/// <summary>
	/// ���}�e���A��������������
	/// </summary>
	inline virtual void Awake() override
	{
		// ���}�e���A�����쐬����
		mistMaterial = CloneMaterialList(
			GetOwner()->GetEngine()->GetStaticMesh("plane_xy"));
		mistMaterial[0]->texBaseColor = GetOwner()->GetEngine()->GetTexture("Res/effect_mist.tga");
	}

	/// <summary>
	/// ���������u�̓���
	/// </summary>
	/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
	virtual void Update(float deltaTime) override
	{
		// �C���^�[�o����
		timer += deltaTime;
		if (timer < time_particle)
		{
			return;
		}

		// �C���^�[�o�����ɂ���
		timer -= time_particle;

		auto owner = GetOwner();
		auto engine = owner->GetEngine();
		// null�`�F�b�N
		if (!owner || !engine)
		{
			LOG_WARNING("�������݂��܂���");
			return;
		}

		// ���𔭐�������ʒu�������_���ɑI��
		vec3 pos = owner->position;
		pos.x += Random::Range(-range.x, range.x);
		pos.y += 0.15f;	// ���������ɕ\�������悤�ɍ����𒲐�
		pos.z += Random::Range(-range.y, range.y);

		// ���𐶐�
		auto mist = engine->Create<GameObject>("mist", pos);
		mist->scale = { Random::Range(0.7f,1.5f),Random::Range(0.7f,1.5f) };
		mist->staticMesh = engine->GetStaticMesh("plane_xy");
		mist->materials = mistMaterial;
		mist->render_queue = RENDER_QUEUE_TRANSPARENT;
		mist->AddComponent<Billboard>();
		mist->AddComponent<Mist>();
	}

	vec2 range = { 1,1 };		// XZ�����̔������a
	float time_particle = 1.0f;	// ���𔭐�����Ԋu

private:
	MaterialList mistMaterial;	// ���}�e���A��
	float timer = 0;			// �������^�C�}�[
};

#endif // !MIST_H_INCLUDED