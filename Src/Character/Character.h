/**
* @file Character.h
*/
#ifndef CHARACTER_H_INCLUDED
#define CHARACTER_H_INCLUDED
#include "../Engine/Component.h"

/// <summary>
/// キャラクター全体の基底クラス
/// </summary>
class Character
	: public Component
{
public:
	// デフォルトコンストラクタ
	virtual ~Character() = default;

protected:
	// コンストラクタ
	Character(int _hp)
		:hp(_hp)
	{}

	// 体力を取得する
	int GetHp() const { return hp; }
	// 体力を設定する
	void SetHp(int damage) { hp -= damage; }

	// ダメージ量
	enum TYPE_DAMAGE
	{
		ATTAK_NORMAL = 1,
		ATTACK_SPECIAL = 2,
	};

private:
	int hp = 0;
};

#endif // !CHARACTER_H_INCLUDED
