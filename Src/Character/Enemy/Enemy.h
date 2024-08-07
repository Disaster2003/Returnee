/**
* @file Enemy.h
*/
#ifndef ENEMY_H_INCLUDED
#define ENEMY_H_INCLUDED
#include "../Character.h"

/// <summary>
/// 敵全体の継承元クラス
/// </summary>
class Enemy
    :public Character
{
public:
    // コンストラクタ
    Enemy(int _hp)
        : Character(_hp)
    {}

    // デフォルトデストラクタ
    virtual ~Enemy() = default;

    // 魔法攻撃を行う
    void AttackMagicMissile(GameObject& _owner);

    // プレイヤーとの相関関係
    struct TARGET_INFO
    {
        // プレイヤーとの位置関係
        enum class POSITON_TYPE
        {
            NO_TARGET,
            FRONT,
            LEFT,
            RIGHT,
            BACK,
        };
        POSITON_TYPE position_type = POSITON_TYPE::NO_TARGET;
        float distance = 0;
        float cosθ_by_left = 0;
        float cosθ_by_front = 0;

        // 情報が取得できていたらtrue
        constexpr explicit operator bool() const { return position_type != POSITON_TYPE::NO_TARGET; }
    };
    // 攻撃対象までの距離と向きを取得する
    TARGET_INFO GetTargetInfo() const;

    /// <summary>
    /// 標的を取得する
    /// </summary>
    /// <returns>プレイヤー</returns>
    GameObjectPtr GetTarget() const { return target; }

    /// <summary>
    /// 標的を設定する
    /// </summary>
    /// <param name="t">プレイヤー</param>
    void SetTarget(const GameObjectPtr& t) { target = t; }

protected:
    static constexpr float DISTANCE_SEARCH = 5.0f;     // 索敵距離
    // 追跡距離(この距離より遠くなったら追跡を諦める)
    static constexpr float DISTANCE_TRACK_MAX = 10.0f;
    static constexpr float DISTANCE_ATTACK = 3.0f;     // 攻撃距離

    static constexpr float SPEED_MOVE_ENEMY = 3.0f;	   // 移動速度
    static constexpr float DECELERATION = 10.0f;       // 減速量
    static constexpr float SPEED_ANIMATION = 1.0f;     // アニメーション速度
    static constexpr float SPEED_MAGICMISSILE = 18.0f; // 魔法弾の速度

private:
    GameObjectPtr target; // プレイヤー
};

#endif // !ENEMY_H_INCLUDED