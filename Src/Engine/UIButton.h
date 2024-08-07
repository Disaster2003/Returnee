/**
* @file UIButton.h
*/
#ifndef UIBUTTON_H_INCLUDED
#define UIBUTTON_H_INCLUDED
#include "UILayout.h"
#include <functional>

/// <summary>
/// UI�p�̃{�^������R���|�[�l���g
/// </summary>
class UIButton
	:public UILayout
{
public:
	// �f�t�H���g�R���X�g���N�^
	UIButton() = default;
	// �f�t�H���g�f�X�g���N�^
	virtual ~UIButton() = default;

	// ��ɃN���b�N����Ă��邩�ǂ������肷��
	virtual void Update(float deltaTime) override;

	bool isInteractable = true;					 // �}�E�X�ɔ�����true = ����Afalse = ���Ȃ�
	vec3 color_normal = { 1,1,1 };				 // �ʏ�F
	vec3 color_highlighted = { 1.3f,1.3f,1.3f }; // �I��F
	vec3 color_pressed = { 0.7f,0.7f,0.7f };	 // �����F

	// �f���Q�[�g
	using ClickEvent = std::function<void(UIButton*)>;
	std::vector<ClickEvent> onClick;

private:
	// �O��X�V���̃}�E�X���{�^����true = ������Ă���Afalse = ������Ă��Ȃ�
	bool isPrevButtonDown = false;
};

#endif // !UIBUTTON_H_INCLUDED