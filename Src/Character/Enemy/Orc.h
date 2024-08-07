/**
* @file Orc.h
*/
#ifndef ORC_H_INCLUDED
#define ORC_H_INCLUDED
#include "Enemy.h"
#include "EnemyAttackCollider.h"
#include "../../Engine/CharacterMovement.h"

class Animator;

/// <summary>
/// �I�[�N
/// </summary>
class Orc
	:public Enemy
{
public:
	// �R���X�g���N�^
	Orc()
		:Enemy(HEALTH_MAX)
	{};
	// �f�t�H���g�f�X�g���N�^
	virtual ~Orc() = default;

	// �I�[�N�̃R���|�[�l���g������������
	virtual void Awake() override;

	// ��ɃI�[�N�̏�Ԃɂ���ē�����X�V����
	virtual void Update(float deltaTime) override;

	// �_���[�W���󂯂�
	void TakeDamage(int damage, GameObject* causer);

	/// <summary>
	/// �A�j���[�V������ݒ肷��
	/// </summary>
	/// <param name="a">�I�[�N�̃A�j���[�V����</param>
	void SetAnimator(const std::shared_ptr<Animator>& a) { animator = a; }

	/// <summary>
	/// �U��������ݒ肷��
	/// </summary>
	/// <param name="d">����������ɂ���ĕς���</param>
	void SetAttackDistance(float d) { DISTANCE_ATTACK = d; }

	// �I�[�N�̖���
	enum class JOB
	{
		FIGHTER,  // �t�@�C�^�[
		MAGICIAN, // �}�W�V����
	};
	/// <summary>
	/// �I�[�N�̖�����ݒ肷��
	/// </summary>
	/// <param name="j">�U�蕪�������</param>
	void SetJob(JOB j) { job = j; }

private:
	// �u�ҋ@�v���J�n����
	void StartWait();
	// �u�ǐՁv���J�n����
	void StartWalk();
	// �u�U���v���J�n����
	void StartAttack();
	// �u���@�U���v���J�n����
	void StartAttackMagicMissile();
	// �u���v���J�n����
	void StartDead();

	// �u�ҋ@�v����
	void DoWait(float deltaTime);
	// �u�ǐՁv����
	void DoWalk(float deltaTime);
	// �u�U���v����
	void DoAttack(float deltaTime);
	// �u���@�U���v����
	void DoAttackMagicMissile(float deltaTime);
	// �u���v����
	void DoDead(float deltaTime);

	static constexpr int HEALTH_MAX = 10; // �̗͍ő�l

	// �I�[�N�̏��
	enum class STATE_ORC
	{
		WAIT,				 // �ҋ@
		WALK,				 // �ǐ�
		ATTACK,				 // �U��
		ATTACK_MAGICMISSILE, // �Ή��e�U��
		DEAD,				 // ��
	};
	STATE_ORC state = STATE_ORC::WAIT;
	/// <summary>
	/// ���݂̃I�[�N�̏�Ԃ��擾����
	/// </summary>
	STATE_ORC GetState() const { return state; }

	JOB job = JOB::FIGHTER;

	CharacterMovementPtr characterMovement;
	std::shared_ptr<Animator> animator; // �|�[���A�j���[�V����
	std::shared_ptr<EnemyAttackCollider> attackCollider_right_arm; // �E��̍U������

	vec3 position_wait = vec3(0), // �ҋ@�ʒu
		destination = vec3(0);	  // �ҋ@���̈ړ���

	int index_random_walk_distance = 0;

	float time_wait = 0,		  // �ҋ@���p�����鎞��
		time_walk = 0,			  // �ړ����p�����鎞��
		time_attack = 1,		  // �U���Ԋu
		time_magic = 1,			  // ���@�U���Ԋu
		DISTANCE_ATTACK = 3;	  // �U������

	bool isCasted = false; // ���@��true = �������Afalse = �����Ă��Ȃ�
};

#endif // !ORC_H_INCLUDED