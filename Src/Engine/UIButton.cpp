/**
* @file UIButton.cpp
*/
#include "UIButton.h"
#include "Engine.h"

/// <summary>
/// ��ɃN���b�N����Ă��邩�ǂ������肷��
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void UIButton::Update(float deltaTime)
{
	// ���N���X�̃����o�֐������s
	UILayout::Update(deltaTime);

	auto owner = GetOwner();
	auto engine = owner->GetEngine();

	// �{�^���̗L���E��������
	if (!isInteractable)
	{
		std::copy_n(&color_normal.x, 3, &owner->color.x);
		owner->color[3] = 0.75f; // �����ۂ����o�����ߏ��������ɂ���
		return;
	}

	const vec2 mouse = engine->GetMousePosition();
	if (abs(mouse.x - position_base.x) < owner->scale.x &&
		abs(mouse.y - position_base.y) < owner->scale.y)
	{
		// �J�[�\�����{�^���̈���ɂ���
		const bool buttonDown = engine->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT);
		if (buttonDown)
			// ���{�^����������Ă���
			std::copy_n(&color_pressed.x, 3, &owner->color.x);
		else
		{
			// ���{�^����������Ă���
			std::copy_n(&color_highlighted.x, 3, &owner->color.x);
			// �{�^���N���b�N����
			if (isPrevButtonDown)
				// �f���Q�[�g�����s
				for (auto& e : onClick)
					e(this);
		}	//if buttonDown

		// �}�E�X�{�^���̏�Ԃ��X�V
		isPrevButtonDown = buttonDown;
	}
	else
	{
		// �J�[�\�����{�^���̈�O�ɂ���
		std::copy_n(&color_normal.x, 3, &owner->color.x);
		isPrevButtonDown = false;
	}
}