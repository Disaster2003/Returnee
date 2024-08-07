/**
* @file UILayout.h
*/
#ifndef UILAYOUT_H_INCLUDED
#define UILAYOUT_H_INCLUDED
#include "Component.h"
#include "VecMath.h"

/// <summary>
/// UI����ʋ�ԂŔz�u���邽�߂̃R���|�[�l���g
/// </summary>
class UILayout
	:public Component
{
public:
	// �f�t�H���g�R���X�g���N�^
	UILayout() = default;
	// �f�t�H���g�f�X�g���N�^
	virtual ~UILayout() = default;

	// UI���C�A�E�g�̎��s���J�n����
	void Start()override;
	// ��ɃJ�����̕��������悤��UI�𒲐�����
	void Update(float deltaTime)override;

	vec3 position_base = vec3(0); // �J�������猩�����W
};

#endif UILAYOUT_H_INCLUDED