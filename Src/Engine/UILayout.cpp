/**
* @file UILayout.cpp
*/
#include "UILayout.h"
#include "Engine.h"

/// <summary>
/// UI���C�A�E�g�̎��s���J�n����
/// </summary>
void UILayout::Start()
{
	// �ŏ��Ɏw�肳�ꂽ���W���u�J�������猩�����W�v�Ƃ���
	position_base = GetOwner()->position;
}

/// <summary>
/// ��ɃJ�����̕��������悤��UI�𒲐�����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void UILayout::Update(float deltaTime)
{
	// ��ɃJ�����̕��������悤�ɃQ�[���I�u�W�F�N�g�̌����𒲐�(�r���{�[�h�Ɠ���)
	auto owner = GetOwner();
	const auto engine = owner->GetEngine();
	const auto& camera = engine->GetMainCamera();
	owner->rotation.y = camera.rotation.y;
	owner->rotation.x = camera.rotation.x;

	// ��ɃJ�����̐��ʂɈʒu����悤�ɃQ�[���I�u�W�F�N�g�̍��W�𒲐�
	// ��]�̌����ɂ���ăJ�����Ƃ̑��΍��W�����߂�
	vec3 pos = position_base; // �p�x0�x�̏ꍇ�̍��W

	// y���W��-1~+1���A�J�����Ɏʂ�͈͂̏�[�Ɖ��[�ɂȂ�悤�ɁAZ���W�𒲐�
	pos.z = -engine->GetFovScale();

	const float sx = sin(camera.rotation.x);
	const float cx = cos(camera.rotation.x);
	const float sy = sin(camera.rotation.y);
	const float cy = cos(camera.rotation.y);
	pos =
	{
	  pos.x,
	  pos.z * -sx + pos.y * cx,
	  pos.z * cx + pos.y * sx
	};
	pos =
	{
	  pos.x * cy + pos.z * sy,
	  pos.y,
	  pos.x * -sy + pos.z * cy
	};
	owner->position = camera.position + pos;
}