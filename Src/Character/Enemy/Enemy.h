/**
* @file Enemy.h
*/
#ifndef ENEMY_H_INCLUDED
#define ENEMY_H_INCLUDED
#include "../Character.h"

/// <summary>
/// �G�S�̂̌p�����N���X
/// </summary>
class Enemy
    :public Character
{
public:
    // �R���X�g���N�^
    Enemy(int _hp)
        : Character(_hp)
    {}

    // �f�t�H���g�f�X�g���N�^
    virtual ~Enemy() = default;

    // ���@�U�����s��
    void AttackMagicMissile(GameObject& _owner);

    // �v���C���[�Ƃ̑��֊֌W
    struct TARGET_INFO
    {
        // �v���C���[�Ƃ̈ʒu�֌W
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
        float cos��_by_left = 0;
        float cos��_by_front = 0;

        // ��񂪎擾�ł��Ă�����true
        constexpr explicit operator bool() const { return position_type != POSITON_TYPE::NO_TARGET; }
    };
    // �U���Ώۂ܂ł̋����ƌ������擾����
    TARGET_INFO GetTargetInfo() const;

    /// <summary>
    /// �W�I���擾����
    /// </summary>
    /// <returns>�v���C���[</returns>
    GameObjectPtr GetTarget() const { return target; }

    /// <summary>
    /// �W�I��ݒ肷��
    /// </summary>
    /// <param name="t">�v���C���[</param>
    void SetTarget(const GameObjectPtr& t) { target = t; }

protected:
    static constexpr float DISTANCE_SEARCH = 5.0f;     // ���G����
    // �ǐՋ���(���̋�����艓���Ȃ�����ǐՂ���߂�)
    static constexpr float DISTANCE_TRACK_MAX = 10.0f;
    static constexpr float DISTANCE_ATTACK = 3.0f;     // �U������

    static constexpr float SPEED_MOVE_ENEMY = 3.0f;	   // �ړ����x
    static constexpr float DECELERATION = 10.0f;       // ������
    static constexpr float SPEED_ANIMATION = 1.0f;     // �A�j���[�V�������x
    static constexpr float SPEED_MAGICMISSILE = 18.0f; // ���@�e�̑��x

private:
    GameObjectPtr target; // �v���C���[
};

#endif // !ENEMY_H_INCLUDED