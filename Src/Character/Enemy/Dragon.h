/**
* @file Dragon.h
*/
#ifndef DRAGON_H_INCLUDED
#define DRAGON_H_INCLUDED
#include "Enemy.h"
#include "EnemyAttackCollider.h"
#include "../../Engine/CharacterMovement.h"
#include "../../Effect/Mist.h"

// 先行宣言
class Animator;
class UILayout;

/// <summary>
/// ドラゴン
/// </summary>
class Dragon
	: public Enemy
{
public:
	// コンストラクタ
	Dragon()
		:Enemy(HP_MAX)
	{};
	// デフォルトデストラクタ
	virtual ~Dragon() = default;

	// ドラゴンのコンポーネントを初期化する
	virtual void Awake() override;

	// 常にドラゴンの状態によって動作を更新する
	virtual void Update(float deltaTime) override;

	// ダメージを受ける
	void TakeDamage(int damage, GameObject* causer);

	/// <summary>
	/// アニメーションを設定する
	/// </summary>
	/// <param name="a">ドラゴンのアニメーション</param>
	void SetAnimator(const std::shared_ptr<Animator>& a) { animator = a; }

	/// <summary>
	/// 索敵距離を設定する
	/// </summary>
	void SetSearchDistance(float d) { DISTANCE_SEARCH = d; }

	std::shared_ptr<MistGenerator> mistGenerator;

private:
	// 「待機」を開始する
	void StartWait();
	// 「追跡」を開始する
	void StartWalk();
	// 「攻撃」を開始する
	void StartAttack();
	// 「尻尾攻撃」を開始する
	void StartAttackTail();
	// 「火炎弾攻撃」を開始する
	void StartAttackFireBall();
	// 「空中滞在」を開始する
	void StartHovering();
	// 「空中滞在時に火炎弾攻撃」を開始する
	void StartHoveringAndFire();
	// 「死」を開始する
	void StartDead();

	// 「待機」する
	void DoWait(float deltaTime);
	// 「追跡」する
	void DoWalk(float deltaTime);
	// 「攻撃」する
	void DoAttack(float deltaTime);
	// 「尻尾攻撃」する
	void DoAttackTail(float deltaTime);
	// 「火炎弾攻撃」する
	void DoAttackFireBall(float deltaTime);
	// 「空中滞在」する
	void DoHovering(float deltaTime);
	// 「空中滞在時に火炎弾攻撃」する
	void DoHoveringAndFire(float deltaTime);
	// 「死」する
	void DoDead(float deltaTime);

	static constexpr int HP_MAX = 21;	   // 体力最大値
	std::shared_ptr<UILayout> ui_hp_frame; // 体力ゲージのフレーム
	std::shared_ptr<UILayout> ui_hp_gauge; // 体力ゲージ
	float current_ratio = 1.0f;			   // 体力ゲージの表示割合
	float scale_hp_gauge = 0.0f;		   // 体力ゲージの伸縮
	// 体力ゲージのスケーリング状態
	enum class MODE_SCALING
	{
		OPEN,
		STATIONARY,
		CLOSE,
	};
	MODE_SCALING mode_scaling = MODE_SCALING::OPEN;
	// 体力ゲージの表示・非表示、調整
	void HpGauge(Engine* engine, float deltaTime);

	// ドラゴンの状態
	enum class STATE_DRAGON
	{
		WAIT,			 // 待機
		WALK,			 // 追跡
		ATTACK,			 // 攻撃
		ATTACK_TAIL,	 // 尻尾攻撃
		ATTACK_FIREBALL, // 火炎弾攻撃
		HOVERING,		 // 空中に滞在
		HOVERING_AND_FIRE,// 空中で火炎弾攻撃
		DEAD,			 // 死
	};
	STATE_DRAGON state = STATE_DRAGON::WAIT;
	/// <summary>
	/// 現在のドラゴンの状態を取得する
	/// </summary>
	STATE_DRAGON GetState() const { return state; }

	CharacterMovementPtr characterMovement;
	std::shared_ptr<Animator> animator; // ポーンアニメーション
	std::shared_ptr<EnemyAttackCollider> attackCollider_right_arm; // 右手の攻撃判定
	std::shared_ptr<EnemyAttackCollider> attackCollider_tail;	   // 尻尾の攻撃判定

	vec3 position_wait = vec3(0),  // 待機位置
		destination = vec3(0);	   // 移動先

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
		attack_max = 2;			   // 攻撃回数の制限

	float time_wait = 0.0f,			  // 待機を継続する時間
		time_walk = 0.0f,			  // 移動を継続する時間
		time_attack = 0.0f,			  // 攻撃間隔
		time_attack_fire_ball = 2.0f, // 火炎弾攻撃間隔
		time_hovering = 8.0f,		  // 空中滞在しに行く間隔
		DISTANCE_SEARCH = 5.0f,		  // 索敵距離
		angry_effect_span = 1.0f;	  // 怒りエフェクトの発生間隔

	// 追跡距離(この距離より遠くなったら追跡を諦める)
	static constexpr float DISTANCE_TRACK_MAX = 35.0f;

	bool isArrived = false, // 空中滞在場所にtrue = 到着した、false = 到着していない
		isCasted = false,	// 火炎弾をtrue = 撃った、false = 撃っていない
		isAngry = false;	// 怒り状態にtrue = なっている、false = なっていない
};

#endif // !DRAGON_H_INCLUDED