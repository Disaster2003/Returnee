/**
* @file PlayerComponent.h
*/
#ifndef PLAYERCOMPONENT_H_INCLUDED
#define PLAYERCOMPONENT_H_INCLUDED
#include "../Character.h"
#include "WeaponAttackCollider.h"
#include "../../Engine/CharacterMovement.h"
#include "../../Engine/Billboard.h"
#include "../../Engine/UILayout.h"
#include "../../Engine/EasyAudio/EasyAudio.h"
#include "../../AudioSettings.h"

/// <summary>
/// �v���C���[
/// </summary>
class PlayerComponent
	:public Character
{
public:
	// �R���X�g���N�^
	PlayerComponent()
		: Character(HP_MAX)
	{}
	// �f�t�H���g�f�X�g���N�^
	virtual ~PlayerComponent() = default;

	// �v���C���[������������
	virtual void Awake() override;

	// ��Ƀv���C���[�̏�Ԃ��X�V����
	virtual void Update(float deltaTime) override;

	// �Փˎ��ɏՓ˂�������ɂ���ď������s��
	virtual void OnCollision(const ComponentPtr& self, const ComponentPtr& other) override;

	// �_���[�W���󂯂�
	void TakeDamage(int damage, GameObject* causer);

	// �v���C���[�̏��
	enum class STATE_PLAYER
	{
		ALIVE, // �����Ă���
		GOAL,  // �S�[������
		DEAD,  // ����ł���
	};
	/// <summary>
	/// �v���C���[�̏�Ԃ��擾����
	/// </summary>
	STATE_PLAYER GetStatePlayer() const { return state_player; }

	/// <summary>
	/// �U���󋵂��擾����
	/// </summary>
	bool GetIsAttacking() const { return isAttacking; }
	/// <summary>
	/// �U������OFF
	/// </summary>
	void SetIsAttacking() { isAttacking = false; }

	// ���̏��
	enum class STATE_SWORD
	{
		IDLE,		  // �������Ă��Ȃ�
		FIRST_SWING,  // �ꌂ��
		SECOND_SWING, // �񌂖�
		THIRD_SWING,  // �O����
		GUARD,		  // �K�[�h��
	};
	/// <summary>
	/// ���̏�Ԃ��擾����
	/// </summary>
	STATE_SWORD GetStateSword() const { return state_sword; }
	/// <summary>
	/// ����߂�
	/// </summary>
	void SetAfterGuard() { isGuarded = true; }

private:
	// �ړ�����
	void Move(GameObject& camera,float deltaTime, int PlusOrMinus,
		float x_movement_amount, float z_movement_amount);

	// �W�����v����
	void Jump(GameObject& camera, float deltaTime);

	// ����U��
	void SwordSwing(float deltaTime, float& rotation_hand);

	void ClickCheck();

	// �U��������������
	void AttackInitialize(STATE_SWORD next, STATE_SWORD finish);

	// �a���Ղ𐶐�����
	void CreateSwordSlashEffect(GameObject& _camera, int angle);

	STATE_PLAYER state_player = STATE_PLAYER::ALIVE; // �v���C���[�̏��
	STATE_SWORD state_sword = STATE_SWORD::IDLE;	 // ���̏��

	CharacterMovementPtr characterMovement;
	std::shared_ptr<WeaponAttackCollider> attackCollider_right_arm; // �E��̍U������

	float old_x = 0.0f,			// ����U��O��x��]
		old_y = 0.0f,			// ����U��O��y��]
		gravity = 0.0f,			// �d��
		anti_power = 0.0f,		// �U��(��)��߂���
		time_swing = 0.0f,		// ���̍U���܂ł̎���
		time_guard = 0.0f,		// ���̃K�[�h�܂ł̎���
		time_invincible = 0.0f, // ���G����
		breath_scale = 0.0f;	// �ċz�^���̑傫��

	static constexpr float POWER_BASE = 4;

	bool isRunning = false,		 // �v���C���[��ture = �����Ă���A�@�@�@false = �����Ă��Ȃ�
		isJumping = false,		 // �v���C���[��true = �W�����v���Ă���Afalse = �W�����v���Ă��Ȃ�
		isAttacking = false,	 // �v���C���[��true = �U�����Ă���A�@�@false = �U�����Ă��Ȃ�
		isGuarded = false,		 // �v���C���[��true = �h�䂵���A�@�@�@�@false = �h�䂵�Ă��Ȃ�
		isNextSwinging = false,	 // +1��true = ������Afalse = �����Ȃ�
		isFinishedClick = false; // �N���b�Ntrue = ���Ă���Afalse = ���Ă��Ȃ�

	static constexpr int HP_MAX = 10;	   // �̗͍ő�l
	std::shared_ptr<UILayout> ui_hp_frame; // �̗̓Q�[�W�̃t���[��
	std::shared_ptr<UILayout> ui_hp_gauge; // �̗̓Q�[�W
	float current_ratio = 1;			   // �̗̓Q�[�W�̕\������
	float scale = 0;					   // �̗̓Q�[�W�̐L�k
	// �̗̓Q�[�W�̃X�P�[�����O���
	enum class SCALING_MODE
	{
		OPEN,
		STATIONARY,
		CLOSE,
	};
	SCALING_MODE scalingMode = SCALING_MODE::OPEN;
	// �̗̓Q�[�W�̕\���E��\���A�������s��
	void HpGauge(Engine* engine, float deltaTime);
};
#endif // !PLAYERCOMPONENT_H_INCLUDED