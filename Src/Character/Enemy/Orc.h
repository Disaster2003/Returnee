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
/// オーク
/// </summary>
class Orc
	:public Enemy
{
public:
	// コンストラクタ
	Orc()
		:Enemy(HEALTH_MAX)
	{};
	// デフォルトデストラクタ
	virtual ~Orc() = default;

	// オークのコンポーネントを初期化する
	virtual void Awake() override;

	// 常にオークの状態によって動作を更新する
	virtual void Update(float deltaTime) override;

	// ダメージを受ける
	void TakeDamage(int damage, GameObject* causer);

	/// <summary>
	/// アニメーションを設定する
	/// </summary>
	/// <param name="a">オークのアニメーション</param>
	void SetAnimator(const std::shared_ptr<Animator>& a) { animator = a; }

	/// <summary>
	/// 攻撃距離を設定する
	/// </summary>
	/// <param name="d">距離を役割によって変える</param>
	void SetAttackDistance(float d) { DISTANCE_ATTACK = d; }

	// オークの役割
	enum class JOB
	{
		FIGHTER,  // ファイター
		MAGICIAN, // マジシャン
	};
	/// <summary>
	/// オークの役割を設定する
	/// </summary>
	/// <param name="j">振り分ける役割</param>
	void SetJob(JOB j) { job = j; }

private:
	// 「待機」を開始する
	void StartWait();
	// 「追跡」を開始する
	void StartWalk();
	// 「攻撃」を開始する
	void StartAttack();
	// 「魔法攻撃」を開始する
	void StartAttackMagicMissile();
	// 「死」を開始する
	void StartDead();

	// 「待機」する
	void DoWait(float deltaTime);
	// 「追跡」する
	void DoWalk(float deltaTime);
	// 「攻撃」する
	void DoAttack(float deltaTime);
	// 「魔法攻撃」する
	void DoAttackMagicMissile(float deltaTime);
	// 「死」する
	void DoDead(float deltaTime);

	static constexpr int HEALTH_MAX = 10; // 体力最大値

	// オークの状態
	enum class STATE_ORC
	{
		WAIT,				 // 待機
		WALK,				 // 追跡
		ATTACK,				 // 攻撃
		ATTACK_MAGICMISSILE, // 火炎弾攻撃
		DEAD,				 // 死
	};
	STATE_ORC state = STATE_ORC::WAIT;
	/// <summary>
	/// 現在のオークの状態を取得する
	/// </summary>
	STATE_ORC GetState() const { return state; }

	JOB job = JOB::FIGHTER;

	CharacterMovementPtr characterMovement;
	std::shared_ptr<Animator> animator; // ポーンアニメーション
	std::shared_ptr<EnemyAttackCollider> attackCollider_right_arm; // 右手の攻撃判定

	vec3 position_wait = vec3(0), // 待機位置
		destination = vec3(0);	  // 待機中の移動先

	int index_random_walk_distance = 0;

	float time_wait = 0,		  // 待機を継続する時間
		time_walk = 0,			  // 移動を継続する時間
		time_attack = 1,		  // 攻撃間隔
		time_magic = 1,			  // 魔法攻撃間隔
		DISTANCE_ATTACK = 3;	  // 攻撃距離

	bool isCasted = false; // 魔法をtrue = 撃った、false = 撃っていない
};

#endif // !ORC_H_INCLUDED