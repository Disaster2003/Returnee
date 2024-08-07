/**
* @file Dragon.cpp
*/
#include "Dragon.h"
#include "../../Engine/Animation.h"
#include "../../Engine/GameObject.h"
#include "../Player/PlayerComponent.h"
#include "../../Effect/Smoke.h"
#include "../../Engine/Random.h"

/// <summary>
/// ドラゴンのコンポーネントを初期化する
/// </summary>
void Dragon::Awake()
{
	auto owner = GetOwner();
	auto engine = owner->GetEngine();

    // コンポーネントを設定する
	for (float i = 0; i < 2; ++i)
	{
		auto collider = owner->AddComponent<SphereCollider>();
		collider->sphere.radius = 0.5f + i * 0.2f;
		collider->sphere.position.y = 0.5f + 0.9f * i;
	}

    // 攻撃判定を設定する
	characterMovement = owner->AddComponent<CharacterMovement>();
	auto go = owner->GetEngine()->Create<GameObject>("Dragon Right Arm Attack Collision ", { -1.7f, 1.65f, 0.6f });
	go->SetParentAndJoint(owner, 2); // 右腕に接続
	attackCollider_right_arm = go->AddComponent<EnemyAttackCollider>();

	auto go2 = owner->GetEngine()->Create<GameObject>("Dragon Tail Attack Collision ", { 0.00f, 1.00f, -0.6f });
	go2->SetParentAndJoint(owner, 8); // 尻尾に接続
	attackCollider_tail = go->AddComponent<EnemyAttackCollider>();

    // 正面の初期化を行う
    const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };

    // 待機位置の初期化を行う
    position_wait = owner->position + dirFront * 2;
}

/// <summary>
/// 常にドラゴンの状態によって動作を更新する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Dragon::Update(float deltaTime)
{
    auto owner = GetOwner();
    auto engine = owner->GetEngine();

    // プレイヤーが存在したら
    if (GetTarget())
    {
        // プレイヤーが死んでいたら何もしない
        const auto player = GetTarget()->GetComponent<PlayerComponent>();
        if (player->GetStatePlayer() != PlayerComponent::STATE_PLAYER::ALIVE)
        {
            if (ui_hp_frame && ui_hp_gauge)
            {
                // 体力ゲージを破棄する
                ui_hp_frame->GetOwner()->Destroy();
                ui_hp_gauge->GetOwner()->Destroy();
            }
            characterMovement->velocity = vec3(0);
            animator->SetSpeed(0);
            return;
        }
    }

    // 体力ゲージの表示・非表示、調整
    HpGauge(engine, deltaTime);

    if (!isAngry)
        // 体力が1/3を切っていた場合
        if (Dragon::GetHp() <= HP_MAX / 3)
            // 怒り状態にする
            isAngry = true;

    // 怒りモードに入ったら
    if (isAngry)
        if (angry_effect_span <= 0)
        {
            angry_effect_span = 1.0f;
            // 疑似赤いスーパーサイヤ人
            auto owner = GetOwner();
            const vec3 spawnPosition = { owner->position.x, owner->position.y + 2, owner->position.z };
            auto smoke = owner->GetEngine()->Create<GameObject>("smoke", spawnPosition);
            smoke->AddComponent<Smoke>();
            smoke->materials[0]->emission = { 1.0f, 0.05f, 0.05f };
        }
    angry_effect_span -= deltaTime;

    /*
      ドラゴンの行動について:

      体力が0以下になったら「死亡」に遷移.

      待機: 出現地点の周囲5mにランダムな目的地を設定して移動.
           目的地に到着したら1～5秒静止.
           5または30m以内にプレイヤーが侵入したらランダム時間でレイキャスト. レイが到達したら「追跡」に遷移.
           上記を繰り返す.

      追跡: プレイヤーを追いかける方向に移動.
           ランダム時間で火炎弾を発動し、「火炎弾攻撃」に遷移.
           プレイヤーまでの距離が3m以内ならば「攻撃」に遷移.
           プレイヤーまでの距離が35m以上ならばランダム時間でレイキャスト. レイが到達しなかったら「待機」に遷移.

      火球:
      攻撃: アニメーションが終了したら「追跡」に遷移.
    */
    switch (state)
    {
    case STATE_DRAGON::WAIT: DoWait(deltaTime); break;
    case STATE_DRAGON::WALK: DoWalk(deltaTime); break;
    case STATE_DRAGON::ATTACK: DoAttack(deltaTime); break;
    case STATE_DRAGON::ATTACK_TAIL: DoAttackTail(deltaTime); break;
    case STATE_DRAGON::ATTACK_FIREBALL: DoAttackFireBall(deltaTime); break;
    case STATE_DRAGON::HOVERING: DoHovering(deltaTime); break;
    case STATE_DRAGON::HOVERING_AND_FIRE: DoHoveringAndFire(deltaTime); break;
    case STATE_DRAGON::DEAD: DoDead(deltaTime); break;
    }
}

/// <summary>
/// ダメージを受ける
/// </summary>
/// <param name="damage">ダメージ量</param>
/// <param name="causer">自身</param>
void Dragon::TakeDamage
(
    int damage,
    GameObject* causer
)
{
    // 状態が「死亡」なら何も起こらない
    if (state == STATE_DRAGON::DEAD)
        return;

    // 斬られた音を鳴らす
    EasyAudio::PlayOneShot(SE::sword_hit);

    // ノックバック
    const vec3 v = normalize(GetOwner()->position - causer->position);
    characterMovement->velocity.x += 4 * v.x;
    characterMovement->velocity.z += 4 * v.z;

    // ダメージを受ける
    Dragon::SetHp(damage);

    // 体力が0以下なら
    if (Dragon::GetHp() <= 0)
    {
        // 死亡
        StartDead();

        // 死亡SEを鳴らす
        EasyAudio::PlayOneShot(SE::dragon_dead);
    }
}

/// <summary>
/// 「待機」を開始する
/// </summary>
void Dragon::StartWait()
{
    // 状態が「待機」なら何もしない
    if (state == STATE_DRAGON::WAIT)
        return;

    if (animator)
    {
        // 「待機」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("wait");
    }

    // 状態を「待機」にする
    state = STATE_DRAGON::WAIT;
}

/// <summary>
/// 「追跡」を開始する
/// </summary>
void Dragon::StartWalk()
{
    // 状態が「追跡」なら何もしない
    if (state == STATE_DRAGON::WALK)
        return;

    if (animator)
    {
        // 「追跡」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION * 2);
        animator->Play("walk");
    }

    // 状態を「追跡」にする
    state = STATE_DRAGON::WALK;
}

/// <summary>
/// 「攻撃」を開始する
/// </summary>
void Dragon::StartAttack()
{
    // 状態が「攻撃」なら何もしない
    if (state == STATE_DRAGON::ATTACK)
        return;

    if (animator)
    {
        // 「攻撃」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("attack");
    }

    // ダメージ量を設定する
    attackCollider_right_arm->SetDamage(ATTAK_NORMAL);

    // 攻撃判定を設定する
    attackCollider_right_arm->Activate(1.0f);

    // 怒り状態なら
    if (isAngry)
        // 攻撃回数が残っていたら
        if (attack_max != 0)
        {
            // 攻撃を続ける
            time_attack = 0;

            // 攻撃回数を減らす
            attack_max--;
        }
        else
        {
            // 次に攻撃するまでの時間(0.5～1.5秒)
            time_attack = float(std::rand() % 11) * 0.1f + 0.5f;

            // 攻撃回数を補充する
            attack_max = 2;
        }
    else
        // 次に攻撃するまでの時間(0.5～1.5秒)
        time_attack = float(std::rand() % 11) * 0.1f + 0.5f;

    // 状態を「攻撃」にする
    state = STATE_DRAGON::ATTACK;
}

/// <summary>
/// 「尻尾攻撃」を開始する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Dragon::StartAttackTail()
{
    // 状態が「尻尾攻撃」なら何もしない
    if (state == STATE_DRAGON::ATTACK_TAIL)
        return;

    if (animator)
    {
        // 「尻尾攻撃」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("attack_tail");
    }

    // ダメージ量を設定する
    attackCollider_tail->SetDamage(ATTACK_SPECIAL);

    // 攻撃判定を設定する
    attackCollider_tail->Activate(10.0f);

    // 次に攻撃するまでの時間(0.5～1.5秒)
    time_attack = float(std::rand() % 11) * 0.1f + 0.5f;

    // 攻撃回数の補充
    attack_max = 2;

    // 状態を「尻尾攻撃」にする
    state = STATE_DRAGON::ATTACK_TAIL;
}

/// <summary>
/// 「火炎弾攻撃」を開始する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Dragon::StartAttackFireBall()
{
    // 状態が「火炎弾攻撃」なら何もしない
    if (state == STATE_DRAGON::ATTACK_FIREBALL)
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

    if (animator)
    {
        // 「火炎弾攻撃」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("magic");
    }
    smoke->materials[0]->emission = { 0.2f, 0.1f, 0.1f };

    // 怒り状態なら
    if (isAngry)
        // 攻撃回数が残っていたら
        if (attack_max != 0)
        {
            // 攻撃を続ける
            time_attack_fire_ball = 0;

            // 攻撃回数を減らす
            attack_max--;
        }
        else
        {
            // 次に火炎弾攻撃するまでの時間(0.5～2.0秒)
            time_attack_fire_ball = float(std::rand() % 16) * 0.1f + 0.5f;

            // 攻撃回数を補充する
            attack_max = 2;
        }
    else
        // 次に火炎弾攻撃するまでの時間(0.5～2.0秒)
        time_attack_fire_ball = float(std::rand() % 16) * 0.1f + 0.5f;

    // 火炎弾攻撃準備
    isCasted = false;

    // 状態を「火炎弾攻撃」にする
    state = STATE_DRAGON::ATTACK_FIREBALL;
}

/// <summary>
/// 「空中滞在」を開始する
/// </summary>
void Dragon::StartHovering()
{
    // 状態が「空中滞在」なら何もしない
    if (state == STATE_DRAGON::HOVERING)
        return;

    if (animator)
    {
        // 「空中滞在」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION * 2);
        animator->Play("hovering");
    }

    // 攻撃回数を補充する
    attack_max = 3;

    // 移動先を決定する
    destination = position_hovering[(int)Random::Range(0, 8)];

    // 次に空中滞在するまでの時間(8.0f秒)
    time_hovering = 8.0f;

    // 状態を「空中滞在」にする
    state = STATE_DRAGON::HOVERING;
}

/// <summary>
/// 「空中滞在時に火炎弾攻撃」を開始する
/// </summary>
void Dragon::StartHoveringAndFire()
{
    // 状態が「空中滞在」なら何もしない
    if (state == STATE_DRAGON::HOVERING_AND_FIRE)
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

    if (animator)
    {
        // 「火炎弾攻撃」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("magic");
    }
    smoke->materials[0]->emission = { 0.2f, 0.1f, 0.1f };

    // 攻撃回数を減らす
    attack_max--;

    // 火炎弾攻撃準備
    isCasted = false;

    // 状態を「空中滞在」にする
    state = STATE_DRAGON::HOVERING_AND_FIRE;
}

/// <summary>
/// 「死」を開始する
/// </summary>
void Dragon::StartDead()
{
    // 状態が「死亡」なら何もしない
    if (state == STATE_DRAGON::DEAD)
        return;

    if (animator)
    {
        // 「死」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION);
        animator->Play("dead");
    }

    // 状態を「死亡」にする
    state = STATE_DRAGON::DEAD;

    // 攻撃判定を破棄する
    attackCollider_right_arm->Deactivate();
    attackCollider_tail->Deactivate();
}

/// <summary>
/// 「待機」する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Dragon::DoWait(float deltaTime)
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
            const float sinθ = cross(dirFront, nv).y;
            owner->rotation.y += 5 * sinθ * deltaTime;
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
    const auto targetInfo = GetTargetInfo();
    if (targetInfo)
    {
        float s = DISTANCE_SEARCH;
        if (targetInfo.distance <= s)
        {
            // 敵を発見したので追跡開始
            time_wait = 0;
            time_walk = 0;
            //StartWalk();
            StartHovering();
        }
    }
}

/// <summary>
/// 「追跡」する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Dragon::DoWalk(float deltaTime)
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
    time_attack = std::max(time_attack - deltaTime, 0.0f);
    time_attack_fire_ball = std::max(time_attack_fire_ball - deltaTime, 0.0f);
    time_hovering = std::max(time_hovering - deltaTime, 0.0f);

    const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };
    const vec3 dirLeft = { cos(owner->rotation.y), 0, -sin(owner->rotation.y) };

    if (time_hovering <= 0)
    {
        // 「空中滞在」を開始する
        StartHovering();
        return;
    }
    // 攻撃対象が自分の前にいたら攻撃のチェック、そうでなければ旋回する
    else if (targetInfo.position_type == TARGET_INFO::POSITON_TYPE::FRONT)
    {
        // プレイヤーが攻撃判定圏内であれば
        if (targetInfo.distance <= DISTANCE_ATTACK)
        {
            if (time_attack <= 0)
            {
                // Healthが半分以下なら、「尻尾攻撃」をランダムで選んで行う
                if (isAngry)
                    // ランダムに出た数字が0.5以下なら
                    if (Random::Range(0, 1) <= 0.5f)
                    {
                        // 「尻尾攻撃」を開始する
                        StartAttackTail();
                        return;
                    }

                // 「攻撃」を開始する
                StartAttack();
                return;
            }
        }
        else if (time_attack_fire_ball <= 0)
        {
            // 「火炎弾攻撃」を開始する
            StartAttackFireBall();
            return;
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
    {
        if (isAngry)
            characterMovement->AccelerateXZ(dirFront * SPEED_MOVE_ENEMY * accel * deltaTime, SPEED_MOVE_ENEMY * 0.5f);
        else
            characterMovement->AccelerateXZ(dirFront * SPEED_MOVE_ENEMY * accel * deltaTime, SPEED_MOVE_ENEMY);
    }
    else
        // 逆方向に加速(減速)する
        characterMovement->AccelerateXZ(-dirFront * SPEED_MOVE_ENEMY * accel * deltaTime, SPEED_MOVE_ENEMY);
}

/// <summary>
/// 「攻撃」する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Dragon::DoAttack(float deltaTime)
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
/// 「尻尾攻撃」する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Dragon::DoAttackTail(float deltaTime)
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
/// 「火炎弾攻撃」する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Dragon::DoAttackFireBall(float deltaTime)
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
/// 「空中滞在」する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Dragon::DoHovering(float deltaTime)
{
    auto owner = GetOwner();

    const vec3 v = destination - owner->position;
    const float d2 = dot(v, v);
    if (d2 < 2 * 2)
    {
        // 目的地に到着
        if (attack_max <= 0)
            StartWalk();
        else
            for (int i = 11; i >= 0; i--)
                if (destination.x == position_hovering[i].x &&
                    destination.z == position_hovering[i].z)
                {
                    if (i == 11)
                    {
                        float theta = GetTargetInfo().cosθ_by_left;
                        owner->rotation.y += 30 * theta * deltaTime;
                        if (GetTargetInfo().position_type == TARGET_INFO::POSITON_TYPE::FRONT)
                            StartHoveringAndFire();
                    }

                    else
                        destination = position_hovering[i + 1];

                    break;
                }
    }
    else
    {
        // 目的地に向かって移動中
        const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };
        const vec3 nv = v * (1 / sqrt(d2));
        const float sinθ = cross(dirFront, nv).y;
        owner->rotation.y += 5 * sinθ * deltaTime;
        characterMovement->AccelerateXZ(30 * nv * deltaTime, 6);
        characterMovement->AccelerateY(30 * nv * deltaTime, 6);
    }
}

/// <summary>
/// 「空中滞在時に火炎弾攻撃」する
/// </summary>
/// <param name="deltaTime"></param>
void Dragon::DoHoveringAndFire(float deltaTime)
{
    auto owner = GetOwner();

    // アニメーションが終了したら
    if (!animator->IsPlaying())
    {
        // 「空中滞在」アニメーションを再生する
        animator->SetSpeed(SPEED_ANIMATION * 2);
        animator->Play("walk");

        // 状態を「空中滞在」にする
        state = STATE_DRAGON::HOVERING;

        return;
    }

    // 停止する
    characterMovement->DecelerateXZ(DECELERATION * 10 * deltaTime);

    const vec3 v = destination - owner->position;
    const float d2 = dot(v, v);
    // 浮遊中
    const vec3 dirFront = { sin(owner->rotation.y), 0, cos(owner->rotation.y) };
    const vec3 nv = v * (1 / sqrt(d2));
    characterMovement->AccelerateY(30 * nv * deltaTime, 3);

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
void Dragon::DoDead(float deltaTime)
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
            float y = float(rand() % 6) * 0.05f + 0.25f;//  0.25～0.5m
            float z = float(rand() % 5) * 0.25f - 0.5f; // -0.5～+0.5m
            auto smoke = engine->Create<GameObject>("smoke", owner->position + vec3(x, y, z));
            smoke->AddComponent<Smoke>();
            float b = float(rand() % 5) * 0.25f + 0.25f;
            smoke->materials[0]->emission = vec3{ 0.125f, 0.15f, 0.1f } *b;
        }

        EasyAudio::PlayOneShot(SE::rock_delete);

        mistGenerator->GetOwner()->Destroy();

        // 自身を破棄する
        owner->Destroy();
    }
}

/// <summary>
/// 体力ゲージの表示・非表示、調整
/// </summary>
/// <param name="engine">ゲームエンジン</param>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Dragon::HpGauge
(
    Engine* engine,
    float deltaTime
)
{
    // プレイヤーが近づいたら、ボスの体力を表示する
    const auto targetInfo = GetTargetInfo();
    if (GetTarget()->GetComponent<PlayerComponent>()->GetStatePlayer() == PlayerComponent::STATE_PLAYER::ALIVE && targetInfo.distance <= DISTANCE_SEARCH)
    {
        // 体力ゲージのフレームを生成していなかったら
        if (!ui_hp_frame)
        {
            // 体力ゲージのフレームを生成する
            ui_hp_frame = engine->CreateUIObject<UILayout>("Res/boss_hp_frame.dds", { 0.00f, -0.80f }, 0.05f).second;
            ui_hp_frame->GetOwner()->render_queue = RENDER_QUEUE_OVERLAY + 1;
        }
        // 体力ゲージを生成していなかったら
        if (!ui_hp_gauge)
        {
            // 体力ゲージを生成する
            ui_hp_gauge = engine->CreateUIObject<UILayout>("Res/boss_hp_gauge.dds", { 0.00f, -0.80f }, 0.05f).second;
            ui_hp_gauge->GetOwner()->materials[0]->baseColor = { 1.0f, 0.0f, 0.0f, 1 };
        }

        // 体力ゲージを表示する
        auto gauge = ui_hp_gauge->GetOwner();
        const float aspectRatio = gauge->materials[0]->texBaseColor->GetAspectRatio();
        const float targetRatio = std::clamp(float(Dragon::GetHp()) / float(HP_MAX), 0.0f, 1.0f);

        // 体力ゲージの増減を計算させる
        if (current_ratio > targetRatio)
            current_ratio = std::max(current_ratio - deltaTime * 0.5f, targetRatio);
        else if (current_ratio < targetRatio)
            current_ratio = std::min(current_ratio + deltaTime * 0.5f, targetRatio);

        // 体力ゲージ全体のスケーリング
        switch (mode_scaling)
        {
        case MODE_SCALING::OPEN:
            scale_hp_gauge += deltaTime * 1.5f;
            if (scale_hp_gauge >= 1)
            {
                scale_hp_gauge = 1;
                mode_scaling = MODE_SCALING::STATIONARY;
            }
            break;
        case MODE_SCALING::STATIONARY:
            if (current_ratio <= 0)
                mode_scaling = MODE_SCALING::CLOSE;
            break;

        case MODE_SCALING::CLOSE:
            scale_hp_gauge = std::max(scale_hp_gauge - deltaTime * 1.5f, 0.0f);
            break;
        }

        // 計算結果を体力ゲージの座標、大きさに反映する
        ui_hp_gauge->position_base.x = -(1 - current_ratio) * aspectRatio * scale_hp_gauge * 0.05f;
        gauge->scale.x = current_ratio * aspectRatio * scale_hp_gauge * 0.05f;
        ui_hp_frame->GetOwner()->scale.x = aspectRatio * scale_hp_gauge * 0.05f;
    }
}
