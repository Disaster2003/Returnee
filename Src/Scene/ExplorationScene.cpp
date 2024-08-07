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
/// �J�������v���C���[�ɒǏ]������R���|�[�l���g
/// </summary>
class FirstPersonCamera
    : public Component
{
public:
    // �f�t�H���g�R���X�g���N�^
    inline FirstPersonCamera() = default;
    // �f�t�H���g�f�X�g���N�^
    inline virtual ~FirstPersonCamera() = default;

    /// <summary>
    /// �J�������v���C���[�ɒǏ]������
    /// </summary>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
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
/// ����K�C�h
/// </summary>
class ControlGuide
    : public Component
{
public:
    /// <summary>
    /// ��ɑ���K�C�h�̏�Ԃ��X�V����
    /// </summary>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        auto player = owner->GetComponent<PlayerComponent>();

        // �v���C���[���Q�[���v���C���łȂ����
        if (player->GetStatePlayer() != PlayerComponent::STATE_PLAYER::ALIVE)
            // ���g��j������
            ui_guide_action->GetOwner()->Destroy();
        // �{�X�����ɓ���������
        else if (isArrivedBossRoom)
        {
            // ����K�C�h��ύX���Ă�����
            if (ui_guide_action)
            {
                // �ύX��3.0�b�͓_�ł�����
                GuideFlashing(ui_guide_action, deltaTime);

                // �������Ȃ�
                return;
            }
            // ��{����K�C�h��j������
            ui_guide_control->GetOwner()->Destroy();
            // �퓬����K�C�h����������Ă��Ȃ�������
            if (!ui_guide_action)
                // �퓬����K�C�h�𐶐�����
                ui_guide_action = engine->CreateUIObject<UILayout>("Res/guide_action.dds", { -1.40f, 0.00f }, 0.60f).second;
        }
        // ��{����K�C�h����������Ă��Ȃ�������
        else if (!ui_guide_control)
            // ��{����K�C�h�𐶐�����
            ui_guide_control = engine->CreateUIObject<UILayout>("Res/guide_base.dds", { -1.40f, 0.00f }, 0.60f).second;
    }

    // �{�X�����ɓ����������Ƃ�ݒ肷��
    inline void SetIsArrivedBossRoom() { isArrivedBossRoom = true; }

    /// <summary>
    /// �K�C�h�̋^���_��
    /// </summary>
    /// <param name="uiGuide">�K�C�h</param>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
    void GuideFlashing
    (
        std::shared_ptr<UILayout> uiGuide,
        float deltaTime
    )
    {
        // 3��_�ł�����I��
        if (timer >= 3.0f)
            return;

        // �����_�ȉ���0.5�b�ȏ�̏ꍇ
        if (fmod(timer, 1.0f) > 0.5f)
            // �\������
            uiGuide->position_base.y = 0;
        else
            // ��\���ɂ���
            uiGuide->position_base.y = 10;

        // �K�C�h�̕ύX������A���Ԍv��
        timer += deltaTime;
    }

private:
    std::shared_ptr<UILayout> ui_guide_control; // ��{����K�C�h
    std::shared_ptr<UILayout> ui_guide_action;  // �퓬����K�C�h

    float timer = 0; // �K�C�h�̓_�Ń^�C�}�[

    bool isArrivedBossRoom = false; // �{�X������true = ���B�Afalse = ����
};

/// <summary>
/// �ڕW�K�C�h
/// </summary>
class GoalGuide
    : public Component
{
public:
    /// <summary>
    /// ��ɖڕW�K�C�h�̏�Ԃ��X�V����
    /// </summary>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        // �v���C���[�R���|�[�l���g���擾����
        auto player = owner->GetComponent<PlayerComponent>();

        // �v���C���[���Q�[���v���C���łȂ����
        if (player->GetStatePlayer() != PlayerComponent::STATE_PLAYER::ALIVE)
        {
            
            if (state_guide == 2)
                // �S�[���ɐG�ꂽ��A���g��j������
                ui_guide_get_treasure->GetOwner()->Destroy();
            else if(state_guide == 1)
                // �Q�[���I�[�o�[�Ȃ�A���g��j������
                ui_guide_boss_battle->GetOwner()->Destroy();
            else if(state_guide == 0)
                // �Q�[���I�[�o�[�Ȃ�A���g��j������
                ui_guide_player_forward->GetOwner()->Destroy();
        }
        // �{�X��|������
        else if (state_guide == 2)
        {
            // �ڕW�K�C�h��ύX���Ă�����
            if (ui_guide_get_treasure)
            {
                // �ύX��3.0�b�͓_�ł�����
                GuideFlashing(ui_guide_get_treasure, deltaTime);

                // �������Ȃ�
                return;
            }
            // �{�X�����ڕW�K�C�h��j������
            ui_guide_boss_battle->GetOwner()->Destroy();
            // �S�[���ԋߖڕW�K�C�h����������Ă��Ȃ�������
            if (!ui_guide_get_treasure)
                // �S�[���ԋߖڕW�K�C�h�𐶐�����
                ui_guide_get_treasure = engine->CreateUIObject<UILayout>("Res/goal_get_treasure.dds", { 1.10f, 0.80f }, 0.20f).second;
        }
        // �{�X�����ɓ���������
        else if (state_guide == 1)
        {
            // �ڕW�K�C�h��ύX���Ă�����
            if (ui_guide_boss_battle)
            {
                // �ύX��3.0�b�͓_�ł�����
                GuideFlashing(ui_guide_boss_battle, deltaTime);

                // �������Ȃ�
                return;
            }
            // �����ڕW�K�C�h��j������
            ui_guide_player_forward->GetOwner()->Destroy();
            // �{�X�����ڕW�K�C�h����������Ă��Ȃ�������
            if (!ui_guide_boss_battle)
                // �{�X�����ڕW�K�C�h�𐶐�����
                ui_guide_boss_battle = engine->CreateUIObject<UILayout>("Res/goal_kill_dragon.dds", { 1.10f, 0.80f }, 0.20f).second;
        }
        // �Q�[�����n�܂�����
        else if (!ui_guide_player_forward)
                // �����ڕW�K�C�h�𐶐�����
                ui_guide_player_forward = engine->CreateUIObject<UILayout>("Res/goal_go_straight.dds", { 1.10f, 0.80f }, 0.20f).second;
    }

    // ���̒i�K�֐ݒ肷��
    inline void SetGuideState() { state_guide = static_cast<STATE_GUIDE>(static_cast<int>(state_guide) + 1); }

    /// <summary>
    /// �K�C�h�̋^���_��
    /// </summary>
    /// <param name="uiGuide">�K�C�h</param>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
    void GuideFlashing
    (
        std::shared_ptr<UILayout> uiGuide,
        float deltaTime
    )
    {
        // 3��_�ł�����I��
        if (timer >= 3.0f)
            return;

        // �����_�ȉ���0.5�b�ȏ�̏ꍇ
        if (fmod(timer, 1.0f) > 0.5f)
            // �\������
            uiGuide->position_base.y = 0.80f;
        else
            // ��\���ɂ���
            uiGuide->position_base.y = 10;

        // �K�C�h�̕ύX������A���Ԍv��
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

    std::shared_ptr<UILayout> ui_guide_player_forward; // �����ڕW�K�C�h
    std::shared_ptr<UILayout> ui_guide_boss_battle;    // �{�X�����ڕW�K�C�h
    std::shared_ptr<UILayout> ui_guide_get_treasure;   // �S�[���ԋߖڕW�K�C�h

    float timer = 0; // �K�C�h�̓_�Ń^�C�}�[
};

/// <summary>
/// �{�X�N���p�g���K�[
/// </summary>
class BossTrigger
    : public Component
{
public:
    /// <summary>
    /// ��Ƀv���C���[���N���������ǂ����Ď����A������ɂ����̏�Ԃ��X�V����
    /// </summary>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
    virtual void Update(float deltaTime) override
    {
        if (!doClosing || isClosed || !enter)
            return;
        
        // ��΂��グ��
        enter->position.y += 8 * deltaTime;
        float vol = EasyAudio::GetVolume(AudioPlayer::bgm);
        EasyAudio::SetVolume(AudioPlayer::bgm, std::max(vol - deltaTime, 0.0f));
        if (enter->position.y >= 0.5f)
        {
            /* ���� */
            // BossBGM���Đ�����
            EasyAudio::Stop(AudioPlayer::bgm);
            EasyAudio::Play(AudioPlayer::bgm, BGM::boss, 1, true);
            enter->position.y = 0.5f;
            isClosed = true;
            guide_goal->SetGuideState();
            GetOwner()->Destroy();
        }
    }

    /// <summary>
    /// �Փˎ��ɏՓ˂�������ɂ���ď������s��
    /// </summary>
    /// <param name="self">���g</param>
    /// <param name="other">�Փ˂�������</param>
    virtual void OnCollision
    (
        const ComponentPtr& self,
        const ComponentPtr& other
    ) override
    {
        // �₪�����ĂȂ���Ԃ��A�v���C���[�ɐG�ꂽ��
        if (!doClosing && other->GetOwner()->name == "player")
        {
            // ��̋N��
            EasyAudio::PlayOneShot(SE::rock_close);
            doClosing = true;
            if (boss && boss->GetOwner())
                boss->SetSearchDistance(30);
        }
    }

    GameObjectPtr enter;                   // ������ɖ��܂��Ă��
    std::shared_ptr<GoalGuide> guide_goal; // �ڕW�K�C�h
    std::shared_ptr<Dragon> boss;          // �{�X

private:
    bool doClosing = false; // ���true = �������Afalse = �������Ȃ�
    bool isClosed = false;  // ��œ������true = �����Afalse = �ʍs
};

/// <summary>
/// �G���G�N���p�g���K�[
/// </summary>
class EnemyTrigger
    : public Component
{
public:
    /// <summary>
    /// �Փˎ��ɏՓ˂�������ɂ���ď������s��
    /// </summary>
    /// <param name="self">���g</param>
    /// <param name="other">�Փ˂�������</param>
    virtual void OnCollision
    (
        const ComponentPtr& self,
        const ComponentPtr& other
    ) override
    {
        // �₪�����ĂȂ���Ԃ��A�v���C���[�ɐG�ꂽ��
        if (other->GetOwner()->name == "player")
        {
            guide_control->SetIsArrivedBossRoom();
            GetOwner()->Destroy();
        }
    }

    std::shared_ptr<ControlGuide> guide_control; // ����K�C�h
};

/// <summary>
/// �{�X��|������A�S�[���O�̏�Q���𓮂���
/// </summary>
class GoalWay
    : public Component
{
public:
    /// <summary>
    /// �{�X�����񂾂�A���ŃS�[���O�̊���ǂ���
    /// </summary>
    virtual void OnDestroy() override
    {
        while (1)
        {
            // ��΂����낷
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
/// �V�[��������������
/// </summary>
/// <param name="engine">�Q�[���G���W��</param>
/// <returns>true : �����������Afalse : ���������s</returns>
bool ExplorationScene::Initialize(Engine& engine)
{
    // PlayBGM���Đ�����
    EasyAudio::Stop(AudioPlayer::bgm);
    EasyAudio::Play(AudioPlayer::bgm, BGM::play, 1, true);

    // �Q�[���I�u�W�F�N�g�z�u�t�@�C����ǂݍ���
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

    // �v���C���[��z�u����
    auto player = engine.Create<GameObject>("player", { 0, 1.5f, 0 });
    playerComponent = player->AddComponent<PlayerComponent>();
    player->AddComponent<FirstPersonCamera>();
    player->rotation.y = radians(180);

    // �X�J�C�X�t�B�A��ݒ肷��
    material_skysphere = std::make_shared<MATERIAL>();
    material_skysphere->texBaseColor = engine.GetTexture("Res/MeshData/sky_sphere/sky.dds");

    // ���L���[�u�}�b�v��ݒ肷��
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
    // ��{���V������
    static const ANIMATION_KEYFRAME kfBody[] =
    {
      {0.00f, vec3(0, 1.00f, 0)}, // �����ʒu
      {1.00f, vec3(0, 0.50f, 0)}, // ���Ɉړ�
      {2.00f, vec3(0, 1.00f, 0)}, // ��Ɉړ�(�����ʒu�ɖ߂�)
    };

    // �h���S���̑ҋ@
    auto clipDragonWait = std::make_shared<AnimationClip>();
    {
        auto curveDragonWaitBody = std::make_shared<AnimationCurve>();
        curveDragonWaitBody->AddKey(std::begin(kfBody), std::end(kfBody));
        clipDragonWait->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonWaitBody);
        clipDragonWait->SetLoopFlag(true); // ���[�v����
    }
    // �h���S���̕��s
    auto clipDragonWalk = std::make_shared<AnimationClip>();
    {
        auto curveDragonWalkBodyPos = std::make_shared<AnimationCurve>();
        curveDragonWalkBodyPos->AddKey(std::begin(kfBody), std::end(kfBody));

        auto curveDragonWalkBodyRot = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.00f, vec3(0.30f, 0, 0)}, // �����X��
        };
        curveDragonWalkBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        clipDragonWalk->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonWalkBodyPos);
        clipDragonWalk->AddCurve(0, ANIMATION_TARGET::ROTATION, curveDragonWalkBodyRot);
        clipDragonWalk->SetLoopFlag(true); // ���[�v����
    }
    // �h���S���̍U��
    auto clipDragonAttack = std::make_shared<AnimationClip>();
    {
        auto curveDragonAttackBodyPos = std::make_shared<AnimationCurve>();
        curveDragonAttackBodyPos->AddKey(std::begin(kfBody), std::end(kfBody));

        auto curveDragonAttackBodyRot = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.00f, vec3(0.30f, 0,     0)}, // �����X��
          {0.50f, vec3(0.50f, 1.00f, 0)}, // �����X���A����]
          {1.00f, vec3(0.30f, 0,     0)}, // �����X��
        };
        curveDragonAttackBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        clipDragonAttack->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonAttackBodyPos);
        clipDragonAttack->AddCurve(0, ANIMATION_TARGET::ROTATION, curveDragonAttackBodyRot);
        clipDragonAttack->SetLoopFlag(false); // ���[�v���Ȃ�
    }
    // �h���S���̐K���U��
    auto clipDragonAttackTail = std::make_shared<AnimationClip>();
    {
        static const ANIMATION_KEYFRAME kfBodyPos[] =
        {
          {0.00f, vec3(0, 1.00f, 0)}, // �����ʒu
          {0.40f, vec3(0)},           // ���Ɉړ�
          {1.50f, vec3(0)},           // ���Ɉړ�
          {2.00f, vec3(0, 1.00f, 0)}, // ��Ɉړ�(�����ʒu�ɖ߂�)
        };
        auto curveDragonAttackTailBodyPos = std::make_shared<AnimationCurve>();
        curveDragonAttackTailBodyPos->AddKey(std::begin(kfBodyPos), std::end(kfBodyPos));

        auto curveDragonAttackTailBodyRot = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.00f, vec3(0.30f, 0,     0)}, // �����X��
          {0.50f, vec3(0.30f, 4.00f, 0)}, // ����U���
          {1.30f, vec3(0.30f, 0,     0)}, // �����X��
        };
        curveDragonAttackTailBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        clipDragonAttackTail->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonAttackTailBodyPos);
        clipDragonAttackTail->AddCurve(0, ANIMATION_TARGET::ROTATION, curveDragonAttackTailBodyRot);
        clipDragonAttackTail->SetLoopFlag(false); // ���[�v���Ȃ�
    }
    // �h���S���̉΋�
    auto clipDragonMagic = std::make_shared<AnimationClip>();
    {
        auto curveDragonMagicBodyPos = std::make_shared<AnimationCurve>();
        curveDragonMagicBodyPos->AddKey(std::begin(kfBody), std::end(kfBody));

        auto curveDragonMagicBodyRot = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.00f, vec3(-0.15f, 0, 0)}, // ���ɂ̂�����
          {1.00f, vec3( 0.30f, 0, 0)}, // �O�ɌX��
        };
        curveDragonMagicBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        clipDragonMagic->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonMagicBodyPos);
        clipDragonMagic->AddCurve(0, ANIMATION_TARGET::ROTATION, curveDragonMagicBodyRot);
        clipDragonMagic->SetLoopFlag(false); // ���[�v���Ȃ�
    }
    // �h���S���̋󒆔�s
    auto clipDragonHovering = std::make_shared<AnimationClip>();
    {
        auto curveDragonHoveringBodyPos = std::make_shared<AnimationCurve>();
        curveDragonHoveringBodyPos->AddKey(std::begin(kfBody), std::end(kfBody));

        auto curveDragonHoveringBodyRot = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.00f, vec3(1.0f, 0, 0)}, // �����X��
        };
        curveDragonHoveringBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        clipDragonHovering->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonHoveringBodyPos);
        clipDragonHovering->AddCurve(0, ANIMATION_TARGET::ROTATION, curveDragonHoveringBodyRot);
        clipDragonHovering->SetLoopFlag(true); // ���[�v����
    }
    // �h���S���̎�
    auto clipDragonDead = std::make_shared<AnimationClip>();
    {
        auto curveDragonDeadBodyPos = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyPos[] =
        {
          {0.00f, vec3(0, 1.00f, 0)}, // �i�X�Əォ��
          {2.00f, vec3(0)},           // ���ɍ~��Ă���
        };
        curveDragonDeadBodyPos->AddKey(std::begin(kfBodyPos), std::end(kfBodyPos));

        auto curveDragonDeadBodyRot = std::make_shared<AnimationCurve>();
        static const ANIMATION_KEYFRAME kfBodyRot[] =
        {
          {0.00f, vec3(0.30f, 0, 0)}, // �i�X�ƑO��
          {1.00f, vec3(1.00f, 0, 0)}, // �X��
        };
        curveDragonDeadBodyRot->AddKey(std::begin(kfBodyRot), std::end(kfBodyRot));

        clipDragonDead->AddCurve(0, ANIMATION_TARGET::POSITION, curveDragonDeadBodyPos);
        clipDragonDead->AddCurve(0, ANIMATION_TARGET::ROTATION, curveDragonDeadBodyRot);
        clipDragonDead->SetLoopFlag(false); // ���[�v���Ȃ�
    }
#pragma endregion

#pragma region ORCSHAMAN_BONEANIMATION
    // �I�[�N�̑ҋ@
    auto clipOrcWait = std::make_shared<AnimationClip>();
    {
        /* �^���I�Ɍċz������ */
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
        clipOrcWait->SetLoopFlag(true); // ���[�v����
    }

    // �I�[�N�̕��s
    auto clipOrcWalk = std::make_shared<AnimationClip>();
    {
        // ���̂��㉺�Ɉړ�����J�[�u
        static const ANIMATION_KEYFRAME kfBody[] =
        {
          {0.00f, vec3(0)},            // �����ʒu�ɔz�u����
          {0.50f, vec3(0, -0.10f, 0)}, // ���Ɉړ�
          {1.00f, vec3(0)},            // ��Ɉړ�(�����ʒu�ɖ߂�)
          {1.50f, vec3(0, -0.10f, 0)}, // ���Ɉړ�
          {2.00f, vec3(0)}             // ��Ɉړ�(�����ʒu�ɖ߂�)
        };
        auto curveBody = std::make_shared<AnimationCurve>();
        curveBody->AddKey(std::begin(kfBody), std::end(kfBody));

        // �E�r�����낵�đO��ɉ�]������J�[�u
        static const ANIMATION_KEYFRAME kfArmR[] =
        {
          {0.0f, vec3( 0   , 0, 1.17f)}, // �����ʒu�ɔz�u����
          {0.5f, vec3( 0.5f, 0, 1.17f)}, // ���ɐU��
          {1.5f, vec3(-0.5f, 0, 1.17f)}, // �O�ɍ~��
          {2.0f, vec3( 0   , 0, 1.17f)}  // �����ʒu�ɖ߂�
        };
        auto curveArmR = std::make_shared<AnimationCurve>();
        curveArmR->AddKey(std::begin(kfArmR), std::end(kfArmR));

        // ���r�����낵�đO��ɉ�]������J�[�u
        static const ANIMATION_KEYFRAME kfArmL[] =
        {
          {0.0f, vec3( 0   , 0, -1.17f)}, // �����ʒu�ɔz�u����
          {0.5f, vec3(-0.5f, 0, -1.17f)}, // �O�ɍ~��
          {1.5f, vec3( 0.5f, 0, -1.17f)}, // ���ɐU��
          {2.0f, vec3( 0   , 0, -1.17f)}  // �����ʒu�ɖ߂�
        };
        auto curveArmL = std::make_shared<AnimationCurve>();
        curveArmL->AddKey(std::begin(kfArmL), std::end(kfArmL));

        // �E����O��ɉ�]������J�[�u
        static const ANIMATION_KEYFRAME kfLegR[] =
        {
          {0.0f, vec3(0)},           // �����ʒu�ɔz�u����
          {0.5f, vec3(-0.5f, 0, 0)}, // �O�ɏo��
          {1.5f, vec3( 0.5f, 0, 0)}, // ���ɏo��
          {2.0f, vec3(0)}            // �����ʒu�ɖ߂�
        };
        auto curveLegR = std::make_shared<AnimationCurve>();
        curveLegR->AddKey(std::begin(kfLegR), std::end(kfLegR));

        // ������O��ɉ�]������J�[�u
        static const ANIMATION_KEYFRAME kfLegL[] =
        {
          {0.0f, vec3(0)},           // �����ʒu�ɔz�u����
          {0.5f, vec3( 0.5f, 0, 0)}, // ���ɏo��
          {1.5f, vec3(-0.5f, 0, 0)}, // �O�ɏo��
          {2.0f, vec3(0)}            // �����ʒu�ɖ߂�
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

    // �I�[�N�̕����U��(��)
    auto clipOrcAttackRight = std::make_shared<AnimationClip>();
    {
        /* ���ɐU��� */
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

    // �I�[�N�V���[�}���̍U��(�c)
    auto clipOrcAttack = std::make_shared<AnimationClip>();
    {
        /* �c�ɐU��� */
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

    // �I�[�N�V���[�}���̖��@�U��
    auto clipOrcMagic = std::make_shared<AnimationClip>();
    {
        /* ���O�ɂ����� */
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

    // �I�[�N�̂��ꃂ�[�V����A
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

    // �I�[�N�̂��ꃂ�[�V����B
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
        // Dragon��z�u����
        if (strcmp("Dragon", go->name.c_str()) == 0)
        {
            // ���b�V����ǂݍ���
            go->renderer = std::make_shared<SkeletalMeshRenderer>();
            go->renderer->SetMesh(
                engine.GetSkeletalMesh("Res/MeshData/free_rocks/Dragon.obj"));

            // �A�j���[�V������ݒ肷��
            auto a = go->AddComponent<Animator>();
            a->AddClip("wait", clipDragonWait);
            a->AddClip("walk", clipDragonWalk);
            a->AddClip("attack", clipDragonAttack);
            a->AddClip("attack_tail", clipDragonAttackTail);
            a->AddClip("magic", clipDragonMagic);
            a->AddClip("hovering", clipDragonHovering);
            a->AddClip("dead", clipDragonDead);
            a->Play("wait");

            // �h���S���̃R���|�[�l���g��ݒ肷��
            auto enemy = go->AddComponent<Dragon>();
            enemy->SetAnimator(a);
            enemy->SetTarget(player);
            go->AddComponent<GoalWay>();

            // ���R���|�[�l���g��ݒ肷��
            auto mist_generator = engine.Create<GameObject>("mist generator", { 0, 1, 80 });
            auto mist_generatorComponent = mist_generator->AddComponent<MistGenerator>();
            mist_generatorComponent->range = { 15,15 };
            enemy->mistGenerator = mist_generatorComponent;

            boss = enemy;
        }
        // OrcShamanFighter��z�u����
        if (strcmp("OrcShamanFighter", go->name.c_str()) == 0)
        {
            // ���b�V����ǂݍ���
            go->renderer = std::make_shared<SkeletalMeshRenderer>();
            go->renderer->SetMesh(
                engine.GetSkeletalMesh("Res/MeshData/orcshaman/orcshaman_fighter.obj"));

            // �A�j���[�V������ݒ肷��
            auto a = go->AddComponent<Animator>();
            a->AddClip("wait", clipOrcWait);
            a->AddClip("walk", clipOrcWalk);
            a->AddClip("attack", clipOrcAttack);
            a->AddClip("attack.right", clipOrcAttackRight);
            a->AddClip("down.0", clipOrcDown0);
            a->AddClip("down.1", clipOrcDown1);
            a->Play("wait");

            // �I�[�N�̃R���|�[�l���g��ݒ肷��
            auto enemy = go->AddComponent<Orc>();
            enemy->SetAnimator(a);
            enemy->SetTarget(player);
            enemy->SetAttackDistance(3);
            enemy->SetJob(Orc::JOB::FIGHTER);
        }
        // OrcShamanMagician��z�u����
        if (strcmp("OrcShamanMagician", go->name.c_str()) == 0)
        {
            // ���b�V����ǂݍ���
            go->renderer = std::make_shared<SkeletalMeshRenderer>();
            go->renderer->SetMesh(
                engine.GetSkeletalMesh("Res/MeshData/orcshaman/orcshaman_magician.obj"));

            // �A�j���[�V������ݒ肷��
            auto a = go->AddComponent<Animator>();
            a->AddClip("wait", clipOrcWait);
            a->AddClip("walk", clipOrcWalk);
            a->AddClip("magic", clipOrcMagic);
            a->AddClip("down.0", clipOrcDown0);
            a->AddClip("down.1", clipOrcDown1);
            a->Play("wait");

            // �I�[�N�̃R���|�[�l���g��ݒ肷��
            auto enemy = go->AddComponent<Orc>();
            enemy->SetAnimator(a);
            enemy->SetTarget(player);
            enemy->SetAttackDistance(5);
            enemy->SetJob(Orc::JOB::MAGICIAN);
        }
        // �S�[������I�u�W�F�N�g��z�u����
        else if (strcmp("Chest", go->name.c_str()) == 0)
            go->AddComponent<GoalEvent>();
        // �G�������N�����m���z�u����
        else if (go->name == "EnemyTrigger")
            enemyTrigger = go;
        // �{�X�����N�����m���z�u����
        else if (go->name == "BossTrigger")
            bossTrigger = go;
        // �{�X�����̓�����𕕍����邽�߂̊��z�u����
        else if (go->name == "Enter")
            enter = go;
        // �{�X�����̏o���𕕍����邽�߂̊��z�u����
        else if (go->name == "Exit")
            exit = go;
    }

    if (enemyTrigger)
    {
        // �G���G�G���A�ɓ��������肷��g���K�[��ݒ肷��
        auto box = enemyTrigger->GetComponent<BoxCollider>();
        if (box)
            box->isTrigger = true;
        auto a = enemyTrigger->AddComponent<EnemyTrigger>();
        a->guide_control = player->AddComponent<ControlGuide>();
    }
    if (bossTrigger && enter)
    {
        // �{�X�G���A�ɓ��������肷��g���K�[��ݒ肷��
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
        // �o���ƃ{�X��A�g������
        boss->GetOwner()->GetComponent<GoalWay>()->exit = exit;
        boss->GetOwner()->GetComponent<GoalWay>()->guide_goal = player->GetComponent<GoalGuide>();
    }

    return true;
}

/// <summary>
/// ��ɃQ�[���I�[�o�[�ɂȂ��ĂȂ����Ď�����
/// </summary>
/// <param name="engine">�Q�[���G���W��</param>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void ExplorationScene::Update
(
    Engine& engine,
    float deltaTime
)
{
    // �v���C���[������ł�����A�Q�[���I�[�o�[��ʂ�\�����āA��Ԃ�gameOver�ɕύX
    if (playerComponent->GetStatePlayer() == PlayerComponent::STATE_PLAYER::DEAD)
    {
        // �Q�[���I�[�o�[�摜��\��
        engine.CreateUIObject<UILayout>("Res/game_over.dds", { 0,0 }, 0.25f);

        // �^�C�g����ʂɖ߂�{�^����\��
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
/// �V�[�����I������
/// </summary>
/// <param name="engine">�Q�[���G���W��</param>
void ExplorationScene::Finalize(Engine& engine)
{
    EasyAudio::PlayOneShot(SE::click);
    engine.ShowMouseCursor();
    engine.ClearGameObjectAll();
}