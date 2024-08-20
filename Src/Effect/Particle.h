/**
* @file Particle.h
*/
#ifndef PARTICLE_H_INCLUDED
#define PARTICLE_H_INCLUDED
#include "../Engine/Billboard.h"

/// <summary>
/// パーティクルの基底クラス
/// </summary>
class Particle
    : public Component
{
public:
    // コンストラクタ
    Particle(float _lifespan)
        :life_span(_lifespan)
    {}

    // デフォルトデストラクタ
    virtual ~Particle() = default;

    /// <summary>
    /// パーティクル全体を初期化する
    /// </summary>
    virtual void Awake() override
    {
        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        owner->AddComponent<Billboard>(); // ビルボード化
        owner->staticMesh = engine->GetStaticMesh("plane_xy");
        owner->materials = CloneMaterialList(owner->staticMesh);
        owner->render_queue = RENDER_QUEUE_TRANSPARENT; // 半透明キューで描画
    }

    /// <summary>
    /// 常にパーティクルの寿命を更新する
    /// </summary>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        // 一定時間経過したら
        if (life_span <= 0)
        {
            // 自身を破棄する
            owner->Destroy();
            return;
        }

        life_span -= deltaTime; // 生存期間を減らす
    }

    /// <summary>
    /// 寿命を取得する
    /// </summary>
    float GetLifespan() const { return life_span; }

private:
    float life_span = 0; // 寿命
};

#endif // PARTICLE_H_INCLUDED