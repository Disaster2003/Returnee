/**
* @file PlayerComponent.cpp
*/
#include "PlayerComponent.h"
#include "../../Engine/SphereCollider.h"
#include "../../Engine/Debug.h"

/// <summary>
/// �v���C���[�̃_���[�W�\���Ɏg���Ԙg
/// </summary>
class DamageFrame
	: public Component
{
public:
	/// <summary>
	/// ��Ƀv���C���[�̃_���[�W�\���Ɏg���Ԙg�̍X�V������
	/// </summary>
	/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
	virtual void Update(float deltaTime) override
	{
		auto owner = GetOwner();
		// ��莞�Ԍo������
		if (lifespan <= 0)
		{
			// ���g��j������
			owner->Destroy();
			return;
		}
		// �����Ɠ����x�𒼌�������
		owner->color.w = lifespan;
		// ���������炷
		lifespan -= deltaTime;
	}

private:
	float lifespan = 1.0f; // �Ԙg�̎���
};

/// <summary>
/// �v���C���[������������
/// </summary>
void PlayerComponent::Awake()
{
	auto owner = GetOwner();
	auto engine = owner->GetEngine();

	// �R���C�_�[��ݒ肷��
	// ���̃R���C�_�[���㉺�ɕ��ׂĒu�����ƂŁA�c���̏Փˌ`����Č�
	for (float i = 0; i < 2; ++i)
	{
		auto collider = owner->AddComponent<SphereCollider>();
		collider->sphere.radius = 0.7f;
		collider->sphere.position.y = -0.5f * i;
	}

	characterMovement = owner->AddComponent<CharacterMovement>();

	// ���ǉ�����
	auto hand = engine->Create<GameObject>("player.hand", { 0.2f, -0.2f, -0.2f });
	hand->rotation.x = -radians(70);
	hand->rotation.y = radians(180);
	hand->staticMesh = engine->GetStaticMesh(
		"Res/MeshData/arm_and_hand/arm_and_hand_grab.obj");
	hand->SetParent(owner);
	old_x = hand->rotation.x;
	old_y = hand->rotation.y;

	// �����ǉ�����
	auto weapon = engine->Create<GameObject>("weapon", { -0.065f, -0.35f, 0.2f });
	weapon->rotation.x = radians(220);
	weapon->rotation.y = radians(180);
	weapon->staticMesh = engine->GetStaticMesh(
		"Res/MeshData/arm_and_hand/sword.obj");
	weapon->SetParent(hand);
	attackCollider_right_arm = weapon->AddComponent<WeaponAttackCollider>();

	// �v���C���[�̉e��ǉ�����
	auto headMesh = engine->Create<GameObject>("player.head.mesh", { 0, 0.4f, 0 });
	headMesh->staticMesh = engine->GetStaticMesh("crystal");
	headMesh->type_lighting = GameObject::TYPE_LIGHTING::SHADOW;
	headMesh->SetParent(owner);
	headMesh->scale = vec3(0.3f, 0.25f, 0.3f);
	auto bodyMesh = engine->Create<GameObject>("player.body.mesh", { 0, -0.2f, 0 });
	bodyMesh->staticMesh = engine->GetStaticMesh("crystal");
	bodyMesh->type_lighting = GameObject::TYPE_LIGHTING::SHADOW;
	bodyMesh->scale = vec3(0.5f, 0.4f, 0.5f);
	bodyMesh->SetParent(owner);
	auto rightLegMesh = engine->Create<GameObject>("player.right_leg.mesh", { 0.125f, -0.7f, 0 });
	rightLegMesh->staticMesh = engine->GetStaticMesh("crystal");
	rightLegMesh->type_lighting = GameObject::TYPE_LIGHTING::SHADOW;
	rightLegMesh->scale = vec3(0.2f, 0.6f, 0.2f);
	rightLegMesh->SetParent(owner);
	auto leftLegMesh = engine->Create<GameObject>("player.left_leg.mesh", { -0.125f, -0.7f, 0 });
	leftLegMesh->staticMesh = engine->GetStaticMesh("crystal");
	leftLegMesh->type_lighting = GameObject::TYPE_LIGHTING::SHADOW;
	leftLegMesh->scale = vec3(0.2f, 0.6f, 0.2f);
	leftLegMesh->SetParent(owner);
}

/// <summary>
/// ��Ƀv���C���[�̏�Ԃ��X�V����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void PlayerComponent::Update(float deltaTime)
{
	Engine* engine = GetOwner()->GetEngine();
	GameObject& camera = *GetOwner(); // engine->GetMainCamera();

	characterMovement->DecelerateXZ(10 * deltaTime);

	// alive�ȊO�̏�Ԃ͈ړ��ł��Ȃ�
	if (state_player != STATE_PLAYER::ALIVE)
	{
		// �J�[�\���̕\��
		engine->ShowMouseCursor();

		// �̗̓Q�[�W�̔j��
		ui_hp_frame->GetOwner()->Destroy();
		ui_hp_gauge->GetOwner()->Destroy();

		// �U�������j������
		attackCollider_right_arm->Deactivate();

		return;
	}

	// �̗̓Q�[�W�̕\���E��\���A����
	HpGauge(engine, deltaTime);

	// �J�[�\���̔�\��
	engine->HideMouseCursor();

	isRunning = false;
	const float cameraSpeed = 5;
	const float cameraCos = cos(camera.rotation.y);
	const float cameraSin = sin(camera.rotation.y);

	// A�L�[�������ꂽ��
	if (engine->GetKey(GLFW_KEY_A))
		// ���Ɉړ�����
		Move(camera, deltaTime, -1, cameraSpeed * cameraCos, cameraSpeed * -cameraSin);
	// D�L�[�������ꂽ��
	if (engine->GetKey(GLFW_KEY_D))
		// �E�Ɉړ�����
		Move(camera, deltaTime, 1, cameraSpeed * cameraCos, cameraSpeed * -cameraSin);
	// W�L�[�������ꂽ��
	if (engine->GetKey(GLFW_KEY_W))
		// �O�i����
		Move(camera, deltaTime, -1, cameraSpeed * cameraSin * 2, cameraSpeed * cameraCos * 2);
	// S�L�[�������ꂽ��
	if (engine->GetKey(GLFW_KEY_S))
		// ��ނ���
		Move(camera, deltaTime, 1, cameraSpeed * cameraSin * 1.5f, cameraSpeed * cameraCos * 1.5f);

	// �X�y�[�X�L�[�������ꂽ��A�W�����v����
	Jump(camera, deltaTime);

	auto hand = camera.GetChild(0);	// player.hand
	switch (state_sword)
	{
	case PlayerComponent::STATE_SWORD::IDLE:
		// �ڒn���Ă�����
		if (camera.isGrounded)
			// �ċz�^�����s��
			hand->rotation.x = sin(breath_scale) * 0.01f * ((isRunning) ? 30 : 5) + old_x;

		// �U���̑ҋ@���Ԃ��Ȃ��Ȃ�����
		if(time_swing <= 0)
			// E�L�[�������ꂽ��
			if (engine->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
			{
				// ���̉�]�p�x��߂�
				hand->rotation.x = old_x;
				hand->rotation.y = old_y;

				// 1���ڂ̏���������
				EasyAudio::PlayOneShot(SE::player_attack_first);
				isNextSwinging = true;
				AttackInitialize(STATE_SWORD::FIRST_SWING, STATE_SWORD::FIRST_SWING);
				break;
			}
		// �K�[�h�̑ҋ@���Ԃ��Ȃ��Ȃ�����
		if(time_guard <= 0)
			// Q�L�[�������ꂽ��
			if (engine->GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
			{
				// ���̉�]�p�x��߂�
				hand->rotation.x = old_x;
				hand->rotation.y = old_y;

				// �K�[�h����
				state_sword = STATE_SWORD::GUARD;
				time_guard = 3.0f;
				hand->rotation.y += radians(45);
				break;
			}
		break;
	case PlayerComponent::STATE_SWORD::FIRST_SWING:
		// 1���ڂ̏������s��
		SwordSwing(deltaTime, hand->rotation.x);

		// �N���b�N���肪�I�����Ă�����
		if (isFinishedClick)
			// ���N���b�N������
			if (engine->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
				// +1���ǉ�����
				isNextSwinging = true;

		// �N���b�N���肪�I���ɂȂ��Ă��Ȃ�������
		if (!isFinishedClick)
			// ���N���b�N���Ă��Ȃ��Ȃ�
			if (!engine->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
				// �N���b�N������I������
				isFinishedClick = true;

		// ���̈ʒu�������ʒu�ȏ�ɖ߂�����
		if (hand->rotation.x < old_x)
		{
			// ���̉�]�p�x��߂�
			hand->rotation.x = old_x;

			// 2���ڂ̏��������� / �U�����I������
			if (isNextSwinging)
				EasyAudio::PlayOneShot(SE::player_attack_second);
			AttackInitialize(STATE_SWORD::SECOND_SWING, STATE_SWORD::IDLE);
		}
		break;
	case PlayerComponent::STATE_SWORD::SECOND_SWING:
		// 2���ڂ̏������s��
		SwordSwing(deltaTime, hand->rotation.y);

		// �N���b�N���肪�I�����Ă�����
		if (isFinishedClick)
			// ���N���b�N������
			if (engine->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
				// +1���ǉ�����
				isNextSwinging = true;

		// �N���b�N���肪�I���ɂȂ��Ă��Ȃ�������
		if (!isFinishedClick)
			// ���N���b�N���Ă��Ȃ��Ȃ�
			if (!engine->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
				// �N���b�N������I������
				isFinishedClick = true;

		// ���̈ʒu�������ʒu�ȏ�ɖ߂�����
		if (hand->rotation.y < old_y)
		{
			// ���̉�]�p�x��߂�
			hand->rotation.y = old_y;

			// 3���ڂ̏��������� / �U�����I������
			if (isNextSwinging)
				EasyAudio::PlayOneShot(SE::player_attack_third);
			AttackInitialize(STATE_SWORD::THIRD_SWING, STATE_SWORD::IDLE);
		}
		break;
	case PlayerComponent::STATE_SWORD::THIRD_SWING:
		// 3���ڂ̏������s��
		SwordSwing(deltaTime, hand->rotation.x);
		anti_power -= 0.005f;
		SwordSwing(deltaTime, hand->rotation.y);

		// ���̈ʒu�������ʒu�ȏ�ɖ߂�����
		if (hand->rotation.x < old_x &&
			hand->rotation.y < old_y)
		{
			// ���̉�]�p�x��߂�
			hand->rotation.x = old_x;
			hand->rotation.y = old_y;

			// �U�����I������
			AttackInitialize(STATE_SWORD::IDLE, STATE_SWORD::IDLE);
		}
		break;
	case PlayerComponent::STATE_SWORD::GUARD:
		// �K�[�h�̑ҋ@���Ԃ��Ȃ��Ȃ�����
		if (time_guard <= 0 || isGuarded)
		{
			// �K�[�h�I��
			isGuarded = false;

			// ���̉�]�p�x��߂�
			hand->rotation.y = old_y;

			// �������Ă��Ȃ���Ԃ�
			state_sword = STATE_SWORD::IDLE;
		}
		break;
	default:
		break;
	}

	// ���G���Ԃ����炷
	if (time_invincible > 0)
		time_invincible = std::max(time_invincible - deltaTime, 0.0f);
	// �U���̑ҋ@����
	if (time_swing > 0)
		time_swing -= deltaTime;
	// �K�[�h�̑ҋ@����
	if (time_guard > 0)
		time_guard -= deltaTime;
	// �ċz�^���̑傫����������
	if (isRunning)
		breath_scale += deltaTime * 10;
	else
		breath_scale += deltaTime;

	static constexpr float SPEED_MOVE_CAMERA = 0.3f; // ���_�ړ��̑���
	static constexpr float CAMERA_ANGLE_MAX = 0.6f;	 // �㉺���_�̍ő�l

	// �}�E�X���������ɓ������āA���̓����������̐�Βl��1�ȏ�Ȃ�
	if (abs(engine->GetMouseMovement().x) > 1)
		// ���������A�J������Y����]���s��
		camera.rotation.y -= engine->GetMouseMovement().x * deltaTime * SPEED_MOVE_CAMERA;

	// �}�E�X���c�����ɓ������āA���̓����������̐�Βl��1�ȏ�Ȃ�
	if (abs(engine->GetMouseMovement().y) > 1 &&
		camera.rotation.x < CAMERA_ANGLE_MAX &&
		camera.rotation.x > -CAMERA_ANGLE_MAX)
		// ���������A�J������X����]���s��
		camera.rotation.x -= engine->GetMouseMovement().y * deltaTime * SPEED_MOVE_CAMERA;
	else if (camera.rotation.x >= CAMERA_ANGLE_MAX)
		// ������̌��x
		camera.rotation.x =  CAMERA_ANGLE_MAX - 0.01f;
	else if (camera.rotation.x <= -CAMERA_ANGLE_MAX)
		// �������̌��x
		camera.rotation.x = -CAMERA_ANGLE_MAX + 0.01f;
}

/// <summary>
/// �ړ�����
/// </summary>
/// <param name="camera">�v���C���[</param>
/// <param name="PlusOrMinus">�� or ���̕���</param>
/// <param name="x_movement_amount">x���W�𓮂�����</param>
/// <param name="z_movement_amount">z���W�𓮂�����</param>
void PlayerComponent::Move
(
	GameObject& camera,
	float deltaTime,
	int PlusOrMinus,
	float x_movement_amount,
	float z_movement_amount
)
{
	// ���s���
	isRunning = true;

	// PlusOrMinus�����Ɉړ�����
	camera.position.x += deltaTime * x_movement_amount * PlusOrMinus;
	camera.position.z += deltaTime * z_movement_amount * PlusOrMinus;
}

/// <summary>
/// �X�y�[�X�L�[�������ꂽ��A�W�����v����
/// </summary>
/// <param name="camera">�v���C���[</param>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void PlayerComponent::Jump
(
	GameObject& camera,
	float deltaTime
)
{
	// �W�����v���Ȃ�
	if (isJumping)
	{
		camera.position.y += POWER_BASE - gravity;
		gravity += deltaTime * 0.1f;
		// �n�ʂɂ�����
		if (camera.isGrounded)
		{
			// �W�����v����
			gravity = 0;
			isJumping = false;
			EasyAudio::PlayOneShot(SE::player_land);
		}
	}
	else
	{
		// space�L�[�������ꂽ��
		if (GetOwner()->GetEngine()->GetKey(GLFW_KEY_SPACE))
		{
			// �W�����v����
			isJumping = true;
			EasyAudio::PlayOneShot(SE::player_jump);
			gravity = 0;
		}
		else
			characterMovement->DecelerateXZ(10 * deltaTime);
	}
}

/// <summary>
/// ����U��
/// </summary>
/// <param name="rotation_hand">��̉�]</param>
void PlayerComponent::SwordSwing
(
	float deltaTime,
	float& rotation_hand
)
{
	rotation_hand += POWER_BASE - anti_power;
	anti_power += deltaTime * 0.3f;
}

/// <summary>
/// �U���O�ɏ��������� / �U�����I������
/// </summary>
/// <param name="isNextSwinging">���U�����邩�ǂ���</param>
/// <param name="next">�U������ꍇ�̌��̏��</param>
/// <param name="finish">�U�����Ȃ��ꍇ�̌��̏��</param>
void PlayerComponent::AttackInitialize
(
	STATE_SWORD next,
	STATE_SWORD finish
)
{
	if (isNextSwinging)
	{
		// ���̍U���̏���������
		isNextSwinging = false;
		state_sword = next;
		anti_power = 0;
		time_swing = 0.2f;
		isAttacking = true;
		// �W�����v���Ă���
		if (isJumping)
			// �_���[�W2�{
			attackCollider_right_arm->SetDamage(ATTACK_SPECIAL);
		else
			// �_���[�W���{
			attackCollider_right_arm->SetDamage(ATTAK_NORMAL);

		// �U�������ݒ肷��
		attackCollider_right_arm->Activate(1.0f);
	}
	else
	{
		// �U�����I������
		state_sword = finish;
		time_swing = 1.0f;
		isAttacking = false;
	}

	// �N���b�N�I�������߂�
	isFinishedClick = false;
}

/// <summary>
/// �Փˎ��ɏՓ˂�������ɂ���ď������s��
/// </summary>
/// <param name="self">���g</param>
/// <param name="other">�Փ˂�������</param>
void PlayerComponent::OnCollision
(
	const ComponentPtr& self,
	const ComponentPtr& other
)
{
	auto targetObject = other->GetOwner();

	// alive�ȊO�̏�Ԃ͏Փ˂ɔ������Ȃ�
	if (state_player != STATE_PLAYER::ALIVE)
		return;
	// Chest�ɐG�ꂽ��S�[������
	else if (targetObject->name == "Chest")
		state_player = STATE_PLAYER::GOAL;
}

/// <summary>
/// �_���[�W���󂯂�
/// </summary>
/// <param name="damage">�_���[�W��</param>
/// <param name="causer">���g</param>
void PlayerComponent::TakeDamage
(
	int damage,
	GameObject* causer
)
{
	// ���G���Ԓ��͍U���𖳌���
	if (time_invincible > 0)
		return;

	EasyAudio::PlayOneShot(SE::enemy_hit_attack);

	//// �_���[�W�{����K�p
	//damage = int(float(damage * GameState::Instance()->takenDamageRatio));

	// �̗͂����炷
	PlayerComponent::SetHp(damage);
	// �̗͂�0�ȉ��Ȃ�
	if (PlayerComponent::GetHp() <= 0)
	{
		// �Q�[���I�[�o�[
		PlayerComponent::SetHp(0);
		state_player = STATE_PLAYER::DEAD;
		// GameOverBGM���Đ�����
		EasyAudio::Stop(AudioPlayer::bgm);
		EasyAudio::Play(AudioPlayer::bgm, BGM::game_over, 1, true);
		EasyAudio::PlayOneShot(SE::player_dead);
	}
	else
		// ���G���Ԃ�����
		time_invincible = 0.2f;

	// �_���[�W�̔���
	const vec3 v = normalize(GetOwner()->position - causer->position);
	characterMovement->velocity.x += 10 * v.x;
	characterMovement->velocity.z += 10 * v.z;

	// �����ɂ�锽��p
	auto cm = causer->GetComponent<CharacterMovement>();
	if (cm)
	{
		cm->velocity.x -= 5 * v.x;
		cm->velocity.z -= 5 * v.z;
	}

	// �_���[�W�G�t�F�N�g��\������
	Engine* engine = GetOwner()->GetEngine();
	auto damageFrame = engine->CreateUIObject<UILayout>("Res/DamageFrame.dds", vec2(0), 1);
	damageFrame.first->AddComponent<DamageFrame>();
}

/// <summary>
/// �̗̓Q�[�W�̕\���E��\���A�������s��
/// </summary>
/// <param name="engine">�Q�[���G���W��</param>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void PlayerComponent::HpGauge(Engine* engine, float deltaTime)
{
	// �̗̓Q�[�W�̃t���[���𐶐����Ă��Ȃ�������
	if (!ui_hp_frame)
	{
		// �̗̓Q�[�W�̃t���[���𐶐�����
		ui_hp_frame = engine->CreateUIObject<UILayout>("Res/boss_hp_frame.dds", { -0.98f, 0.95f }, 0.05f).second;
		ui_hp_frame->GetOwner()->render_queue = RENDER_QUEUE_OVERLAY + 1;
	}
	// �̗̓Q�[�W�𐶐����Ă��Ȃ�������
	if (!ui_hp_gauge)
	{
		// �̗̓Q�[�W�𐶐�����
		ui_hp_gauge = engine->CreateUIObject<UILayout>("Res/boss_hp_gauge.dds", { -0.98f, 0.95f }, 0.05f).second;
		ui_hp_gauge->GetOwner()->materials[0]->baseColor = { 0.0f, 1.0f, 0.0f, 1 };
	}
	// �̗̓Q�[�W��\������
	auto gauge = ui_hp_gauge->GetOwner();
	const float aspectRatio = gauge->materials[0]->texBaseColor->GetAspectRatio();
	const float targetRatio = std::clamp(float(PlayerComponent::GetHp()) / float(HP_MAX), 0.0f, 1.0f);

	// �̗̓Q�[�W�̑������v�Z������
	if (current_ratio > targetRatio)
		current_ratio = std::max(current_ratio - deltaTime * 0.5f, targetRatio);
	else if (current_ratio < targetRatio)
		current_ratio = std::min(current_ratio + deltaTime * 0.5f, targetRatio);

	// �Q�[�W�S�̂̃X�P�[�����O
	switch (scalingMode)
	{
	case SCALING_MODE::OPEN:
		scale += deltaTime * 1.5f;
		if (scale >= 1)
		{
			scale = 1;
			scalingMode = SCALING_MODE::STATIONARY;
		}
		break;
	case SCALING_MODE::STATIONARY:
		if (current_ratio <= 0)
			scalingMode = SCALING_MODE::CLOSE;
		break;

	case SCALING_MODE::CLOSE:
		scale = std::max(scale - deltaTime * 1.5f, 0.0f);
		break;
	}

	// �v�Z���ʂ�̗̓Q�[�W�̍��W�A�傫���ɔ��f����
	ui_hp_gauge->position_base.x = -(1 - current_ratio) * aspectRatio * scale * 0.05f - 0.98f;
	gauge->scale.x = current_ratio * aspectRatio * scale * 0.05f;
	ui_hp_frame->GetOwner()->scale.x = aspectRatio * scale * 0.05f;
}