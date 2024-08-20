/**
* @file TitleScene.h
*/
#ifndef TITLESCENE_H_INCLUDED
#define TITLESCENE_H_INCLUDED
#include "../Engine/Scene.h"
#include "../Engine/UIButton.h"

/// <summary>
/// �^�C�g�����
/// </summary>
class TitleScene
	:public Scene
{
public:
	// �f�t�H���g�R���X�g���N�^
	TitleScene() = default;
	// �f�t�H���g�f�X�g���N�^
	virtual ~TitleScene() = default;

	// �^�C�g����ʂ�����������
	virtual bool Initialize(Engine& engine) override;
	// ��Ƀ^�C�g����ʂ��X�V����
	virtual void Update(Engine& engine, float deltaTime) override;
	// �^�C�g����ʂ��I������
	virtual void Finalize(Engine& engine) override;

private:
	GameObjectPtr obj_fade;	// �t�F�[�h�A�E�g�I�u�W�F�N�g
	float time_fade = 0;	// �t�F�[�h�A�E�g�܂ł̎���

	std::shared_ptr<UILayout> ui_logo_title;	   // �^�C�g�����S
	std::shared_ptr<UILayout> ui_background_title; // �^�C�g���w�i

	// �t�F�[�h�C��true = �����Afalse = ���Ă��Ȃ�
	bool isFadedIn = false;
};

#endif // ! TITLESCENE_H_INCLUDED