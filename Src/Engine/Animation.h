/**
* @file Animation.h
*/
#ifndef ANIMATION_H_INCLUDED
#define ANIMATION_H_INCLUDED
#include "VecMath.h"
#include "Component.h"
#include <vector>
#include <string>
#include <unordered_map>

// ��s�錾
class GameObject;

// �����ƃA�j���[�V������Ԃ̃y�A
struct ANIMATION_KEYFRAME
{
    float time; // ����
    vec3 value; // �A�j���[�V�������
};

/// <summary>
/// �L�[�t���[���̏W��
/// </summary>
class AnimationCurve
{
public:
    // �L�[�t���[����ǉ�����
    bool AddKey(float time, const vec3& value);

    // �L�[�t���[���z���ǉ�����
    int AddKey(const ANIMATION_KEYFRAME* begin, const ANIMATION_KEYFRAME* end);

    // ����̎����̃A�j���[�V������Ԃ��擾����
    vec3 Evaluate(float time) const;

    // �A�j���[�V�����̒���(�b)���擾����
    float Length() const;

private:
    std::vector<ANIMATION_KEYFRAME> keys; // �L�[�t���[��
};
using AnimationCurvePtr = std::shared_ptr<AnimationCurve>;

// �A�j���[�V�����̑ΏۂƂȂ�f�[�^
enum class ANIMATION_TARGET
{
    POSITION, // �ʒu
    ROTATION, // ��]
    SCALE,    // �g�嗦
};

/// <summary>
/// �A�j���[�V�����J�[�u�̏W��
/// </summary>
class AnimationClip
{
public:
    // �A�j���[�V�����J�[�u��ǉ�����
    bool AddCurve(int target_bone, ANIMATION_TARGET target, const AnimationCurvePtr& curve);

    // �A�j���[�V�����𔽉f����
    void Evaluate(GameObject& gameObject, float time);

    /// <summary>
    /// �A�j���[�V�����̍Đ����Ԃ��擾����
    /// </summary>
    float Length() const { return length; }

    /// <summary>
    /// ���[�v�t���O���擾����
    /// </summary>
    /// <returns>true = ���[�v����Afalse = ���[�v���Ȃ�</returns>
    bool IsLoop() const { return isLoop; }

    /// <summary>
    /// ���[�v�t���O��ݒ肷��
    /// </summary>
    /// <param name="flag">true = ���[�v����Afalse = ���[�v���Ȃ�</param>
    void SetLoopFlag(bool flag) { isLoop = flag; }

private:
    // �A�j���[�V�����J�[�u�̐ݒ���
    struct ANIMATION_CURVE_INFO
    {
        int targetBone;          // �A�j���[�V�����̑ΏۂƂȂ�{�[���ԍ�
        ANIMATION_TARGET target; // �A�j���[�V�����̑ΏۂƂȂ�f�[�^
        AnimationCurvePtr curve; // �A�j���[�V�����J�[�u
    };
    std::vector<ANIMATION_CURVE_INFO> curves;

    float length = 0.0f; // �A�j���[�V�����̍Đ�����(�b)
    bool isLoop = false; // �A�j���[�V������true = ���[�v����Afalse = ���[�v���Ȃ�
};
 using AnimationClipPtr = std::shared_ptr<AnimationClip>;

/// <summary>
/// �A�j���[�V�����𐧌䂷��R���|�[�l���g
/// </summary>
class Animator
    : public Component
{
public:
    // �f�t�H���g�R���X�g���N�^
    Animator() = default;
    // �f�t�H���g�f�X�g���N�^
    virtual ~Animator() = default;

    // �A�j���[�V�������X�V����
    virtual void Update(float deltaTime) override;

    // �A�j���[�V�����N���b�v��ǉ�����
    void AddClip(const char* name, const AnimationClipPtr& clip);

    // �A�j���[�V�������Đ�����
    void Play(const char* name);

    // �A�j���[�V�������~����
    void Stop();
        
    /// <summary>
    /// �����Ԃ��擾����
    /// </summary>
    /// <returns>true = �Đ����Afalse = ��~��</returns>
    bool IsPlaying() const { return isPlaying; }

    /// <summary>
    /// �Đ����̃A�j���[�V�����N���b�v�����擾����
    /// </summary>
    const std::string& GetCurrentClipName() const { return current_clip_name; }

    /// <summary>
    /// �A�j���[�V�������x���擾����
    /// </summary>
    float GetSpeed() const { return speed; }

    /// <summary>
    /// �A�j���[�V�������x��ݒ肷��
    /// </summary>
    /// <param name="s">�K�p����A�j���[�V�������x</param>
    void SetSpeed(float s) { speed = s; }

    /// <summary>
    /// �o�ߎ��Ԃ��擾����
    /// </summary>
    float GetTimer() const { return timer; }

    /// <summary>
    /// �o�ߎ��Ԃ�ݒ肷��
    /// </summary>
    /// <param name="t">�K�p����o�ߎ���</param>
    void SetTimer(float t) { timer = t; }

private:
    std::unordered_map<std::string, AnimationClipPtr> clips; // �N���b�v�z��
    std::string current_clip_name; // �Đ����̃A�j���[�V�����N���b�v��
    AnimationClipPtr current_clip; // �ݒ肳��Ă���A�j���[�V�����N���b�v
    bool isPlaying = false;        // �A�j���[�V������true = �Đ����Afalse = ��~��
    float timer = 0.0f;            // �A�j���[�V�����̌o�ߎ���
    float speed = 1.0f;            // �A�j���[�V�������x
};
using AnimationPtr = std::shared_ptr<Animator>;

 #endif // ANIMATION_H_INCLUDED