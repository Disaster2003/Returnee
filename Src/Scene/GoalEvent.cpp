/**
* @file GoalEvent.cpp
*/
#include "GoalEvent.h"
#include "../Engine/Engine.h"
#include "../Engine/SphereCollider.h"
#include "TitleScene.h"
#include "../Effect/Stardust.h"
#include "../Engine/EasyAudio/EasyAudio.h"
#include "../AudioSettings.h"

/// <summary>
/// �R���|�[�l���g������������
/// </summary>
void GoalEvent::Awake()
{
	auto owner = GetOwner();

	// �R���|�[�l���g�̐ݒ�
	auto collider = owner->AddComponent<SphereCollider>();
	collider->sphere.radius = 1.0f;
	collider->sphere.position.y = 0.5f + 0.9f;
	collider->isStatic = true;
}

/// <summary>
/// ��ɃS�[����ʂ��X�V����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void GoalEvent::Update(float deltaTime)
{
	auto owner = GetOwner();
	auto engine = owner->GetEngine();

	// �S�[���ɐG�ꂽ��A�L���L�������p�[�e�B�N���𐶐���������
	if (isTriggered)
	{
		time_particle += deltaTime;
		if (time_particle >= 0.1f)
		{
			time_particle -= 0.1f;
			auto light = engine->Create<GameObject>("light", owner->position);
			light->AddComponent<Stardust>();
		}
	}

	// ���X�Ƀt�F�[�h�A�E�g
	if (time_fade > 0)
	{
		time_fade -= deltaTime;
		obj_fade->color[3] = 1 - time_fade;
		// �t�F�[�h�A�E�g���I�������A�^�C�g����ʂɐ؂�ւ���
		if (time_fade <= 0)
			engine->SetNextScene<TitleScene>();
	} // if fadeTimer
}

/// <summary>
/// �S�[�����o�ɂ�����Փ˃C�x���g
/// </summary>
/// <param name="self">�Փ˂����R���C�_�[�R���|�[�l���g(����)</param>
/// <param name="other">�Փ˂����R���C�_�[�R���|�[�l���g(����)</param>
void GoalEvent::OnCollision
(
	const ComponentPtr& self,
	const ComponentPtr& other
)
{
	// ��x�����v���C���[�ƏՓ˂ł���
	if (isTriggered || other->GetOwner()->name != "player")
		return;

	isTriggered = true; // ��Ԃ��u�Փˍς݁v�ɂ���

	// ���b�Z�[�W�I�u�W�F�N�g�̐���
	Engine* engine = GetOwner()->GetEngine();
	engine->CreateUIObject<UILayout>("Res/UI_logo_game_clear.dds", { 0,0 }, 0.1f);

	// �߂�{�^���̐���
	auto button =
		engine->CreateUIObject<UIButton>
		(
			"Res/button_return_title.dds",
			{ 0,-0.5f },
			0.1f
		);
	button.second->onClick.push_back
	(
		[this](UIButton* button)
		{
			time_fade = 1;
			button->isInteractable = false;
			EasyAudio::PlayOneShot(SE::click);
		}
	);

	// �t�F�[�h�A�E�g�p�摜�̐���
	auto fade =
		engine->CreateUIObject<UILayout>
		(
			"Res/white.tga",
			{ 0,0 },
			1
		);
	obj_fade = fade.first;
	const vec2 fbSize = engine->GetFramebufferSize();
	obj_fade->scale = { fbSize.x / fbSize.y,1,1 };
	std::fill_n(&obj_fade->color.x, 4, 0.0f);

	// �Q�[���N���ABGM/SE���Đ�
	EasyAudio::Stop(AudioPlayer::bgm);
	EasyAudio::Stop(AudioPlayer::run);
	EasyAudio::Play(AudioPlayer::bgm, BGM::game_clear, 1, true);
	EasyAudio::PlayOneShot(SE::goal);
}