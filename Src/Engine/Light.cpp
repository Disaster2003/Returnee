/**
* @file Light.cpp
*/
#include "Light.h"
#include "Engine.h"

/// <summary>
/// �f�X�g���N�^
/// </summary>
Light::~Light()
{
	// ���C�g���������
	GetOwner()->GetEngine()->DeallocateLight(index_light);
}

/// <summary>
/// ���C�g������������
/// </summary>
void Light::Awake()
{
	// ���C�g���擾���Ă��Ȃ���΁A���C�g���擾����
	if (index_light < 0)
	{
		index_light = GetOwner()->GetEngine()->AllocateLight();
	}
}

/// <summary>
/// ��Ƀ��C�g�̏�Ԃ��X�V����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Light::Update(float deltaTime)
{
	// �R���|�[�l���g�̒l�����C�g�ɔ��f����
	auto owner = GetOwner();
	auto light = owner->GetEngine()->GetLight(index_light);
	if (light)
	{
		light->position = owner->position;
		light->intensity = intensity;
		light->color = color;
		light->radius = radius;
		light->coneAngle = 0; // �Ǝˊp�x��0�ȉ��Ȃ�|�C���g���C�g����

		// �X�|�b�g���C�g�̏ꍇ�A�X�|�b�g���C�g�̃p�����[�^�𔽉f����
		if (type == TYPE_LIGHT::SPOT)
		{
			// ���C�g�̌������v�Z
			vec3 direction = { 0,0,-1 }; // ��]�Ȃ��̌����x�N�g��

			// X����]
			const float sinX = sin(owner->rotation.x);
			const float cosX = cos(owner->rotation.x);
			direction =
			{
				direction.x,
				direction.y * cosX - direction.z * sinX,
				direction.y * sinX + direction.z * cosX
			};

			// Y����]
			const float sinY = sin(owner->rotation.y);
			const float cosY = cos(owner->rotation.y);
			direction =
			{
				direction.x * cosY + direction.z * sinY,
				direction.y,
				-direction.x * sinY + direction.z * cosY
			};

			// �p�����[�^�𔽉f����
			light->direction = direction;
			light->coneAngle = angle_cone;
			light->falloffAngle = angle_falloff;
		}
	}
}
