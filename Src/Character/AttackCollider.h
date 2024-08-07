/**
* @file AttackCollider.h
*/
#ifndef ATTACKCOLLIDER_H_INCLUDED
#define ATTACKCOLLIDER_H_INCLUDED
#include "../Engine/SphereCollider.h"

/// <summary>
/// 攻撃用のコライダー制御
/// </summary>
class AttackCollider
    : public Component
{
public:
    // 攻撃判定の状態
    enum class STATE_ATTACK_COLLIDER
    {
        SLEEP,
        COLLISION_ENABLE,
        COLLISION_DISABLE,
    };

    // コンストラクタ
    AttackCollider(STATE_ATTACK_COLLIDER _state)
        :state(_state)
    {}
    // デフォルトデストラクタ
    virtual ~AttackCollider() = default;

    // コライダーを初期化する
    virtual void Awake() override;

    // 攻撃用のコライダー状態を更新する
    virtual void Update(float deltaTime) override;

    /// <summary>
    /// 衝突時に衝突した相手によって処理を行う
    /// 
    /// 派生クラスによって処理が異なり、
    /// 必ずオーバーライドする必要があるため、
    /// 純粋仮想関数にする
    /// </summary>
    /// <param name="self">自身</param>
    /// <param name="other">衝突した相手</param>
    virtual void OnCollision(
        const ComponentPtr& self, const ComponentPtr& other) override = 0;

    // 攻撃処理を起動する
    void Activate(float collider_sphere_radius);

    /// <summary>
    /// 攻撃処理を停止する
    /// </summary>
    void Deactivate() { state = STATE_ATTACK_COLLIDER::SLEEP; }

    /// <summary>
    /// 攻撃判定の状態を取得する
    /// </summary>
    STATE_ATTACK_COLLIDER GetState() const { return state; }

private:
    STATE_ATTACK_COLLIDER state = STATE_ATTACK_COLLIDER::SLEEP;

    std::shared_ptr<SphereCollider> collider; // 攻撃判定用のコライダー
    float timer = 0.0f;                       // 時間計測用変数
};

#endif // !ATTACKCOLLIDER_H_INCLUDED
