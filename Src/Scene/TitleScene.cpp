/**
* @file TitleScene.cpp
*/
#include "TitleScene.h"
#include "ExplorationScene.h"
#include "../Engine/Light.h"
#include "../Engine/Engine.h"
#include "../Engine/EasyAudio/EasyAudio.h"
#include "../AudioSettings.h"

/// <summary>
/// �^�C�g����ʂ�����������
/// </summary>
/// <return>true = �����������Afalse = ���������s</return>
bool TitleScene::Initialize(Engine& engine)
{
	// ���C�g�̔z�u
	auto lightObject =
		engine.Create<GameObject>
		(
			"light",
			engine.GetMainCamera().position
		);
	auto light = lightObject->AddComponent<Light>();
	light->color = { 1.0f,0.9f,0.8f };
	light->intensity = 10;
	light->radius = 5;

	// �^�C�g�����S�Ɣw�i�A�v���C��ʂֈڂ��{�^���̐���
	ui_background_title = engine.CreateUIObject<UILayout>("Res/UI_background_title.dds", { 0,0 }, 1.3f).second;
	ui_background_title->GetOwner()->materials[0]->baseColor = { 0, 0, 0, 1 };
	ui_logo_title = engine.CreateUIObject<UILayout>("Res/UI_logo_title.dds", { 0,0.3f }, 0.3f).second;
	ui_logo_title->GetOwner()->materials[0]->baseColor = { 0, 0, 0, 1 };
	auto startButton = engine.CreateUIObject<UIButton>("Res/button_start_play.dds", { 0,-0.6f }, 0.1f);

	// �{�^���������ꂽ�烁�C���Q�[���V�[���ɐ؂�ւ��郉���_���̐ݒ�
	startButton.second->onClick.push_back
	(
		[this](UIButton* button)
		{
			time_fade = 1;
			button->isInteractable = false;	//�{�^���𖳌���
			EasyAudio::PlayOneShot(SE::click);
		}
	);

	// �t�F�[�h�A�E�g�pUI�I�u�W�F�N�g�̐���
	auto fade = engine.CreateUIObject<UILayout>("Res/white.tga", { 0,0 }, 1);
	obj_fade = fade.first;
	// ��ʑS�̂𕢂��T�C�Y�ɐݒ�
	const vec2 fbSize = engine.GetFramebufferSize();
	obj_fade->scale = { fbSize.x / fbSize.y,1,1 };
	// �J���[���u���A�����v�ɐݒ�
	std::fill_n(&obj_fade->color.x, 4, 0.0f);

	// �^�C�g��BGM���Đ�
	EasyAudio::Stop(AudioPlayer::bgm);
	EasyAudio::Play(AudioPlayer::bgm, BGM::title, 1, true);

	return true; // ����������
}

/// <summary>
/// ��Ƀ^�C�g����ʂ��X�V����
/// </summary>
/// <param name="engine">�Q�[���G���W��</param>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void TitleScene::Update
(
	Engine& engine,
	float deltaTime
)
{
	// �^�C�g�����S���t�F�[�h�C��
	if (!isFadedIn)
	{
		float c = ui_logo_title->GetOwner()->materials[0]->baseColor.x;
		c = std::min(c + deltaTime * 0.5f, 1.0f);
		ui_logo_title->GetOwner()->materials[0]->baseColor = { c, c, c, 1 };
		if (c >= 1)
		{
			c = 1;
			isFadedIn = true;
		}
	}
	// �w�i���t�F�[�h�C��
	else
	{
		float c = ui_background_title->GetOwner()->materials[0]->baseColor.x;
		c = std::min(c + deltaTime * 0.5f, 1.0f);
		ui_background_title->GetOwner()->materials[0]->baseColor = { c, c, c, 1 };
		if (c >= 1)
			c = 1;
	}

	// ���X�Ƀt�F�[�h�A�E�g
	if (time_fade > 0)
	{
		time_fade -= deltaTime;
		obj_fade->color[3] = 1 - time_fade;

		// �t�F�[�h�A�E�g���I�������A�v���C��ʂɐ؂�ւ���
		if (time_fade <= 0)
			engine.SetNextScene<ExplorationScene>();
	}//if fadeTimer
}

/// <summary>
/// �^�C�g����ʂ��I������
/// </summary>
void TitleScene::Finalize(Engine& engine)
{
	engine.ClearGameObjectAll();
}