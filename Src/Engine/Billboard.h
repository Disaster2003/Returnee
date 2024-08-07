/**
* @file Billboard.h
*/
#ifndef BILLBOARD_H_INCLUDED
#define BILLBOARD_H_INCLUDED
#include "Component.h"
#include "Engine.h"

/// <summary>
/// �r���{�[�h���R���|�[�l���g
/// </summary>
class Billboard
	:public Component
{
public:
	// �f�t�H���g�R���X�g���N�^
	inline Billboard() = default;
	// �f�t�H���g�f�X�g���N�^
	inline virtual ~Billboard() = default;

	/// <summary>
	/// �r���{�[�h��
	/// </summary>
	/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
	virtual void Update(float deltaTime)override
	{
		// ��ɃJ�����̕��������悤�Ƀ��b�Z�[�W�I�u�W�F�N�g�̌����𒲐�
		auto owner = GetOwner();
		const auto engine = owner->GetEngine();
		const auto& camera = engine->GetMainCamera();
		owner->rotation.y = camera.rotation.y;
	}
};

#endif // !BILLBOARD_H_INCLUDED