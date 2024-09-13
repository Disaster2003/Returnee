/**
* @file Animation.cpp
*/
#include "Animation.h"
#include "GameObject.h"
#include "SkeletalMeshRenderer.h"
#include "Debug.h"
#include <algorithm>

/// <summary>
/// �L�[�t���[����ǉ�����
/// </summary>
/// <param name="time">�t���[��</param>
/// <param name="value">�L�[</param>
/// <returns>true = �ǉ������Afalse = �ǉ����s</returns>
bool AnimationCurve::AddKey
(
    float time,
    const vec3& value
)
{
    // �ǉ�����ʒu��T������
    auto itr = std::lower_bound(keys.begin(), keys.end(), time,
        [](const ANIMATION_KEYFRAME& key, float time) { return key.time < time; });

    // �����̓������L�[�t���[�������݂���ꍇ�͒ǉ��ł��Ȃ�
    if (itr != keys.end() && itr->time == time)
    {
        LOG_WARNING
        (
            "���Ԃ̏d������KeyFrame�͒ǉ��ł��܂���(time=%0.3f, value=%0.3f,%0.3f,%0.3f",
            time,
            value.x,
            value.y,
            value.z
        );
        return false; // �ǉ����s
    }

    // �L�[�t���[����ǉ�����
    keys.insert(itr, ANIMATION_KEYFRAME{ time, value });
    return true;      // �ǉ�����
}

/// <summary>
/// �L�[�t���[���z���ǉ�����
/// </summary>
/// <param name="begin">�v�f���̍ŏ��l</param>
/// <param name="end">�v�f���̍ő�l</param>
/// <returns>�ǉ������v�f��</returns>
int AnimationCurve::AddKey
(
    const ANIMATION_KEYFRAME* begin,
    const ANIMATION_KEYFRAME* end
)
{
    keys.reserve(end - begin); // �v�f����\�񂷂�
                               
    int count = 0;             // �ǉ������v�f��
    for (auto itr = begin; itr != end; ++itr)
    {
        count += AddKey(itr->time, itr->value);
    }

    return count;
}

/// <summary>
/// ����̃t���[���̃A�j���[�V������Ԃ��擾����
/// </summary>
/// <param name="time">�t���[��</param>
vec3 AnimationCurve::Evaluate(float time) const
{
    if (keys.empty())
    {
        return vec3(0);             // �L�[�t���[�����Ȃ��ꍇ
    }
    else if (keys.size() == 1)
    {
        return keys.begin()->value; // �L�[�t���[�����ЂƂ̏ꍇ
    }

    // �����ɑΉ�����L�[�t���[������������
    auto itr = std::lower_bound(keys.begin(), keys.end(), time,
        [](const ANIMATION_KEYFRAME& k, float time) { return k.time < time; });

    // �擪�����������ꍇ�͐擪�L�[�t���[���̒l��Ԃ�
    if (itr == keys.begin())
    {
        return itr->value;
    }

    // ������Ȃ������ꍇ�͖����̃L�[�t���[���̒l��Ԃ�
    if (itr == keys.end())
    {
        return keys.back().value;
    }

    // �������L�[�t���[���ƂЂƂO�̃L�[�t���[������`��Ԃ���
    auto prev = itr - 1; // �ЂƂO�̃L�[�t���[��
    const float ratio = (time - prev->time) / (itr->time - prev->time);
    return prev->value * (1 - ratio) + itr->value * ratio;
}

/// <summary>
/// �A�j���[�V�����̒���(�b)���擾����
/// </summary>
float AnimationCurve::Length() const
{
    // ������Ȃ��ꍇ
    if (keys.empty())
    {
        // 0��Ԃ�
        return 0;
    }

    // �A�j���[�V�����̒���(�b)�̒l��Ԃ�
    return keys.back().time;
}

/// <summary>
/// �A�j���[�V�����J�[�u��ǉ�����
/// </summary>
/// <param name="target_bone">�֐ߔԍ�</param>
/// <param name="target">���W�A��]�A�傫���̎w��</param>
/// <param name="curve">�A�j���[�V����</param>
/// <returns>true = �ǉ������Afalse = �ǉ����s</returns>
bool AnimationClip::AddCurve
(
    int target_bone,
    ANIMATION_TARGET target,
    const AnimationCurvePtr& curve
)
{
    // �{�[���ԍ��ƃ^�[�Q�b�g����v����f�[�^����������
    auto itr = std::find_if(curves.begin(), curves.end(),
        [target_bone, target](const ANIMATION_CURVE_INFO& info) {
            return info.targetBone == target_bone && info.target == target; });

    // �{�[���ԍ��ƃ^�[�Q�b�g�̗������������J�[�u�����݂���ꍇ�͐ݒ�ł��Ȃ�
    if (itr != curves.end())
    {
        static const char* const targetNames[] = { "position", "rotation", "scale" };
        LOG_WARNING
        (
            "�^�[�Q�b�g���d������J�[�u�͐ݒ�ł��܂���(targetBone=%d, target=%s",
            target_bone,
            targetNames[static_cast<int>(target)]
        );
        return false; // �ǉ����s
    }

    // �V�����J�[�u��ǉ�
    curves.push_back(ANIMATION_CURVE_INFO{ target_bone, target, curve });
    length = std::max(length, curve->Length()); // �������X�V
    return true;      // �ǉ�����
}

/// <summary>
/// �A�j���[�V�����𔽉f����
/// </summary>
/// <param name="gameObject">���f��̃Q�[���I�u�W�F�N�g</param>
/// <param name="time">�A�j���[�V������]�����鎞��</param>
void AnimationClip::Evaluate
(
    GameObject& gameObject,
    float time
)
{
    // �{�[�������擾����
    size_t count = 0;
    if (gameObject.renderer)
    {
        count = gameObject.renderer->GetJointMatrixCount();
    }

    // ���ׂẴA�j���[�V�����J�[�u��]������
    for (const auto& e : curves)
    {
        // �J�[�u�̃{�[���ԍ����{�[�����ȏ�̏ꍇ�͉������Ȃ�
        if (e.targetBone >= count)
        {
            continue;
        }

        // time�ɂ�����J�[�u��]������
        const vec3 v = e.curve->Evaluate(time);

        // �]�����ʂ�Ώۂɔ��f����
        if (e.targetBone >= 0)
        {
            // �{�[���ԍ���0�ȏ�̏ꍇ�A�Ή�����{�[���ɕ]�����ʂ𔽉f����
            // �⑫: �����ɗ���̂�count��1�ȏ�̏ꍇ�����Ȃ̂ŁArenderer��null�`�F�b�N�͕s�v
            JOINT_TRANSFORM& t = gameObject.renderer->GetJointTransform(e.targetBone);
            switch (e.target)
            {
            case ANIMATION_TARGET::POSITION: t.position = v; break;
            case ANIMATION_TARGET::ROTATION: t.rotation = v; break;
            case ANIMATION_TARGET::SCALE: t.scale = v; break;
            }
        }
        else
        {
            // �{�[���ԍ����}�C�i�X�̏ꍇ�A�Q�[���I�u�W�F�N�g�ɕ]�����ʂ𔽉f����
            switch (e.target)
            {
            case ANIMATION_TARGET::POSITION: gameObject.position = v; break;
            case ANIMATION_TARGET::ROTATION: gameObject.rotation = v; break;
            case ANIMATION_TARGET::SCALE: gameObject.scale = v; break;
            }
        }
    } // for curves
}

/// <summary>
/// �A�j���[�V�������X�V����
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void Animator::Update(float deltaTime)
{
    // �N���b�v���ݒ肳��Ă��Ȃ��A�܂��͍Đ�����Ă��Ȃ��ꍇ�͉������Ȃ�
    if (!current_clip || !isPlaying)
    {
        return;
    }

    // �o�ߎ��Ԃ��X�V����
    timer += deltaTime * speed;

    // ���[�v�̗L���ŏ����𕪂���
    const float length = current_clip->Length();
    if (current_clip->IsLoop())
    {
        // �A�j���[�V�����̍Đ����ԂŃ��[�v
        timer = fmod(timer + length, length);
    }
    else
    {
        // �Đ����Ԃ𒴉߂��Ă�����Đ����~����
        if (speed < 0 && timer <= 0)
        {
            isPlaying = false;
            timer = 0;
        }
        else if (speed >= 0 && timer >= length)
        {
            isPlaying = false;
            timer = length;
        }
    }

    // �A�j���[�V�����N���b�v��]������
    current_clip->Evaluate(*GetOwner(), timer);
}

/// <summary>
/// �A�j���[�V�����N���b�v��ǉ�����
/// </summary>
/// <param name="name">�A�j���[�V�����N���b�v�ɕt���閼�O</param>
/// <param name="clip">�ǉ�����A�j���[�V�����N���b�v</param>
void Animator::AddClip
(
    const char* name,
    const AnimationClipPtr& clip
)
{
    // �����A�j���[�V�����N���b�v�����݂���ꍇ�͒ǉ��ł��Ȃ�
    auto itr = clips.find(name);
    if (itr != clips.end())
    {
        LOG_WARNING
        (
            "�������O�̃N���b�v�͒ǉ��ł��܂���(name=%s)",
            name
        );
        return;
    }

    // �A�j���[�V�����N���b�v��ǉ�����
    clips.emplace(name, clip);
}

/// <summary>
/// �A�j���[�V�������Đ�����
/// </summary>
/// <param name="name">�Đ�����A�j���[�V�����N���b�v�̖��O</param>
void Animator::Play(const char* name)
{
    // �A�j���[�V�����N���b�v�����݂��Ȃ��ꍇ�͍Đ��ł��Ȃ�
    auto itr = clips.find(name);
    if (itr == clips.end())
    {
        LOG_WARNING
        (
            "�w�肳�ꂽ���O�̃N���b�v��������܂���(name=%s)",
            name
        );
        return;
    }

    // ���W�ϊ��p�����[�^�������l�ɖ߂�
    auto owner = GetOwner();
    if (owner->renderer)
    {
        owner->renderer->ClearJointTransforms();
    }

    // �N���b�v��ݒ肵�A�Đ���Ԃɂ���
    current_clip_name = itr->first;
    current_clip = itr->second;
    isPlaying = true; // �Đ���Ԃɂ���
    timer = 0;        // �ŏ�����Đ�

    // �A�j���[�V�����N���b�v��]������
    current_clip->Evaluate(*GetOwner(), timer);
}

/// <summary>
/// �A�j���[�V�������~����
/// </summary>
void Animator::Stop()
{
    isPlaying = false;
    timer = 0;
}