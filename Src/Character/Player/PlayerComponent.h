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
/// プレイヤー
/// </summary>
class PlayerComponent
	:public Character
{
public:
	// コンストラクタ
	PlayerComponent()
		: Character(HP_MAX)
	{}
	// デフォルトデストラクタ
	virtual ~PlayerComponent() = default;

	// プレイヤーを初期化する
	virtual void Awake() override;

	// 常にプレイヤーの状態を更新する
	virtual void Update(float deltaTime) override;

	// 衝突時に衝突した相手によって処理を行う
	virtual void OnCollision(const ComponentPtr& self, const ComponentPtr& other) override;

	// ダメージを受ける
	void TakeDamage(int damage, GameObject* causer);

	// プレイヤーの状態
	enum class STATE_PLAYER
	{
		ALIVE, // 生きている
		GOAL,  // ゴールした
		DEAD,  // 死んでいる
	};
	/// <summary>
	/// プレイヤーの状態を取得する
	/// </summary>
	STATE_PLAYER GetStatePlayer() const { return state_player; }

	/// <summary>
	/// 攻撃状況を取得する
	/// </summary>
	bool GetIsAttacking() const { return isAttacking; }
	/// <summary>
	/// 攻撃判定OFF
	/// </summary>
	void SetIsAttacking() { isAttacking = false; }

	// 剣の状態
	enum class STATE_SWORD
	{
		IDLE,		  // 何もしていない
		FIRST_SWING,  // 一撃目
		SECOND_SWING, // 二撃目
		THIRD_SWING,  // 三撃目
		GUARD,		  // ガード中
	};
	/// <summary>
	/// 剣の状態を取得する
	/// </summary>
	STATE_SWORD GetStateSword() const { return state_sword; }
	/// <summary>
	/// 剣を戻す
	/// </summary>
	void SetAfterGuard() { isGuarded = true; }

private:
	// 移動する
	void Move(GameObject& camera,float deltaTime, int PlusOrMinus,
		float x_movement_amount, float z_movement_amount);

	// ジャンプする
	void Jump(GameObject& camera, float deltaTime);

	// 剣を振る
	void SwordSwing(float deltaTime, float& rotation_hand);

	void ClickCheck();

	// 攻撃を初期化する
	void AttackInitialize(STATE_SWORD next, STATE_SWORD finish);

	// 斬撃跡を生成する
	void CreateSwordSlashEffect(GameObject& _camera, int angle);

	STATE_PLAYER state_player = STATE_PLAYER::ALIVE; // プレイヤーの状態
	STATE_SWORD state_sword = STATE_SWORD::IDLE;	 // 剣の状態

	CharacterMovementPtr characterMovement;
	std::shared_ptr<WeaponAttackCollider> attackCollider_right_arm; // 右手の攻撃判定

	float old_x = 0.0f,			// 剣を振る前のx回転
		old_y = 0.0f,			// 剣を振る前のy回転
		gravity = 0.0f,			// 重力
		anti_power = 0.0f,		// 攻撃(剣)を戻す力
		time_swing = 0.0f,		// 次の攻撃までの時間
		time_guard = 0.0f,		// 次のガードまでの時間
		time_invincible = 0.0f, // 無敵時間
		breath_scale = 0.0f;	// 呼吸運動の大きさ

	static constexpr float POWER_BASE = 4;

	bool isRunning = false,		 // プレイヤーがture = 走っている、　　　false = 走っていない
		isJumping = false,		 // プレイヤーがtrue = ジャンプしている、false = ジャンプしていない
		isAttacking = false,	 // プレイヤーがtrue = 攻撃している、　　false = 攻撃していない
		isGuarded = false,		 // プレイヤーがtrue = 防御した、　　　　false = 防御していない
		isNextSwinging = false,	 // +1撃true = 加える、false = 加えない
		isFinishedClick = false; // クリックtrue = している、false = していない

	static constexpr int HP_MAX = 10;	   // 体力最大値
	std::shared_ptr<UILayout> ui_hp_frame; // 体力ゲージのフレーム
	std::shared_ptr<UILayout> ui_hp_gauge; // 体力ゲージ
	float current_ratio = 1;			   // 体力ゲージの表示割合
	float scale = 0;					   // 体力ゲージの伸縮
	// 体力ゲージのスケーリング状態
	enum class SCALING_MODE
	{
		OPEN,
		STATIONARY,
		CLOSE,
	};
	SCALING_MODE scalingMode = SCALING_MODE::OPEN;
	// 体力ゲージの表示・非表示、調整を行う
	void HpGauge(Engine* engine, float deltaTime);
};
#endif // !PLAYERCOMPONENT_H_INCLUDED