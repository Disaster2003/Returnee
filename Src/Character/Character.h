/**
* @file Character.h
*/
#ifndef CHARACTER_H_INCLUDED
#define CHARACTER_H_INCLUDED
#include "../Engine/Component.h"

/// <summary>
/// �L�����N�^�[�S�̂̊��N���X
/// </summary>
class Character
	: public Component
{
public:
	// �f�t�H���g�R���X�g���N�^
	virtual ~Character() = default;

protected:
	// �R���X�g���N�^
	Character(int _hp)
		:hp(_hp)
	{}

	/// <summary>
	/// �̗͂��擾����
	/// </summary>
	int GetHp() const { return hp; }
	/// <summary>
	/// �̗͂�ݒ肷��
	/// </summary>
	/// <param name="damage">�_���[�W��</param>
	void SetHp(int damage) { hp -= damage; }

	// �_���[�W��
	enum TYPE_DAMAGE
	{
		ATTAK_NORMAL = 1,
		ATTACK_SPECIAL = 2,
	};

private:
	int hp = 0; // �̗�
};

#endif // !CHARACTER_H_INCLUDED
