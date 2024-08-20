/**
* @file Orc.cpp
*/
#include "Orc.h"
#include "../../Engine/Animation.h"
#include "../../Engine/GameObject.h"
#include "../Player/PlayerComponent.h"
#include "../../Effect/Smoke.h"
#include "../../Engine/Random.h"

/// <summary>
/// オークのコンポーネントを初期化する
/// </summary>
void Orc::Awake()
{
	auto owner = GetOwner();
	auto engine = owner->GetEngine();

	// コンポーネントを設定する
	for (float i = 0; i < 2; ++i)
	{
		auto collider = owner->AddComponent<SphereCollider>();
		collider->sphere.radius = 0.5f + i * 0.2f;
		collider->sphere.position.y = 1.0f + 0.9f * i;
	}

	// 攻撃判定を設定する
	characterMovement = owner->AddComponent<CharacterMovement>();
    if (job == JOB::FIGHTER)
    {
        auto go = owner->GetEngine()->Create<GameObject>("OrcShaman Right Arm Attack Collision ", { -1.7f, 1.65f, 0.6f });
        go->SetParentAndJoint(owner, 2); // 右腕に接続
        attackCollider_right_arm = go->AddComponent<EnemyAttackCollider>();
    }

	// 正面の初期化を行う
	const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };

	// 待機位置の初期化を行う
	position_wait = owner->position + dirFront * 2;
}

/// <summary>
/// 常にオークの状態によって動作を更新する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Orc::Update(float deltaTime)
{
    // プレイヤーが存在したら
    if (GetTarget())
    {
        // プレイヤーが死んでいたら何もしない
        const auto player = GetTarget()->GetComponent<PlayerComponent>();
        if (player->GetStatePlayer() == PlayerComponent::STATE_PLAYER::DEAD)
        {
            characterMovement->velocity = vec3(0);
            animator->SetSpeed(0);
            return;
        }
    }

    /*
      オークの行動について:

      体力が0以下になったら「死亡」に遷移.

      待機: 出現地点の周囲5mにランダムな目的地を設定して移動.
           目的地に到着したら1～5秒静止.
           10m以内にプレイヤーが侵入したらランダム時間でレイキャスト. レイが到達したら「追跡」に遷移.
           上記を繰り返す.

      追跡: プレイヤーを追いかける方向に移動.
           ランダム時間で「魔法」に遷移.
           プレイヤーまでの距離が3または5m以内ならば「攻撃」に遷移.
           プレイヤーまでの距離が10m以上ならばランダム時間でレイキャスト. レイが到達しなかったら「待機」に遷移.

      魔法:
      攻撃: アニメーションが終了したら「追跡」に遷移.
    */
    switch (state)
    {
    case STATE_ORC::WAIT: DoWait(deltaTime); break;
    case STATE_ORC::WALK: DoWalk(deltaTime); break;
    case STATE_ORC::ATTACK: DoAttack(deltaTime); break;
    case STATE_ORC::ATTACK_MAGICMISSILE: DoAttackMagicMissile(deltaTime); break;
    case STATE_ORC::DEAD: DoDead(deltaTime); break;
    }
}

/// <summary>
/// ダメージを受ける
/// </summary>
/// <param name="damage">ダメージ量</param>
/// <param name="causer">自身</param>
void Orc::TakeDamage
(
    int damage,
    GameObject* causer
)
{
    // 状態が「死亡」なら何も起こらない
    if (state == STATE_ORC::DEAD)
        return;

    // 斬られた音を鳴らす
    EasyAudio::PlayOneShot(SE::sword_hit);

    // ノックバック
    const vec3 v = normalize(GetOwner()->position - causer->position);
    characterMovement->velocity.x += 4 * v.x;
    characterMovement->velocity.z += 4 * v.z;

    // ダメージを受ける
    Orc::SetHp(damage);

    // 体力が0以下なら
    if (Orc::GetHp() <= 0)
    {
        // 死亡
        StartDead();

        // 死亡SEを鳴らす
        EasyAudio::PlayOneShot(SE::orc_dead);
    }
}

/// <summary>
/// 「待機」を開始する
/// </summary>
void Orc::StartWait()
{
    // 状態が「待機」なら何もしない
    if (state == STATE_ORC::WAIT)
        return;

    if (animator)
    {
        // 「待機」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("wait");
    }

    // 状態を「待機」にする
    state = STATE_ORC::WAIT;
}

/// <summary>
/// 「追跡」を開始する
/// </summary>
void Orc::StartWalk()
{
    // 状態が「追跡」なら何もしない
    if (state == STATE_ORC::WALK)
        return;

    if (animator)
    {
        // 「追跡」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION * 2);
        animator->Play("walk");
    }

    // 状態を「追跡」にする
    state = STATE_ORC::WALK;
}

/// <summary>
/// 「攻撃」を開始する
/// </summary>
void Orc::StartAttack()
{
    // 状態が「攻撃」なら何もしない
    if (state == STATE_ORC::ATTACK)
        return;

    if (animator)
    {
        // 「攻撃」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION);
        // 「攻撃」、「横攻撃」をランダムで選んで行う
        if (Random::Range(0, 1) <= 0.5f)
            animator->Play("attack");
        else
            animator->Play("attack.right");
    }

    // ダメージ量を設定する
    attackCollider_right_arm->SetDamage(ATTAK_NORMAL);

    // 攻撃判定を設定する
    attackCollider_right_arm->Activate(1.0f);

    // 次に攻撃するまでの時間(0.5～1.5秒)
    time_attack = float(std::rand() % 11) * 0.1f + 0.5f;

    // 状態を「攻撃」にする
    state = STATE_ORC::ATTACK;
}

/// <summary>
/// 「魔法攻撃」を開始する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Orc::StartAttackMagicMissile()
{
    // 状態が「魔法攻撃」なら何もしない
    if (state == STATE_ORC::ATTACK_MAGICMISSILE)
        return;

    auto owner = GetOwner();
    auto engine = owner->GetEngine();

    // 正面を決定する
    const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };

    // 発動を表す煙を表示する
    constexpr vec3 spawnOffset = { 0, 3.0f, 0 };
    const vec3 spawnPosition = owner->position + dirFront + spawnOffset;
    auto smoke = engine->Create<GameObject>("smoke", spawnPosition);
    smoke->AddComponent<Smoke>();
    smoke->materials[0]->texBaseColor = engine->GetTexture("Res/effect_magic_circle.tga");
    smoke->materials[0]->texEmission = engine->GetTexture("Res/effect_magic_circle.tga");
    smoke->materials[0]->baseColor = { 2,2,0.2f,1 };
    smoke->materials[0]->emission = { 2,2,0.2f };

    if (animator)
    {
        // 「魔法攻撃」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("magic");
    }
    smoke->materials[0]->emission = { 0.2f, 0.1f, 0.1f };

    // 次に魔法を詠唱するまでの時間(0.5～2.0秒)
    time_magic = float(std::rand() % 16) * 0.1f + 0.5f;

    isCasted = false;

    // 状態を「魔法攻撃」にする
    state = STATE_ORC::ATTACK_MAGICMISSILE;
}

/// <summary>
/// 「死」を開始する
/// </summary>
void Orc::StartDead()
{
    // 状態が「死亡」なら何もしない
    if (state == STATE_ORC::DEAD)
        return;

    if (animator)
    {
        // 「死」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION);
        if (Random::Range(0, 1) <= 0.5f)
            animator->Play("down.0");
        else
            animator->Play("down.1");
    }

    // 状態を「死亡」にする
    state = STATE_ORC::DEAD;

    // 攻撃判定を破棄する
    attackCollider_right_arm->Deactivate();
}

/// <summary>
/// 「待機」する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Orc::DoWait(float deltaTime)
{
    auto owner = GetOwner();
    auto engine = owner->GetEngine();
    if (time_walk > 0)
    {
        time_walk -= deltaTime;

        const vec3 v = destination - owner->position;
        const float d2 = dot(v, v);
        if (d2 < 0.5f * 0.5f)
        {
            // 目的地に到着
            time_walk = 0;
            time_wait = float(rand() % 5) + 1;
            animator->Play("wait");
            animator->SetSpeed(SPEED_ANIMATION);
            characterMovement->DecelerateXZ(DECELERATION * deltaTime);
        }
        else
        {
            // 目的地に向かって移動中
            const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };
            const vec3 nv = v * (1 / sqrt(d2));
            const float sinTheta = cross(dirFront, nv).y;
            owner->rotation.y += 5 * sinTheta * deltaTime;
            characterMovement->AccelerateXZ(5 * nv * deltaTime, 1);
        }
    }
    else if (time_wait > 0)
    {
        // 目的地にて待機中
        time_wait -= deltaTime;
        characterMovement->DecelerateXZ(DECELERATION * deltaTime);
    }
    else
    {
        // 次の目的地を選択する
        float r = radians(float(rand() % 18) * 20);
        constexpr float distanceList[] = { 5, 1, 3, 4, 2, 5, 4, 5, 3, 4 };
        float s = sin(r);
        float c = cos(r);
        destination = position_wait + vec3(c, 0, -s) * distanceList[index_random_walk_distance];
        destination.y = owner->position.y;
        time_walk = 5;
        index_random_walk_distance = (index_random_walk_distance + 1) % std::size(distanceList);
        if (animator->GetCurrentClipName() != "walk")
        {
            animator->Play("walk");
            animator->SetSpeed(SPEED_ANIMATION * 2);
        }
        // 停止する
        characterMovement->DecelerateXZ(DECELERATION * deltaTime);
    }

    // 索敵する
    const auto target_info = GetTargetInfo();
    if (target_info)
    {
        float s = DISTANCE_SEARCH;
        if (target_info.distance <= s)
        {
            // 敵を発見したので追跡開始
            time_wait = 0;
            time_walk = 0;
            StartWalk();
        }
    }
}

/// <summary>
/// 「追跡」する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Orc::DoWalk(float deltaTime)
{
    auto owner = GetOwner();
    auto engine = owner->GetEngine();

    // 視認距離にプレイヤーがいなかったら
    const auto targetInfo = GetTargetInfo();
    if (!targetInfo || targetInfo.distance > DISTANCE_TRACK_MAX)
    {
        // 「待機」を開始する
        StartWait();
        return;
    }

    // 攻撃開始タイマーを更新する
    if (job == JOB::FIGHTER)
        time_attack = std::max(time_attack - deltaTime, 0.0f);
    else if (job == JOB::MAGICIAN)
        time_magic = std::max(time_magic - deltaTime, 0.0f);

    const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };
    const vec3 dirLeft = { cos(owner->rotation.y), 0, -sin(owner->rotation.y) };

    // 攻撃対象が自分の前にいたら攻撃のチェック、そうでなければ旋回する
    if (targetInfo.position_type == TARGET_INFO::POSITON_TYPE::FRONT)
    {
        // プレイヤーが攻撃判定圏内であれば
        if (targetInfo.distance <= DISTANCE_ATTACK)
        {
            if (time_attack <= 0)
            {
                // 「攻撃」を開始する
                StartAttack();
                return;
            }
            if (time_magic <= 0)
            {
                // 「火炎弾攻撃」を開始する
                StartAttackMagicMissile();
                return;
            }
        }
    }
    else
    {
        float theta = targetInfo.cosθ_by_left;
        owner->rotation.y += 30 * theta * deltaTime;
    }

    // 敵までの距離が攻撃範囲の境界に来るように加速方向を切り替える
    constexpr float accel = 4; // 基本加速度
    if (targetInfo.distance > DISTANCE_ATTACK)
        characterMovement->AccelerateXZ(dirFront * SPEED_MOVE_ENEMY * accel * deltaTime, SPEED_MOVE_ENEMY);
    else
        // 逆方向に加速(減速)する
        characterMovement->AccelerateXZ(-dirFront * SPEED_MOVE_ENEMY * accel * deltaTime, SPEED_MOVE_ENEMY);
}

/// <summary>
/// 「攻撃」する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Orc::DoAttack(float deltaTime)
{
    // アニメーションが終了したら
    if (!animator->IsPlaying())
    {
        // 「追跡」を開始する
        StartWalk();
        return;
    }

    // 停止する
    characterMovement->DecelerateXZ(DECELERATION * deltaTime);
}

/// <summary>
/// 「魔法」する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Orc::DoAttackMagicMissile(float deltaTime)
{
    auto owner = GetOwner();

    // アニメーションが終了したら
    if (!animator->IsPlaying())
    {
        // 「追跡」を開始する
        StartWalk();
        return;
    }

    // 停止する
    characterMovement->DecelerateXZ(DECELERATION * deltaTime);

    // 詠唱開始から0.8秒後に魔法を発動する
    const float t = animator->GetTimer();
    if (isCasted || t < 0.8f)
        return;

    // 発射
    isCasted = true;
    AttackMagicMissile(*owner);
}

/// <summary>
/// 「死」する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Orc::DoDead(float deltaTime)
{
    auto owner = GetOwner();
    auto engine = owner->GetEngine();

    // 停止する
    characterMovement->DecelerateXZ(DECELERATION * deltaTime);

    if (!animator->IsPlaying())
    {
        // 突然消えるのをごまかすため、煙エフェクトを表示する
        for (int i = 0; i < 5; ++i)
        {
            float x = float(rand() % 5) * 0.25f - 0.5f; // -0.5～+0.5m
            float y = float(rand() % 6) * 0.05f + 0.25f;// 0.25～0.5m
            float z = float(rand() % 5) * 0.25f - 0.5f; // -0.5～+0.5m
            auto smoke = engine->Create<GameObject>("smoke", owner->position + vec3(x, y, z));
            smoke->AddComponent<Smoke>();
            float b = float(rand() % 5) * 0.25f + 0.25f;
            smoke->materials[0]->emission = vec3{ 0.125f, 0.15f, 0.1f } *b;
        }

        // 自身を破棄する
        owner->Destroy();
    }
}
