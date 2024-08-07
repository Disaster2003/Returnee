/**
* @file ExplorationScence.cpp
*/
#include "ExplorationScene.h"
#include "../Character/Player/PlayerComponent.h"
#include "GoalEvent.h"
#include "../Character/Enemy/Dragon.h"
#include "../Character/Enemy/Orc.h"
#include "TitleScene.h"
#include "../Engine/BoxCollider.h"
#include "../Engine/SkeletalMeshRenderer.h"
#include "../Engine/Animation.h"
#include "../Effect/Mist.h"

/// <summary>
/// カメラをプレイヤーに追従させるコンポーネント
/// </summary>
class FirstPersonCamera
    : public Component
{
public:
    // デフォルトコンストラクタ
    inline FirstPersonCamera() = default;
    // デフォルトデストラクタ
    inline virtual ~FirstPersonCamera() = default;

    /// <summary>
    /// カメラをプレイヤーに追従させる
    /// </summary>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        auto& camera = engine->GetMainCamera();
        camera.position = owner->position;
        camera.rotation = owner->rotation;
    }
};

/// <summary>
/// 操作ガイド
/// </summary>
class ControlGuide
    : public Component
{
public:
    /// <summary>
    /// 常に操作ガイドの状態を更新する
    /// </summary>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        auto player = owner->GetComponent<PlayerComponent>();

        // プレイヤーがゲームプレイ中でなければ
        if (player->GetStatePlayer() != PlayerComponent::STATE_PLAYER::ALIVE)
            // 自身を破棄する
            ui_guide_action->GetOwner()->Destroy();
        // ボス部屋に到着したら
        else if (isArrivedBossRoom)
        {
            // 操作ガイドを変更していたら
            if (ui_guide_action)
            {
                // 変更後3.0秒は点滅させる
                GuideFlashing(ui_guide_action, deltaTime);

                // 何もしない
                return;
            }
            // 基本操作ガイドを破棄する
            ui_guide_control->GetOwner()->Destroy();
            // 戦闘操作ガイドが生成されていなかったら
            if (!ui_guide_action)
                // 戦闘操作ガイドを生成する
                ui_guide_action = engine->CreateUIObject<UILayout>("Res/guide_action.dds", { -1.40f, 0.00f }, 0.60f).second;
        }
        // 基本操作ガイドが生成されていなかったら
        else if (!ui_guide_control)
            // 基本操作ガイドを生成する
            ui_guide_control = engine->CreateUIObject<UILayout>("Res/guide_base.dds", { -1.40f, 0.00f }, 0.60f).second;
    }

    // ボス部屋に到着したことを設定する
    inline void SetIsArrivedBossRoom() { isArrivedBossRoom = true; }

    /// <summary>
    /// ガイドの疑似点滅
    /// </summary>
    /// <param name="uiGuide">ガイド</param>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    void GuideFlashing
    (
        std::shared_ptr<UILayout> uiGuide,
        float deltaTime
    )
    {
        // 3回点滅したら終了
        if (timer >= 3.0f)
            return;

        // 小数点以下が0.5秒以上の場合
        if (fmod(timer, 1.0f) > 0.5f)
            // 表示する
            uiGuide->position_base.y = 0;
        else
            // 非表示にする
            uiGuide->position_base.y = 10;

        // ガイドの変更した後、時間計測
        timer += deltaTime;
    }

private:
    std::shared_ptr<UILayout> ui_guide_control; // 基本操作ガイド
    std::shared_ptr<UILayout> ui_guide_action;  // 戦闘操作ガイド

    float timer = 0; // ガイドの点滅タイマー

    bool isArrivedBossRoom = false; // ボス部屋へtrue = 到達、false = 未到
};

/// <summary>
/// 目標ガイド
/// </summary>
class GoalGuide
    : public Component
{
public:
    /// <summary>
    /// 常に目標ガイドの状態を更新する
    /// </summary>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        // プレイヤーコンポーネントを取得する
        auto player = owner->GetComponent<PlayerComponent>();

        // プレイヤーがゲームプレイ中でなければ
        if (player->GetStatePlayer() != PlayerComponent::STATE_PLAYER::ALIVE)
        {
            
            if (state_guide == 2)
                // ゴールに触れたら、自身を破棄する
                ui_guide_get_treasure->GetOwner()->Destroy();
            else if(state_guide == 1)
                // ゲームオーバーなら、自身を破棄する
                ui_guide_boss_battle->GetOwner()->Destroy();
            else if(state_guide == 0)
                // ゲームオーバーなら、自身を破棄する
                ui_guide_player_forward->GetOwner()->Destroy();
        }
        // ボスを倒したら
        else if (state_guide == 2)
        {
            // 目標ガイドを変更していたら
            if (ui_guide_get_treasure)
            {
                // 変更後3.0秒は点滅させる
                GuideFlashing(ui_guide_get_treasure, deltaTime);

                // 何もしない
                return;
            }
            // ボス討伐目標ガイドを破棄する
            ui_guide_boss_battle->GetOwner()->Destroy();
            // ゴール間近目標ガイドが生成されていなかったら
            if (!ui_guide_get_treasure)
                // ゴール間近目標ガイドを生成する
                ui_guide_get_treasure = engine->CreateUIObject<UILayout>("Res/goal_get_treasure.dds", { 1.10f, 0.80f }, 0.20f).second;
        }
        // ボス部屋に到着したら
        else if (state_guide == 1)
        {
            // 目標ガイドを変更していたら
            if (ui_guide_boss_battle)
            {
                // 変更後3.0秒は点滅させる
                GuideFlashing(ui_guide_boss_battle, deltaTime);

                // 何もしない
                return;
            }
            // 初期目標ガイドを破棄する
            ui_guide_player_forward->GetOwner()->Destroy();
            // ボス討伐目標ガイドが生成されていなかったら
            if (!ui_guide_boss_battle)
                // ボス討伐目標ガイドを生成する
                ui_guide_boss_battle = engine->CreateUIObject<UILayout>("Res/goal_kill_dragon.dds", { 1.10f, 0.80f }, 0.20f).second;
        }
        // ゲームが始まったら
        else if (!ui_guide_player_forward)
                // 初期目標ガイドを生成する
                ui_guide_player_forward = engine->CreateUIObject<UILayout>("Res/goal_go_straight.dds", { 1.10f, 0.80f }, 0.20f).second;
    }

    // 次の段階へ設定する
    inline void SetGuideState() { state_guide = static_cast<STATE_GUIDE>(static_cast<int>(state_guide) + 1); }

    /// <summary>
    /// ガイドの疑似点滅
    /// </summary>
    /// <param name="uiGuide">ガイド</param>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    void GuideFlashing
    (
        std::shared_ptr<UILayout> uiGuide,
        float deltaTime
    )
    {
        // 3回点滅したら終了
        if (timer >= 3.0f)
            return;

        // 小数点以下が0.5秒以上の場合
        if (fmod(timer, 1.0f) > 0.5f)
            // 表示する
            uiGuide->position_base.y = 0.80f;
        else
            // 非表示にする
            uiGuide->position_base.y = 10;

        // ガイドの変更した後、時間計測
        timer += deltaTime;
    }

private:
    enum STATE_GUIDE
    {
        PLAYER_FORWARD,
        BOSS_BATTLE,
        GET_TREASURE,
    };
    STATE_GUIDE state_guide = PLAYER_FORWARD;

    std::shared_ptr<UILayout> ui_guide_player_forward; // 初期目標ガイド
    std::shared_ptr<UILayout> ui_guide_boss_battle;    // ボス討伐目標ガイド
    std::shared_ptr<UILayout> ui_guide_get_treasure;   // ゴール間近目標ガイド

    float timer = 0; // ガイドの点滅タイマー
};

/// <summary>
/// ボス起動用トリガー
/// </summary>
class BossTrigger
    : public Component
{
public:
    /// <summary>
    /// 常にプレイヤーが侵入したかどうか監視し、入り口にある岩の状態を更新する
    /// </summary>
    /// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
    virtual void Update(float deltaTime) override
    {
        if (!doClosing || isClosed || !enter)
            return;
        
        // 岩石を上げる
        enter->position.y += 8 * deltaTime;
        float vol = EasyAudio::GetVolume(AudioPlayer::bgm);
        EasyAudio::SetVolume(AudioPlayer::bgm, std::max(vol - deltaTime, 0.0f));
        if (enter->position.y >= 0.5f)
        {
            /* 完了 */
            // BossBGMを再生する
            EasyAudio::Stop(AudioPlayer::bgm);
            EasyAudio::Play(AudioPlayer::bgm, BGM::boss, 1, true);
            enter->position.y = 0.5f;
            isClosed = true;
            guide_goal->SetGuideState();
            GetOwner()->Destroy();
        }
    }

    /// <summary>
    /// 衝突時に衝突した相手によって処理を行う
    /// </summary>
    /// <param name="self">自身</param>
    /// <param name="other">衝突した相手</param>
    virtual void OnCollision
    (
        const ComponentPtr& self,
        const ComponentPtr& other
    ) override
    {
        // 岩が動いてない状態かつ、プレイヤーに触れたら
        if (!doClosing && other->GetOwner()->name == "player")
        {
            // 岩の起動
            EasyAudio::PlayOneShot(SE::rock_close);
            doClosing = true;
            if (boss && boss->GetOwner())
                boss->SetSearchDistance(30);
        }
    }

    GameObjectPtr enter;                   // 入り口に埋まってる岩
    std::shared_ptr<GoalGuide> guide_goal; // 目標ガイド
    std::shared_ptr<Dragon> boss;          // ボス

private:
    bool doClosing = false; // 岩をtrue = 動かす、false = 動かさない
    bool isClosed = false;  // 岩で入り口をtrue = 封鎖、false = 通行
};

/// <summary>
/// 雑魚敵起動用トリガー
/// </summary>
class EnemyTrigger
    : public Component
{
public:
    /// <summary>
    /// 衝突時に衝突した相手によって処理を行う
    /// </summary>
    /// <param name="self">自身</param>
    /// <param name="other">衝突した相手</param>
    virtual void OnCollision
    (
        const ComponentPtr& self,
        const ComponentPtr& other
    ) override
    {
        // 岩が動いてない状態かつ、プレイヤーに触れたら
        if (other->GetOwner()->name == "player")
        {
            guide_control->SetIsArrivedBossRoom();
            GetOwner()->Destroy();
        }
    }

    std::shared_ptr<ControlGuide> guide_control; // 操作ガイド
};

/// <summary>
/// ボスを倒したら、ゴール前の障害物を動かす
/// </summary>
class GoalWay
    : public Component
{
public:
    /// <summary>
    /// ボスが死んだら、ついでゴール前の岩をどかす
    /// </summary>
    virtual void OnDestroy() override
    {
        while (1)
        {
            // 岩石を下ろす
            exit->position.y -= 1.0f;
            if (exit->position.y <= -4.0f)
            {
                exit->position.y = -4.0f;
                guide_goal->SetGuideState();
                break;
            }
        }
    }

    GameObjectPtr exit;
    std::shared_ptr<GoalGuide> guide_goal;
};

/// <summary>
/// シーンを初期化する
/// </summary>
/// <param name="engine">ゲームエンジン</param>
/// <returns>true : 初期化成功、false : 初期化失敗</returns>
bool ExplorationScene::Initialize(Engine& engine)
{
    // PlayBGMを再生する
    EasyAudio::Stop(AudioPlayer::bgm);
    EasyAudio::Play(AudioPlayer::bgm, BGM::play, 1, true);

    // ゲームオブジェクト配置ファイルを読み込む
    Engine::FilepathMap filepathMap;
    filepathMap["Floor"] = "Res/MeshData/a_piece_of_nature/Floor.obj";
    filepathMap["cliff"] = "Res/MeshData/free_rocks/Cliff.obj";
    filepathMap["Enter"] = "Res/MeshData/free_rocks/Wall.obj";
    filepathMap["Exit"] = "Res/MeshData/free_rocks/Wall.obj";
    filepathMap["Chest"] = "Res/MeshData/a_piece_of_nature/Chest.obj";
    filepathMap["sticker_punch"] = "Res/MeshData/sticker/punch.obj";
    filepathMap["sticker_magic"] = "Res/MeshData/sticker/magic.obj";
    filepathMap["sticker_arrow_down"] = "Res/MeshData/sticker/arrow_down.obj";
    filepathMap["sticker_dragon"] = "Res/MeshData/sticker/dragon.obj";
    filepathMap["sticker_dragon_reverse"] = "Res/MeshData/sticker/dragon_reverse.obj";
    auto list = engine.LoadGameObjectMap("Res/GameObjectMap.json", filepathMap);

    // プレイヤーを配置する
    auto player = engine.Create<GameObject>("player", { 0, 1.5f, 0 });
    playerComponent = player->AddComponent<PlayerComponent>();
    player->AddComponent<FirstPersonCamera>();
    player->rotation.y = radians(180);

    // スカイスフィアを設定する
    material_skysphere = std::make_shared<MATERIAL>();
    material_skysphere->texBaseColor = engine.GetTexture("Res/MeshData/sky_sphere/sky.dds");

    // 環境キューブマップを設定する
    static const char* const cubemapFilenames[6] =
    {
        "Res/cubemap/px.dds",
        "Res/cubemap/nx.dds",
        "Res/cubemap/py.dds",
        "Res/cubemap/ny.dds",
        "Res/cubemap/pz.dds",
        "Res/cubemap/nz.dds",
    };
    engine.SetEnvironmentCubemap(
        engine.LoadCubemapTexture("environment", cubemapFilenames));

#pragma region DRAGON_BONEANIMATION
    // 基本浮遊させる
    static const ANIMATION_KEYFRAME kfBody[] =
    {
      {0.00f, vec3(0, 1.00f, 0)}, // 初期位置
      {1.00f, vec3(0, 0.50f, 0)}, // 下に移動
      {2.00f, vec3(0, 1.00f, 0)}, // 上に移動(初期位置に戻る)
    };

    // ドラゴンの待機
    auto clipDragonWait = std::make_shared<AnimationClip>();
    {
        auto curveDragonWaitBody = std::make_shared<AnimationCurve>();
        curveDragonWaitBody->AddKey(std::begin(kfBody), std::end(kfBody));
        clipDragonWait->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonWaitBody);
        clipDragonWait->SetLoopFlag(true); // ループする
    }
    // ドラゴンの歩行
    auto clipDragonWalk = std::make_shared<AnimationClip>();
    {
        auto curveDragonWalkBodyPos = std::make_shared<AnimationCurve>();
        curveDragonWalkBodyPos->AddKey(std::begin(kfBody), std::end(kfBody));

        auto curveDragonWalkBodyRot = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.00f, vec3(0.30f, 0, 0)}, // 少し傾き
        };
        curveDragonWalkBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        clipDragonWalk->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonWalkBodyPos);
        clipDragonWalk->AddCurve(0, ANIMATION_TARGET::ROTATION, curveDragonWalkBodyRot);
        clipDragonWalk->SetLoopFlag(true); // ループする
    }
    // ドラゴンの攻撃
    auto clipDragonAttack = std::make_shared<AnimationClip>();
    {
        auto curveDragonAttackBodyPos = std::make_shared<AnimationCurve>();
        curveDragonAttackBodyPos->AddKey(std::begin(kfBody), std::end(kfBody));

        auto curveDragonAttackBodyRot = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.00f, vec3(0.30f, 0,     0)}, // 少し傾き
          {0.50f, vec3(0.50f, 1.00f, 0)}, // 少し傾き、左回転
          {1.00f, vec3(0.30f, 0,     0)}, // 少し傾き
        };
        curveDragonAttackBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        clipDragonAttack->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonAttackBodyPos);
        clipDragonAttack->AddCurve(0, ANIMATION_TARGET::ROTATION, curveDragonAttackBodyRot);
        clipDragonAttack->SetLoopFlag(false); // ループしない
    }
    // ドラゴンの尻尾攻撃
    auto clipDragonAttackTail = std::make_shared<AnimationClip>();
    {
        static const ANIMATION_KEYFRAME kfBodyPos[] =
        {
          {0.00f, vec3(0, 1.00f, 0)}, // 初期位置
          {0.40f, vec3(0)},           // 下に移動
          {1.50f, vec3(0)},           // 下に移動
          {2.00f, vec3(0, 1.00f, 0)}, // 上に移動(初期位置に戻る)
        };
        auto curveDragonAttackTailBodyPos = std::make_shared<AnimationCurve>();
        curveDragonAttackTailBodyPos->AddKey(std::begin(kfBodyPos), std::end(kfBodyPos));

        auto curveDragonAttackTailBodyRot = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.00f, vec3(0.30f, 0,     0)}, // 少し傾き
          {0.50f, vec3(0.30f, 4.00f, 0)}, // 尾を振り回す
          {1.30f, vec3(0.30f, 0,     0)}, // 少し傾き
        };
        curveDragonAttackTailBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        clipDragonAttackTail->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonAttackTailBodyPos);
        clipDragonAttackTail->AddCurve(0, ANIMATION_TARGET::ROTATION, curveDragonAttackTailBodyRot);
        clipDragonAttackTail->SetLoopFlag(false); // ループしない
    }
    // ドラゴンの火球
    auto clipDragonMagic = std::make_shared<AnimationClip>();
    {
        auto curveDragonMagicBodyPos = std::make_shared<AnimationCurve>();
        curveDragonMagicBodyPos->AddKey(std::begin(kfBody), std::end(kfBody));

        auto curveDragonMagicBodyRot = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.00f, vec3(-0.15f, 0, 0)}, // 後ろにのけぞる
          {1.00f, vec3( 0.30f, 0, 0)}, // 前に傾く
        };
        curveDragonMagicBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        clipDragonMagic->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonMagicBodyPos);
        clipDragonMagic->AddCurve(0, ANIMATION_TARGET::ROTATION, curveDragonMagicBodyRot);
        clipDragonMagic->SetLoopFlag(false); // ループしない
    }
    // ドラゴンの空中飛行
    auto clipDragonHovering = std::make_shared<AnimationClip>();
    {
        auto curveDragonHoveringBodyPos = std::make_shared<AnimationCurve>();
        curveDragonHoveringBodyPos->AddKey(std::begin(kfBody), std::end(kfBody));

        auto curveDragonHoveringBodyRot = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.00f, vec3(1.0f, 0, 0)}, // 少し傾き
        };
        curveDragonHoveringBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        clipDragonHovering->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonHoveringBodyPos);
        clipDragonHovering->AddCurve(0, ANIMATION_TARGET::ROTATION, curveDragonHoveringBodyRot);
        clipDragonHovering->SetLoopFlag(true); // ループする
    }
    // ドラゴンの死
    auto clipDragonDead = std::make_shared<AnimationClip>();
    {
        auto curveDragonDeadBodyPos = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyPos[] =
        {
          {0.00f, vec3(0, 1.00f, 0)}, // 段々と上から
          {2.00f, vec3(0)},           // 下に降りてくる
        };
        curveDragonDeadBodyPos->AddKey(std::begin(kfBodyPos), std::end(kfBodyPos));

        auto curveDragonDeadBodyRot = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.00f, vec3(0.30f, 0, 0)}, // 段々と前に
          {1.00f, vec3(1.00f, 0, 0)}, // 傾く
        };
        curveDragonDeadBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        clipDragonDead->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonDeadBodyPos);
        clipDragonDead->AddCurve(0, ANIMATION_TARGET::ROTATION, curveDragonDeadBodyRot);
        clipDragonDead->SetLoopFlag(false); // ループしない
    }
#pragma endregion

#pragma region ORCSHAMAN_BONEANIMATION
    // オークの待機
    auto clipOrcWait = std::make_shared<AnimationClip>();
    {
        /* 疑似的に呼吸させる */
        static const ANIMATION_KEYFRAME kfBody[] =
        {
          {0.0f, vec3(0, 0.00f, 0.00f)},
          {0.1f, vec3(0,-0.01f, 0.01f)},
          {0.4f, vec3(0,-0.04f, 0.03f)},
          {0.5f, vec3(0,-0.05f, 0.04f)},
          {0.6f, vec3(0,-0.04f, 0.03f)},
          {0.9f, vec3(0,-0.01f, 0.01f)},
          {1.0f, vec3(0, 0.00f, 0.00f)}
        };
        auto curveBody = std::make_shared<AnimationCurve>();
        curveBody->AddKey(std::begin(kfBody), std::end(kfBody));

        static const ANIMATION_KEYFRAME kfHead[] =
        {
          {0.00f, vec3(0, 0.00f, 0)},
          {0.15f, vec3(0,-0.01f, 0)},
          {0.44f, vec3(0,-0.04f, 0)},
          {0.53f, vec3(0,-0.05f, 0)},
          {0.62f, vec3(0,-0.04f, 0)},
          {0.91f, vec3(0,-0.01f, 0)},
          {1.00f, vec3(0, 0.00f, 0)}
        };
        auto curveHead = std::make_shared<AnimationCurve>();
        curveHead->AddKey(std::begin(kfHead), std::end(kfHead));

        static const ANIMATION_KEYFRAME kfArmR[] =
        {
          {0.0f, vec3(-0.50f, 0, 1.17f)},
          {0.1f, vec3(-0.52f, 0, 1.16f)},
          {0.4f, vec3(-0.58f, 0, 1.11f)},
          {0.5f, vec3(-0.60f, 0, 1.10f)},
          {0.6f, vec3(-0.58f, 0, 1.11f)},
          {0.9f, vec3(-0.52f, 0, 1.16f)},
          {1.0f, vec3(-0.50f, 0, 1.17f)}
        };
        auto curveArmR = std::make_shared<AnimationCurve>();
        curveArmR->AddKey(std::begin(kfArmR), std::end(kfArmR));

        static const ANIMATION_KEYFRAME kfArmL[] =
        {
          {0.0f, vec3( 0.00f, 0, -1.17f)},
          {0.1f, vec3(-0.02f, 0, -1.16f)},
          {0.4f, vec3(-0.08f, 0, -1.11f)},
          {0.5f, vec3(-0.10f, 0, -1.10f)},
          {0.6f, vec3(-0.08f, 0, -1.11f)},
          {0.9f, vec3( 0.02f, 0, -1.16f)},
          {1.0f, vec3( 0.00f, 0, -1.17f)}
        };
        auto curveArmL = std::make_shared<AnimationCurve>();
        curveArmL->AddKey(std::begin(kfArmL), std::end(kfArmL));

        static const ANIMATION_KEYFRAME kfLegRot[] =
        {
          {0.0f, vec3(0.00f, 0, 0)},
          {0.1f, vec3(0.02f, 0, 0)},
          {0.4f, vec3(0.08f, 0, 0)},
          {0.5f, vec3(0.10f, 0, 0)},
          {0.6f, vec3(0.08f, 0, 0)},
          {0.9f, vec3(0.02f, 0, 0)},
          {1.0f, vec3(0.00f, 0, 0)}
        };
        auto curveLegRot = std::make_shared<AnimationCurve>();
        curveLegRot->AddKey(std::begin(kfLegRot), std::end(kfLegRot));

        static const ANIMATION_KEYFRAME kfLegPos[] =
        {
          {0.0f, vec3(0, 0.00f, 0)},
          {0.1f, vec3(0, 0.01f, 0)},
          {0.4f, vec3(0, 0.04f, 0)},
          {0.5f, vec3(0, 0.05f, 0)},
          {0.6f, vec3(0, 0.04f, 0)},
          {0.9f, vec3(0, 0.01f, 0)},
          {1.0f, vec3(0, 0.00f, 0)}
        };
        auto curveLegPos = std::make_shared<AnimationCurve>();
        curveLegPos->AddKey(std::begin(kfLegPos), std::end(kfLegPos));

        clipOrcWait->AddCurve(0, ANIMATION_TARGET::ROTATION, curveBody);
        clipOrcWait->AddCurve(1, ANIMATION_TARGET::ROTATION, curveHead);
        clipOrcWait->AddCurve(2, ANIMATION_TARGET::ROTATION, curveArmR);
        clipOrcWait->AddCurve(3, ANIMATION_TARGET::ROTATION, curveArmL);
        clipOrcWait->AddCurve(4, ANIMATION_TARGET::POSITION, curveLegPos);
        clipOrcWait->AddCurve(5, ANIMATION_TARGET::POSITION, curveLegPos);
        clipOrcWait->AddCurve(4, ANIMATION_TARGET::ROTATION, curveLegRot);
        clipOrcWait->AddCurve(5, ANIMATION_TARGET::ROTATION, curveLegRot);
        clipOrcWait->SetLoopFlag(true); // ループする
    }

    // オークの歩行
    auto clipOrcWalk = std::make_shared<AnimationClip>();
    {
        // 胴体を上下に移動するカーブ
        static const ANIMATION_KEYFRAME kfBody[] =
        {
          {0.00f, vec3(0)},            // 初期位置に配置する
          {0.50f, vec3(0, -0.10f, 0)}, // 下に移動
          {1.00f, vec3(0)},            // 上に移動(初期位置に戻る)
          {1.50f, vec3(0, -0.10f, 0)}, // 下に移動
          {2.00f, vec3(0)}             // 上に移動(初期位置に戻る)
        };
        auto curveBody = std::make_shared<AnimationCurve>();
        curveBody->AddKey(std::begin(kfBody), std::end(kfBody));

        // 右腕を下ろして前後に回転させるカーブ
        static const ANIMATION_KEYFRAME kfArmR[] =
        {
          {0.0f, vec3( 0   , 0, 1.17f)}, // 初期位置に配置する
          {0.5f, vec3( 0.5f, 0, 1.17f)}, // 後ろに振る
          {1.5f, vec3(-0.5f, 0, 1.17f)}, // 前に降る
          {2.0f, vec3( 0   , 0, 1.17f)}  // 初期位置に戻る
        };
        auto curveArmR = std::make_shared<AnimationCurve>();
        curveArmR->AddKey(std::begin(kfArmR), std::end(kfArmR));

        // 左腕を下ろして前後に回転させるカーブ
        static const ANIMATION_KEYFRAME kfArmL[] =
        {
          {0.0f, vec3( 0   , 0, -1.17f)}, // 初期位置に配置する
          {0.5f, vec3(-0.5f, 0, -1.17f)}, // 前に降る
          {1.5f, vec3( 0.5f, 0, -1.17f)}, // 後ろに振る
          {2.0f, vec3( 0   , 0, -1.17f)}  // 初期位置に戻る
        };
        auto curveArmL = std::make_shared<AnimationCurve>();
        curveArmL->AddKey(std::begin(kfArmL), std::end(kfArmL));

        // 右足を前後に回転させるカーブ
        static const ANIMATION_KEYFRAME kfLegR[] =
        {
          {0.0f, vec3(0)},           // 初期位置に配置する
          {0.5f, vec3(-0.5f, 0, 0)}, // 前に出す
          {1.5f, vec3( 0.5f, 0, 0)}, // 後ろに出す
          {2.0f, vec3(0)}            // 初期位置に戻る
        };
        auto curveLegR = std::make_shared<AnimationCurve>();
        curveLegR->AddKey(std::begin(kfLegR), std::end(kfLegR));

        // 左足を前後に回転させるカーブ
        static const ANIMATION_KEYFRAME kfLegL[] =
        {
          {0.0f, vec3(0)},           // 初期位置に配置する
          {0.5f, vec3( 0.5f, 0, 0)}, // 後ろに出す
          {1.5f, vec3(-0.5f, 0, 0)}, // 前に出す
          {2.0f, vec3(0)}            // 初期位置に戻る
        };   
        auto curveLegL = std::make_shared<AnimationCurve>();
        curveLegL->AddKey(std::begin(kfLegL), std::end(kfLegL));

        clipOrcWalk->AddCurve(0, ANIMATION_TARGET::POSITION, curveBody);
        clipOrcWalk->AddCurve(2, ANIMATION_TARGET::ROTATION, curveArmR);
        clipOrcWalk->AddCurve(3, ANIMATION_TARGET::ROTATION, curveArmL);
        clipOrcWalk->AddCurve(4, ANIMATION_TARGET::ROTATION, curveLegR);
        clipOrcWalk->AddCurve(5, ANIMATION_TARGET::ROTATION, curveLegL);
        clipOrcWalk->SetLoopFlag(true);
    }

    // オークの物理攻撃(横)
    auto clipOrcAttackRight = std::make_shared<AnimationClip>();
    {
        /* 横に振り回す */
        static const ANIMATION_KEYFRAME kfBodyPos[] =
        {
          {0.0f, vec3(0)},
          {0.4f, vec3(0, 0.0f,-0.2f)},
          {0.6f, vec3(0,-0.1f, 0.2f)},
          {0.7f, vec3(0,-0.1f, 0.2f)},
          {1.0f, vec3(0,-0.1f, 0.2f)}
        };
        auto curveBodyPos = std::make_shared<AnimationCurve>();
        curveBodyPos->AddKey(std::begin(kfBodyPos), std::end(kfBodyPos));

        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.0f, vec3(0)},
          {0.4f, vec3(-0.1f, 0.4f, 0)},
          {0.6f, vec3( 0.4f,-0.2f, 0)},
          {0.7f, vec3( 0.3f,-0.3f, 0)},
          {1.0f, vec3( 0.3f,-0.3f, 0)}
        };
        auto curveBodyRot = std::make_shared<AnimationCurve>();
        curveBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        static const ANIMATION_KEYFRAME kfHead[] =
        {
          {0.0f, vec3(0)},
          {0.4f, vec3( 0.1f,-0.4f, 0)},
          {0.6f, vec3(-0.4f, 0.2f, 0)},
          {0.7f, vec3(-0.3f, 0.3f, 0)},
          {1.0f, vec3(-0.3f, 0.3f, 0)}
        };
        auto curveHead = std::make_shared<AnimationCurve>();
        curveHead->AddKey(std::begin(kfHead), std::end(kfHead));

        static const ANIMATION_KEYFRAME kfArmR[] =
        {
          {0.0f, vec3(0.0f, 0.0f, 1.17f)},
          {0.4f, vec3(0.3f, 2.0f, 0.00f)},
          {0.6f, vec3(0.0f,-0.7f, 0.30f)},
          {0.7f, vec3(0.0f,-0.5f, 0.20f)},
          {1.0f, vec3(0.0f,-0.5f, 0.20f)}
        };
        auto curveArmR = std::make_shared<AnimationCurve>();
        curveArmR->AddKey(std::begin(kfArmR), std::end(kfArmR));

        static const ANIMATION_KEYFRAME kfArmL[] =
        {
          {0.0f, vec3( 0.0f, 0,-1.17f)},
          {0.4f, vec3(-1.0f, 0,-1.17f)},
          {0.6f, vec3( 0.0f, 0,-1.17f)},
          {0.7f, vec3( 0.5f, 0,-1.17f)},
          {1.0f, vec3( 0.5f, 0,-1.17f)}
        };
        auto curveArmL = std::make_shared<AnimationCurve>();
        curveArmL->AddKey(std::begin(kfArmL), std::end(kfArmL));

        static const ANIMATION_KEYFRAME kfLegR[] =
        {
          {0.0f, vec3(0)},
          {0.4f, vec3(-0.4f,-0.4f, 0.0f)},
          {0.6f, vec3(-0.4f, 0.4f,-0.3f)},
          {0.7f, vec3(-0.4f, 0.4f,-0.2f)},
          {1.0f, vec3(-0.4f, 0.4f,-0.2f)}
        };
        auto curveLegR = std::make_shared<AnimationCurve>();
        curveLegR->AddKey(std::begin(kfLegR), std::end(kfLegR));

        static const ANIMATION_KEYFRAME kfLegL[] =
        {
          {0.0f, vec3(0)},
          {0.4f, vec3(0)},
          {0.6f, vec3(0.4f, 0.5f, 0.0f)},
          {0.7f, vec3(0.4f, 0.5f, 0.0f)},
          {1.0f, vec3(0.4f, 0.5f, 0.0f)}
        };
        auto curveLegL = std::make_shared<AnimationCurve>();
        curveLegL->AddKey(std::begin(kfLegL), std::end(kfLegL));

        clipOrcAttackRight->AddCurve(0, ANIMATION_TARGET::POSITION, curveBodyPos);
        clipOrcAttackRight->AddCurve(0, ANIMATION_TARGET::ROTATION, curveBodyRot);
        clipOrcAttackRight->AddCurve(1, ANIMATION_TARGET::ROTATION, curveHead);
        clipOrcAttackRight->AddCurve(2, ANIMATION_TARGET::ROTATION, curveArmR);
        clipOrcAttackRight->AddCurve(3, ANIMATION_TARGET::ROTATION, curveArmL);
        clipOrcAttackRight->AddCurve(4, ANIMATION_TARGET::ROTATION, curveLegR);
        clipOrcAttackRight->AddCurve(5, ANIMATION_TARGET::ROTATION, curveLegL);
    }

    // オークシャーマンの攻撃(縦)
    auto clipOrcAttack = std::make_shared<AnimationClip>();
    {
        /* 縦に振り回す */
        static const ANIMATION_KEYFRAME kfBody[] =
        {
          {0.0f, vec3(0)},
          {0.4f, vec3(-0.1f,-0.3f, 0)},
          {0.6f, vec3( 0.2f, 0.6f, 0)},
          {0.7f, vec3( 0.2f, 0.7f, 0)},
          {1.0f, vec3( 0.2f, 0.7f, 0)}
        };
        auto curveBody = std::make_shared<AnimationCurve>();
        curveBody->AddKey(std::begin(kfBody), std::end(kfBody));

        static const ANIMATION_KEYFRAME kfBodyPos[] =
        {
          {0.0f, vec3(0)},
          {0.4f, vec3(0, 0,-0.2f)},
          {0.6f, vec3(0, 0, 0.2f)},
          {0.7f, vec3(0, 0, 0.2f)},
          {1.0f, vec3(0, 0, 0.2f)}
        };
        auto curveBodyPos = std::make_shared<AnimationCurve>();
        curveBodyPos->AddKey(std::begin(kfBodyPos), std::end(kfBodyPos));

        static const ANIMATION_KEYFRAME kfHead[] =
        {
          {0.0f, vec3(0)},
          {0.4f, vec3( 0.1f, 0.3f, 0)},
          {0.6f, vec3(-0.2f,-0.6f, 0)},
          {0.7f, vec3(-0.2f,-0.7f, 0)},
          {1.0f, vec3(-0.2f,-0.7f, 0)}
        };
        auto curveHead = std::make_shared<AnimationCurve>();
        curveHead->AddKey(std::begin(kfHead), std::end(kfHead));

        static const ANIMATION_KEYFRAME kfArmR[] =
        {
          {0.0f, vec3( 0.0f, 0, 1.17f)},
          {0.4f, vec3(-2.5f, 0, 1.17f)},
          {0.6f, vec3(-0.8f, 0, 1.17f)},
          {0.7f, vec3(-1.0f, 0, 1.27f)},
          {1.0f, vec3(-1.0f, 0, 1.27f)}
        };
        auto curveArmR = std::make_shared<AnimationCurve>();
        curveArmR->AddKey(std::begin(kfArmR), std::end(kfArmR));

        static const ANIMATION_KEYFRAME kfArmL[] =
        {
          {0.0f, vec3( 0.0f, 0,-1.17f)},
          {0.4f, vec3(-1.0f, 0,-1.17f)},
          {0.6f, vec3( 0.0f, 0,-1.17f)},
          {0.7f, vec3( 0.5f, 0,-1.17f)},
          {1.0f, vec3( 0.5f, 0,-1.17f)}
        };
        auto curveArmL = std::make_shared<AnimationCurve>();
        curveArmL->AddKey(std::begin(kfArmL), std::end(kfArmL));

        static const ANIMATION_KEYFRAME kfLegL[] =
        {
          {0.0f, vec3(0)},
          {0.4f, vec3(0)},
          {0.6f, vec3(0.5f,-0.2f, 0.2f)},
          {0.7f, vec3(0.5f,-0.2f, 0.2f)},
          {1.0f, vec3(0.5f,-0.2f, 0.2f)}
        };
        auto curveLegL = std::make_shared<AnimationCurve>();
        curveLegL->AddKey(std::begin(kfLegL), std::end(kfLegL));

        static const ANIMATION_KEYFRAME kfLegR[] =
        {
          {0.0f, vec3(0)},
          {0.4f, vec3( 0.1f, 0.3f, 0.0f)},
          {0.6f, vec3(-0.3f,-0.6f, 0.2f)},
          {0.7f, vec3(-0.3f,-0.7f, 0.2f)},
          {1.0f, vec3(-0.3f,-0.7f, 0.2f)}
        };
        auto curveLegR = std::make_shared<AnimationCurve>();
        curveLegR->AddKey(std::begin(kfLegR), std::end(kfLegR));

        clipOrcAttack->AddCurve(0, ANIMATION_TARGET::ROTATION, curveBody);
        clipOrcAttack->AddCurve(0, ANIMATION_TARGET::POSITION, curveBodyPos);
        clipOrcAttack->AddCurve(1, ANIMATION_TARGET::ROTATION, curveHead);
        clipOrcAttack->AddCurve(2, ANIMATION_TARGET::ROTATION, curveArmR);
        clipOrcAttack->AddCurve(3, ANIMATION_TARGET::ROTATION, curveArmL);
        clipOrcAttack->AddCurve(4, ANIMATION_TARGET::ROTATION, curveLegR);
        clipOrcAttack->AddCurve(5, ANIMATION_TARGET::ROTATION, curveLegL);
    }

    // オークシャーマンの魔法攻撃
    auto clipOrcMagic = std::make_shared<AnimationClip>();
    {
        /* 杖を前にかざす */
        static const ANIMATION_KEYFRAME kfBody[] =
        {
          {0.0f, vec3(0.0f, 0.0f, 0)},
          {0.3f, vec3(0.0f,-0.2f, 0)},
          {0.6f, vec3(0.0f,-0.2f, 0)},
          {0.8f, vec3(0.2f, 0.3f, 0)},
          {0.9f, vec3(0.2f, 0.2f, 0)},
          {1.1f, vec3(0.2f, 0.2f, 0)}
        };
        auto curveBody = std::make_shared<AnimationCurve>();
        curveBody->AddKey(std::begin(kfBody), std::end(kfBody));

        static const ANIMATION_KEYFRAME kfBodyInv[] =
        {
          {0.0f, vec3( 0.0f, 0.0f, 0)},
          {0.3f, vec3( 0.0f, 0.2f, 0)},
          {0.6f, vec3( 0.0f, 0.2f, 0)},
          {0.8f, vec3(-0.2f,-0.3f, 0)},
          {0.9f, vec3(-0.2f,-0.2f, 0)},
          {1.1f, vec3(-0.2f,-0.2f, 0)}
        };
        auto curveBodyInv = std::make_shared<AnimationCurve>();
        curveBodyInv->AddKey(std::begin(kfBodyInv), std::end(kfBodyInv));

        static const ANIMATION_KEYFRAME kfBodyPos[] =
        {
          {0.0f, vec3(0)},
          {0.3f, vec3(0)},
          {0.6f, vec3(0)},
          {0.8f, vec3(0, 0, 0.4f)},
          {0.9f, vec3(0, 0, 0.4f)},
          {1.1f, vec3(0, 0, 0.4f)}
        };
        auto curveBodyPos = std::make_shared<AnimationCurve>();
        curveBodyPos->AddKey(std::begin(kfBodyPos), std::end(kfBodyPos));

        static const ANIMATION_KEYFRAME kfArmR[] =
        {
          {0.0f,  vec3( 0.0f,  0, 1.17f)},
          {0.3f,  vec3(-1.05f, 0, 1.17f)},
          {0.35f, vec3(-1.0f,  0, 1.17f)},
          {0.6f,  vec3(-1.0f,  0, 1.17f)},
          {0.8f,  vec3(-1.6f,  0, 1.57f)},
          {0.9f,  vec3(-1.5f,  0, 1.57f)},
          {1.1f,  vec3(-1.5f,  0, 1.57f)}
        };
        auto curveArmR = std::make_shared<AnimationCurve>();
        curveArmR->AddKey(std::begin(kfArmR), std::end(kfArmR));

        static const ANIMATION_KEYFRAME kfArmL[] =
        {
          {0.0f,  vec3(0.0f, 0.0f,-1.17f)},
          {0.3f,  vec3(0.0f,-1.5f, 0.05f)},
          {0.35f, vec3(0.0f,-1.5f,-0.00f)},
          {0.6f,  vec3(0.0f,-1.5f,-0.00f)},
          {0.8f,  vec3(0.5f, 0.5f,-0.57f)},
          {0.9f,  vec3(0.5f, 0.0f,-0.57f)},
          {1.1f,  vec3(0.5f, 0.0f,-0.57f)}
        };
        auto curveArmL = std::make_shared<AnimationCurve>();
        curveArmL->AddKey(std::begin(kfArmL), std::end(kfArmL));

        static const ANIMATION_KEYFRAME kfLegL[] =
        {
          {0.0f, vec3(0)},
          {0.3f, vec3(0.0f, 0.2f, 0.0f)},
          {0.6f, vec3(0.0f, 0.2f, 0.0f)},
          {0.8f, vec3(0.5f,-0.2f, 0.2f)},
          {0.9f, vec3(0.4f,-0.2f, 0.2f)},
          {1.1f, vec3(0.4f,-0.2f, 0.2f)}
        };
        auto curveLegL = std::make_shared<AnimationCurve>();
        curveLegL->AddKey(std::begin(kfLegL), std::end(kfLegL));

        clipOrcMagic->AddCurve(0, ANIMATION_TARGET::ROTATION, curveBody);
        clipOrcMagic->AddCurve(0, ANIMATION_TARGET::POSITION, curveBodyPos);
        clipOrcMagic->AddCurve(1, ANIMATION_TARGET::ROTATION, curveBodyInv);
        clipOrcMagic->AddCurve(2, ANIMATION_TARGET::ROTATION, curveArmR);
        clipOrcMagic->AddCurve(3, ANIMATION_TARGET::ROTATION, curveArmL);
        clipOrcMagic->AddCurve(4, ANIMATION_TARGET::ROTATION, curveBodyInv);
        clipOrcMagic->AddCurve(5, ANIMATION_TARGET::ROTATION, curveLegL);
    }

    // オークのやられモーションA
    auto clipOrcDown0 = std::make_shared<AnimationClip>();
    {
        static const ANIMATION_KEYFRAME kfBodyPos[] =
        {
          {0.0f, vec3(0,-0.0f, 0.0f)},
          {0.3f, vec3(0,-0.2f, 0.4f)},
          {0.5f, vec3(0,-0.4f, 0.4f)},
          {0.7f, vec3(0,-0.7f, 0.7f)},
          {0.8f, vec3(0,-0.6f, 0.7f)},
          {0.9f, vec3(0,-0.7f, 0.7f)}
        };
        auto curveBodyPos = std::make_shared<AnimationCurve>();
        curveBodyPos->AddKey(std::begin(kfBodyPos), std::end(kfBodyPos));

        static const ANIMATION_KEYFRAME kfBodyRot[] = {
          {0.0f, vec3(0.0f, 0.0f, 0)},
          {0.3f, vec3(-0.1f, 0.0f, 0)},
          {0.5f, vec3(0.5f, 0.0f, 0)},
          {0.7f, vec3(1.4f, 0.0f, 0)},
          {0.8f, vec3(1.2f, 0.0f, 0)},
          {0.9f, vec3(1.4f, 0.0f, 0)} };
        auto curveBodyRot = std::make_shared<AnimationCurve>();
        curveBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        static const ANIMATION_KEYFRAME kfHead[] =
        {
          {0.0f, vec3(0)},
          {0.3f, vec3(-0.3f, 0.0f, 0)},
          {0.5f, vec3(-1.0f, 0.0f, 0)},
          {0.7f, vec3(-1.4f, 0.0f, 0)},
          {0.8f, vec3(-1.0f, 0.0f, 0)},
          {0.9f, vec3(-1.4f, 0.0f, 0)}
        };
        auto curveHead = std::make_shared<AnimationCurve>();
        curveHead->AddKey(std::begin(kfHead), std::end(kfHead));

        static const ANIMATION_KEYFRAME kfArmR[] =
        {
          {0.0f, vec3( 0.0f,-0.0f, 1.3f)},
          {0.3f, vec3(-0.3f,-0.7f, 1.3f)},
          {0.7f, vec3(-0.8f,-1.2f, 1.3f)},
          {0.8f, vec3(-0.8f,-1.2f, 1.3f)}
        };
        auto curveArmR = std::make_shared<AnimationCurve>();
        curveArmR->AddKey(std::begin(kfArmR), std::end(kfArmR));

        static const ANIMATION_KEYFRAME kfArmL[] =
        {
          {0.0f, vec3( 0.0f, 0.0f,-1.3f)},
          {0.3f, vec3(-0.3f, 0.7f,-1.3f)},
          {0.7f, vec3(-0.5f, 1.2f,-1.3f)},
          {0.8f, vec3(-0.5f, 1.2f,-1.3f)}
        };
        auto curveArmL = std::make_shared<AnimationCurve>();
        curveArmL->AddKey(std::begin(kfArmL), std::end(kfArmL));

        static const ANIMATION_KEYFRAME kfLeg[] =
        {
          {0.0f, vec3(0)},
          {0.3f, vec3(0.8f, 0.0f, 0)},
          {0.7f, vec3(0.1f, 0.0f, 0)},
          {0.8f, vec3(0.2f, 0.0f, 0)},
          {0.9f, vec3(0.1f, 0.0f, 0)}
        };
        auto curveLeg = std::make_shared<AnimationCurve>();
        curveLeg->AddKey(std::begin(kfLeg), std::end(kfLeg));

        clipOrcDown0->AddCurve(0, ANIMATION_TARGET::POSITION, curveBodyPos);
        clipOrcDown0->AddCurve(0, ANIMATION_TARGET::ROTATION, curveBodyRot);
        clipOrcDown0->AddCurve(1, ANIMATION_TARGET::ROTATION, curveHead);
        clipOrcDown0->AddCurve(2, ANIMATION_TARGET::ROTATION, curveArmR);
        clipOrcDown0->AddCurve(3, ANIMATION_TARGET::ROTATION, curveArmL);
        clipOrcDown0->AddCurve(4, ANIMATION_TARGET::ROTATION, curveLeg);
        clipOrcDown0->AddCurve(5, ANIMATION_TARGET::ROTATION, curveLeg);
    }

    // オークのやられモーションB
    auto clipOrcDown1 = std::make_shared<AnimationClip>();
    {
        static const ANIMATION_KEYFRAME kfBodyPos[] =
        {
          {0.0f, vec3(0,-0.0f, -0.0f)},
          {0.3f, vec3(0,-0.02f,-0.05f)},
          {0.5f, vec3(0,-0.2f, -0.5f)},
          {0.7f, vec3(0,-0.7f, -0.7f)},
          {0.8f, vec3(0,-0.6f, -0.7f)},
          {0.9f, vec3(0,-0.7f, -0.7f)}
        };
        auto curveBodyPos = std::make_shared<AnimationCurve>();
        curveBodyPos->AddKey(std::begin(kfBodyPos), std::end(kfBodyPos));

        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.0f, vec3(0)},
          {0.3f, vec3(-0.6f, 0, 0)},
          {0.5f, vec3(-0.7f, 0, 0)},
          {0.7f, vec3(-1.5f, 0, 0)},
          {0.8f, vec3(-1.4f, 0, 0)},
          {0.9f, vec3(-1.5f, 0, 0)}
        };
        auto curveBodyRot = std::make_shared<AnimationCurve>();
        curveBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        static const ANIMATION_KEYFRAME kfHead[] = 
        {
          {0.0f, vec3(0)},
          {0.3f, vec3( 0.3f,    0, 0)},
          {0.5f, vec3( 0.5f,    0, 0)},
          {0.7f, vec3( 0.3f,    0, 0)},
          {0.8f, vec3(-0.5f, 1.0f, 0)},
          {0.9f, vec3(-0.3f, 0.8f, 0)}
        };
        auto curveHead = std::make_shared<AnimationCurve>();
        curveHead->AddKey(std::begin(kfHead), std::end(kfHead));

        static const ANIMATION_KEYFRAME kfArmR[] =
        {
          {0.0f, vec3(0.0f, 0.0f, 1.2f)},
          {0.3f, vec3(0.0f, 0.5f, 0.5f)},
          {0.5f, vec3(0.3f, 0.8f, 0.6f)},
          {0.7f, vec3(0.5f, 1.0f, 0.8f)},
          {0.8f, vec3(0.9f, 1.1f, 1.2f)}
        };
        auto curveArmR = std::make_shared<AnimationCurve>();
        curveArmR->AddKey(std::begin(kfArmR), std::end(kfArmR));

        static const ANIMATION_KEYFRAME kfArmL[] =
        {
          {0.0f, vec3( 0.0f, 0.0f,-1.2f)},
          {0.3f, vec3(-0.5f, 0.0f,-1.3f)},
          {0.5f, vec3(-1.0f,-0.0f,-1.4f)},
          {0.7f, vec3(-0.5f, 0.0f,-1.2f)},
          {0.8f, vec3(-0.0f, 0.0f,-1.2f)}
        };
        auto curveArmL = std::make_shared<AnimationCurve>();
        curveArmL->AddKey(std::begin(kfArmL), std::end(kfArmL));

        static const ANIMATION_KEYFRAME kfLegR[] =
        {
          {0.0f, vec3(0)},
          {0.3f, vec3(-0.1f, 0.0f, 0)},
          {0.5f, vec3(-0.7f, 0.0f, 0)},
          {0.7f, vec3(0)},
          {0.8f, vec3(-0.1f, 0.0f, 0)},
          {0.9f, vec3(0)}
        };
        auto curveLegR = std::make_shared<AnimationCurve>();
        curveLegR->AddKey(std::begin(kfLegR), std::end(kfLegR));
        static const ANIMATION_KEYFRAME kfLegL[] =
        {
          {0.0f, vec3(0)},
          {0.3f, vec3( 0.6f, 0, 0)},
          {0.5f, vec3( 0.1f, 0, 0)},
          {0.7f, vec3(0)},
          {0.8f, vec3(-0.1f, 0, 0)},
          {0.9f, vec3(0)}
        };
        auto curveLegL = std::make_shared<AnimationCurve>();
        curveLegL->AddKey(std::begin(kfLegL), std::end(kfLegL));

        clipOrcDown1->AddCurve(0, ANIMATION_TARGET::POSITION, curveBodyPos);
        clipOrcDown1->AddCurve(0, ANIMATION_TARGET::ROTATION, curveBodyRot);
        clipOrcDown1->AddCurve(1, ANIMATION_TARGET::ROTATION, curveHead);
        clipOrcDown1->AddCurve(2, ANIMATION_TARGET::ROTATION, curveArmR);
        clipOrcDown1->AddCurve(3, ANIMATION_TARGET::ROTATION, curveArmL);
        clipOrcDown1->AddCurve(4, ANIMATION_TARGET::ROTATION, curveLegR);
        clipOrcDown1->AddCurve(5, ANIMATION_TARGET::ROTATION, curveLegL);
    }
#pragma endregion

    GameObjectPtr enemyTrigger;
    GameObjectPtr bossTrigger;
    GameObjectPtr enter;
    GameObjectPtr exit;

    for (auto& go : list)
    {
        // Dragonを配置する
        if (strcmp("Dragon", go->name.c_str()) == 0)
        {
            // メッシュを読み込む
            go->renderer = std::make_shared<SkeletalMeshRenderer>();
            go->renderer->SetMesh(
                engine.GetSkeletalMesh("Res/MeshData/free_rocks/Dragon.obj"));

            // アニメーションを設定する
            auto a = go->AddComponent<Animator>();
            a->AddClip("wait", clipDragonWait);
            a->AddClip("walk", clipDragonWalk);
            a->AddClip("attack", clipDragonAttack);
            a->AddClip("attack_tail", clipDragonAttackTail);
            a->AddClip("magic", clipDragonMagic);
            a->AddClip("hovering", clipDragonHovering);
            a->AddClip("dead", clipDragonDead);
            a->Play("wait");

            // ドラゴンのコンポーネントを設定する
            auto enemy = go->AddComponent<Dragon>();
            enemy->SetAnimator(a);
            enemy->SetTarget(player);
            go->AddComponent<GoalWay>();

            // 霧コンポーネントを設定する
            auto mist_generator = engine.Create<GameObject>("mist generator", { 0, 1, 80 });
            auto mist_generatorComponent = mist_generator->AddComponent<MistGenerator>();
            mist_generatorComponent->range = { 15,15 };
            enemy->mistGenerator = mist_generatorComponent;

            boss = enemy;
        }
        // OrcShamanFighterを配置する
        if (strcmp("OrcShamanFighter", go->name.c_str()) == 0)
        {
            // メッシュを読み込む
            go->renderer = std::make_shared<SkeletalMeshRenderer>();
            go->renderer->SetMesh(
                engine.GetSkeletalMesh("Res/MeshData/orcshaman/orcshaman_fighter.obj"));

            // アニメーションを設定する
            auto a = go->AddComponent<Animator>();
            a->AddClip("wait", clipOrcWait);
            a->AddClip("walk", clipOrcWalk);
            a->AddClip("attack", clipOrcAttack);
            a->AddClip("attack.right", clipOrcAttackRight);
            a->AddClip("down.0", clipOrcDown0);
            a->AddClip("down.1", clipOrcDown1);
            a->Play("wait");

            // オークのコンポーネントを設定する
            auto enemy = go->AddComponent<Orc>();
            enemy->SetAnimator(a);
            enemy->SetTarget(player);
            enemy->SetAttackDistance(3);
            enemy->SetJob(Orc::JOB::FIGHTER);
        }
        // OrcShamanMagicianを配置する
        if (strcmp("OrcShamanMagician", go->name.c_str()) == 0)
        {
            // メッシュを読み込む
            go->renderer = std::make_shared<SkeletalMeshRenderer>();
            go->renderer->SetMesh(
                engine.GetSkeletalMesh("Res/MeshData/orcshaman/orcshaman_magician.obj"));

            // アニメーションを設定する
            auto a = go->AddComponent<Animator>();
            a->AddClip("wait", clipOrcWait);
            a->AddClip("walk", clipOrcWalk);
            a->AddClip("magic", clipOrcMagic);
            a->AddClip("down.0", clipOrcDown0);
            a->AddClip("down.1", clipOrcDown1);
            a->Play("wait");

            // オークのコンポーネントを設定する
            auto enemy = go->AddComponent<Orc>();
            enemy->SetAnimator(a);
            enemy->SetTarget(player);
            enemy->SetAttackDistance(5);
            enemy->SetJob(Orc::JOB::MAGICIAN);
        }
        // ゴール判定オブジェクトを配置する
        else if (strcmp("Chest", go->name.c_str()) == 0)
            go->AddComponent<GoalEvent>();
        // 雑魚部屋侵入検知器を配置する
        else if (go->name == "EnemyTrigger")
            enemyTrigger = go;
        // ボス部屋侵入検知器を配置する
        else if (go->name == "BossTrigger")
            bossTrigger = go;
        // ボス部屋の入り口を封鎖するための岩を配置する
        else if (go->name == "Enter")
            enter = go;
        // ボス部屋の出口を封鎖するための岩を配置する
        else if (go->name == "Exit")
            exit = go;
    }

    if (enemyTrigger)
    {
        // 雑魚敵エリアに入った判定するトリガーを設定する
        auto box = enemyTrigger->GetComponent<BoxCollider>();
        if (box)
            box->isTrigger = true;
        auto a = enemyTrigger->AddComponent<EnemyTrigger>();
        a->guide_control = player->AddComponent<ControlGuide>();
    }
    if (bossTrigger && enter)
    {
        // ボスエリアに入った判定するトリガーを設定する
        auto box = bossTrigger->GetComponent<BoxCollider>();
        if (box)
            box->isTrigger = true;
        auto a = bossTrigger->AddComponent<BossTrigger>();
        a->enter = enter;
        a->guide_goal = player->AddComponent<GoalGuide>();
        a->boss = boss;
    }
    if (exit)
    {
        // 出口とボスを連携させる
        boss->GetOwner()->GetComponent<GoalWay>()->exit = exit;
        boss->GetOwner()->GetComponent<GoalWay>()->guide_goal = player->GetComponent<GoalGuide>();
    }

    return true;
}

/// <summary>
/// 常にゲームオーバーになってないか監視する
/// </summary>
/// <param name="engine">ゲームエンジン</param>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void ExplorationScene::Update
(
    Engine& engine,
    float deltaTime
)
{
    // プレイヤーが死んでいたら、ゲームオーバー画面を表示して、状態をgameOverに変更
    if (playerComponent->GetStatePlayer() == PlayerComponent::STATE_PLAYER::DEAD)
    {
        // ゲームオーバー画像を表示
        engine.CreateUIObject<UILayout>("Res/game_over.dds", { 0,0 }, 0.25f);

        // タイトル画面に戻るボタンを表示
        auto button = engine.CreateUIObject<UIButton>("Res/return_button.dds", { 0,-0.5f }, 0.1f);
        button.second->onClick.push_back
        (
            [](UIButton* button)
            {
                Engine* engine = button->GetOwner()->GetEngine();
                engine->SetNextScene<TitleScene>();
            }
        );
    }
}

/// <summary>
/// シーンを終了する
/// </summary>
/// <param name="engine">ゲームエンジン</param>
void ExplorationScene::Finalize(Engine& engine)
{
    EasyAudio::PlayOneShot(SE::click);
    engine.ShowMouseCursor();
    engine.ClearGameObjectAll();
}