/**
* @file MagicMissile
*/
#ifndef MAGICMISSILE_H_INCLUDED
#define MAGICMISSILE_H_INCLUDED
#include "../../Effect/MagicMissileParticle.h"
#include "../../Engine/SphereCollider.h"

/// <summary>
/// 魔法
/// </summary>
class MagicMissile
    : public Particle
{
public:
    // コンストラクタ
    MagicMissile()
        :Particle(LIFE_SPAN)
    {}
    //デフォルトデストラクタ
    virtual ~MagicMissile() = default;

    // 魔法を初期化する
    virtual void Awake() override;

    // 常に魔法の状態を更新する
    virtual void Update(float deltaTime) override;

    // 衝突時に衝突した相手によって処理を行う
    virtual void OnCollision(
        const ComponentPtr& self, const ComponentPtr& other) override;

    // ダメージ量を設定する
    void SetDamage(int _damage);

    vec3 velocity = vec3(0);

private:
    int damage = 0;              // 与えるダメージ量
    float time_particle = 0.05f; // パーティクル生成までの時間
    static constexpr float LIFE_SPAN = 1.0f; // 魔法の寿命(s)
};

#endif // !MAGICMISSILE_H_INCLUDED
