/**
* @file Animation.cpp
*/
#include "Animation.h"
#include "GameObject.h"
#include "SkeletalMeshRenderer.h"
#include "Debug.h"
#include <algorithm>

/// <summary>
/// キーフレームを追加する
/// </summary>
/// <param name="time">フレーム</param>
/// <param name="value">キー</param>
/// <returns>true = 追加成功、false = 追加失敗</returns>
bool AnimationCurve::AddKey
(
    float time,
    const vec3& value
)
{
    // 追加する位置を探索する
    auto itr = std::lower_bound(keys.begin(), keys.end(), time,
        [](const ANIMATION_KEYFRAME& key, float time) { return key.time < time; });

    // 時刻の等しいキーフレームが存在する場合は追加できない
    if (itr != keys.end() && itr->time == time)
    {
        LOG_WARNING
        (
            "時間の重複するKeyFrameは追加できません(time=%0.3f, value=%0.3f,%0.3f,%0.3f",
            time,
            value.x,
            value.y,
            value.z
        );
        return false; // 追加失敗
    }

    // キーフレームを追加する
    keys.insert(itr, ANIMATION_KEYFRAME{ time, value });
    return true;      // 追加成功
}

/// <summary>
/// キーフレーム配列を追加する
/// </summary>
/// <param name="begin">要素数の最小値</param>
/// <param name="end">要素数の最大値</param>
/// <returns>追加した要素数</returns>
int AnimationCurve::AddKey
(
    const ANIMATION_KEYFRAME* begin,
    const ANIMATION_KEYFRAME* end
)
{
    keys.reserve(end - begin); // 要素数を予約する
                               
    int count = 0;             // 追加した要素数
    for (auto itr = begin; itr != end; ++itr)
    {
        count += AddKey(itr->time, itr->value);
    }

    return count;
}

/// <summary>
/// 特定のフレームのアニメーション状態を取得する
/// </summary>
/// <param name="time">フレーム</param>
vec3 AnimationCurve::Evaluate(float time) const
{
    if (keys.empty())
    {
        return vec3(0);             // キーフレームがない場合
    }
    else if (keys.size() == 1)
    {
        return keys.begin()->value; // キーフレームがひとつの場合
    }

    // 時刻に対応するキーフレームを検索する
    auto itr = std::lower_bound(keys.begin(), keys.end(), time,
        [](const ANIMATION_KEYFRAME& k, float time) { return k.time < time; });

    // 先頭が見つかった場合は先頭キーフレームの値を返す
    if (itr == keys.begin())
    {
        return itr->value;
    }

    // 見つからなかった場合は末尾のキーフレームの値を返す
    if (itr == keys.end())
    {
        return keys.back().value;
    }

    // 見つけたキーフレームとひとつ前のキーフレームを線形補間する
    auto prev = itr - 1; // ひとつ前のキーフレーム
    const float ratio = (time - prev->time) / (itr->time - prev->time);
    return prev->value * (1 - ratio) + itr->value * ratio;
}

/// <summary>
/// アニメーションの長さ(秒)を取得する
/// </summary>
float AnimationCurve::Length() const
{
    // 見つからない場合
    if (keys.empty())
    {
        // 0を返す
        return 0;
    }

    // アニメーションの長さ(秒)の値を返す
    return keys.back().time;
}

/// <summary>
/// アニメーションカーブを追加する
/// </summary>
/// <param name="target_bone">関節番号</param>
/// <param name="target">座標、回転、大きさの指定</param>
/// <param name="curve">アニメーション</param>
/// <returns>true = 追加成功、false = 追加失敗</returns>
bool AnimationClip::AddCurve
(
    int target_bone,
    ANIMATION_TARGET target,
    const AnimationCurvePtr& curve
)
{
    // ボーン番号とターゲットが一致するデータを検索する
    auto itr = std::find_if(curves.begin(), curves.end(),
        [target_bone, target](const ANIMATION_CURVE_INFO& info) {
            return info.targetBone == target_bone && info.target == target; });

    // ボーン番号とターゲットの両方が等しいカーブが存在する場合は設定できない
    if (itr != curves.end())
    {
        static const char* const targetNames[] = { "position", "rotation", "scale" };
        LOG_WARNING
        (
            "ターゲットが重複するカーブは設定できません(targetBone=%d, target=%s",
            target_bone,
            targetNames[static_cast<int>(target)]
        );
        return false; // 追加失敗
    }

    // 新しいカーブを追加
    curves.push_back(ANIMATION_CURVE_INFO{ target_bone, target, curve });
    length = std::max(length, curve->Length()); // 長さを更新
    return true;      // 追加成功
}

/// <summary>
/// アニメーションを反映する
/// </summary>
/// <param name="gameObject">反映先のゲームオブジェクト</param>
/// <param name="time">アニメーションを評価する時刻</param>
void AnimationClip::Evaluate
(
    GameObject& gameObject,
    float time
)
{
    // ボーン数を取得する
    size_t count = 0;
    if (gameObject.renderer)
    {
        count = gameObject.renderer->GetJointMatrixCount();
    }

    // すべてのアニメーションカーブを評価する
    for (const auto& e : curves)
    {
        // カーブのボーン番号がボーン数以上の場合は何もしない
        if (e.targetBone >= count)
        {
            continue;
        }

        // timeにおけるカーブを評価する
        const vec3 v = e.curve->Evaluate(time);

        // 評価結果を対象に反映する
        if (e.targetBone >= 0)
        {
            // ボーン番号が0以上の場合、対応するボーンに評価結果を反映する
            // 補足: ここに来るのはcountが1以上の場合だけなので、rendererのnullチェックは不要
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
            // ボーン番号がマイナスの場合、ゲームオブジェクトに評価結果を反映する
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
/// アニメーションを更新する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Animator::Update(float deltaTime)
{
    // クリップが設定されていない、または再生されていない場合は何もしない
    if (!current_clip || !isPlaying)
    {
        return;
    }

    // 経過時間を更新する
    timer += deltaTime * speed;

    // ループの有無で処理を分ける
    const float length = current_clip->Length();
    if (current_clip->IsLoop())
    {
        // アニメーションの再生時間でループ
        timer = fmod(timer + length, length);
    }
    else
    {
        // 再生時間を超過していたら再生を停止する
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

    // アニメーションクリップを評価する
    current_clip->Evaluate(*GetOwner(), timer);
}

/// <summary>
/// アニメーションクリップを追加する
/// </summary>
/// <param name="name">アニメーションクリップに付ける名前</param>
/// <param name="clip">追加するアニメーションクリップ</param>
void Animator::AddClip
(
    const char* name,
    const AnimationClipPtr& clip
)
{
    // 同じアニメーションクリップが存在する場合は追加できない
    auto itr = clips.find(name);
    if (itr != clips.end())
    {
        LOG_WARNING
        (
            "同じ名前のクリップは追加できません(name=%s)",
            name
        );
        return;
    }

    // アニメーションクリップを追加する
    clips.emplace(name, clip);
}

/// <summary>
/// アニメーションを再生する
/// </summary>
/// <param name="name">再生するアニメーションクリップの名前</param>
void Animator::Play(const char* name)
{
    // アニメーションクリップが存在しない場合は再生できない
    auto itr = clips.find(name);
    if (itr == clips.end())
    {
        LOG_WARNING
        (
            "指定された名前のクリップが見つかりません(name=%s)",
            name
        );
        return;
    }

    // 座標変換パラメータを初期値に戻す
    auto owner = GetOwner();
    if (owner->renderer)
    {
        owner->renderer->ClearJointTransforms();
    }

    // クリップを設定し、再生状態にする
    current_clip_name = itr->first;
    current_clip = itr->second;
    isPlaying = true; // 再生状態にする
    timer = 0;        // 最初から再生

    // アニメーションクリップを評価する
    current_clip->Evaluate(*GetOwner(), timer);
}

/// <summary>
/// アニメーションを停止する
/// </summary>
void Animator::Stop()
{
    isPlaying = false;
    timer = 0;
}