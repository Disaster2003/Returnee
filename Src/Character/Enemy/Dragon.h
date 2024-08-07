/**
* @file Dragon.h
*/
#ifndef DRAGON_H_INCLUDED
#define DRAGON_H_INCLUDED
#include "Enemy.h"
#include "EnemyAttackCollider.h"
#include "../../Engine/CharacterMovement.h"
#include "../../Effect/Mist.h"

// ��s�錾
class Animator;
class UILayout;

/// <summary>
/// �h���S��
/// </summary>
class Dragon
	: public Enemy
{
public:
	// �R���X�g���N�^
	Dragon()
		:Enemy(HP_MAX)
	{};
	// �f�t�H���g�f�X�g���N�^
	virtual ~Dragon() = default;

	// �h���S���̃R���|�[�l���g������������
	virtual void Awake() override;

	// ��Ƀh���S���̏�Ԃɂ���ē�����X�V����
	virtual void Update(float deltaTime) override;

	// �_���[�W���󂯂�
	void TakeDamage(int damage, GameObject* causer);

	/// <summary>
	/// �A�j���[�V������ݒ肷��
	/// </summary>
	/// <param name="a">�h���S���̃A�j���[�V����</param>
	void SetAnimator(const std::shared_ptr<Animator>& a) { animator = a; }

	/// <summary>
	/// ���G������ݒ肷��
	/// </summary>
	void SetSearchDistance(float d) { DISTANCE_SEARCH = d; }

	std::shared_ptr<MistGenerator> mistGenerator;

private:
	// �u�ҋ@�v���J�n����
	void StartWait();
	// �u�ǐՁv���J�n����
	void StartWalk();
	// �u�U���v���J�n����
	void StartAttack();
	// �u�K���U���v���J�n����
	void StartAttackTail();
	// �u�Ή��e�U���v���J�n����
	void StartAttackFireBall();
	// �u�󒆑؍݁v���J�n����
	void StartHovering();
	// �u�󒆑؍ݎ��ɉΉ��e�U���v���J�n����
	void StartHoveringAndFire();
	// �u���v���J�n����
	void StartDead();

	// �u�ҋ@�v����
	void DoWait(float deltaTime);
	// �u�ǐՁv����
	void DoWalk(float deltaTime);
	// �u�U���v����
	void DoAttack(float deltaTime);
	// �u�K���U���v����
	void DoAttackTail(float deltaTime);
	// �u�Ή��e�U���v����
	void DoAttackFireBall(float deltaTime);
	// �u�󒆑؍݁v����
	void DoHovering(float deltaTime);
	// �u�󒆑؍ݎ��ɉΉ��e�U���v����
	void DoHoveringAndFire(float deltaTime);
	// �u���v����
	void DoDead(float deltaTime);

	static constexpr int HP_MAX = 21;	   // �̗͍ő�l
	std::shared_ptr<UILayout> ui_hp_frame; // �̗̓Q�[�W�̃t���[��
	std::shared_ptr<UILayout> ui_hp_gauge; // �̗̓Q�[�W
	float current_ratio = 1.0f;			   // �̗̓Q�[�W�̕\������
	float scale_hp_gauge = 0.0f;		   // �̗̓Q�[�W�̐L�k
	// �̗̓Q�[�W�̃X�P�[�����O���
	enum class MODE_SCALING
	{
		OPEN,
		STATIONARY,
		CLOSE,
	};
	MODE_SCALING mode_scaling = MODE_SCALING::OPEN;
	// �̗̓Q�[�W�̕\���E��\���A����
	void HpGauge(Engine* engine, float deltaTime);

	// �h���S���̏��
	enum class STATE_DRAGON
	{
		WAIT,			 // �ҋ@
		WALK,			 // �ǐ�
		ATTACK,			 // �U��
		ATTACK_TAIL,	 // �K���U��
		ATTACK_FIREBALL, // �Ή��e�U��
		HOVERING,		 // �󒆂ɑ؍�
		HOVERING_AND_FIRE,// �󒆂ŉΉ��e�U��
		DEAD,			 // ��
	};
	STATE_DRAGON state = STATE_DRAGON::WAIT;
	/// <summary>
	/// ���݂̃h���S���̏�Ԃ��擾����
	/// </summary>
	STATE_DRAGON GetState() const { return state; }

	CharacterMovementPtr characterMovement;
	std::shared_ptr<Animator> animator; // �|�[���A�j���[�V����
	std::shared_ptr<EnemyAttackCollider> attackCollider_right_arm; // �E��̍U������
	std::shared_ptr<EnemyAttackCollider> attackCollider_tail;	   // �K���̍U������

	vec3 position_wait = vec3(0),  // �ҋ@�ʒu
		destination = vec3(0);	   // �ړ���

	static constexpr vec3 position_hovering[12] =
	{
		{	   -7, 8, 88.66f },
		{ -10.66f, 8, 85 },
		{	  -12, 8, 80 },
		{ -10.66f, 8, 75 },
		{	   -7, 8, 71.44f },
		{		0, 8, 70 },
		{		7, 8, 71.44f },
		{  10.66f, 8, 75 },
		{	   12, 8, 80 },
		{  10.66f, 8, 85 },
		{		7, 8, 88.66f },
		{		0, 8, 90 },
	};

	int index_random_walk_distance = 0,
		attack_max = 2;			   // �U���񐔂̐���

	float time_wait = 0.0f,			  // �ҋ@���p�����鎞��
		time_walk = 0.0f,			  // �ړ����p�����鎞��
		time_attack = 0.0f,			  // �U���Ԋu
		time_attack_fire_ball = 2.0f, // �Ή��e�U���Ԋu
		time_hovering = 8.0f,		  // �󒆑؍݂��ɍs���Ԋu
		DISTANCE_SEARCH = 5.0f,		  // ���G����
		angry_effect_span = 1.0f;	  // �{��G�t�F�N�g�̔����Ԋu

	// �ǐՋ���(���̋�����艓���Ȃ�����ǐՂ���߂�)
	static constexpr float DISTANCE_TRACK_MAX = 35.0f;

	bool isArrived = false, // �󒆑؍ݏꏊ��true = ���������Afalse = �������Ă��Ȃ�
		isCasted = false,	// �Ή��e��true = �������Afalse = �����Ă��Ȃ�
		isAngry = false;	// �{���Ԃ�true = �Ȃ��Ă���Afalse = �Ȃ��Ă��Ȃ�
};

#endif // !DRAGON_H_INCLUDED