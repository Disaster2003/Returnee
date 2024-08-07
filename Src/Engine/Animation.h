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

// 先行宣言
class GameObject;

// 時刻とアニメーション状態のペア
struct ANIMATION_KEYFRAME
{
    float time; // 時刻
    vec3 value; // アニメーション状態
};

/// <summary>
/// キーフレームの集合
/// </summary>
class AnimationCurve
{
public:
    // キーフレームを追加する
    bool AddKey(float time, const vec3& value);

    // キーフレーム配列を追加する
    int AddKey(const ANIMATION_KEYFRAME* begin, const ANIMATION_KEYFRAME* end);

    // 特定の時刻のアニメーション状態を取得する
    vec3 Evaluate(float time) const;

    // アニメーションの長さ(秒)を取得する
    float Length() const;

private:
    std::vector<ANIMATION_KEYFRAME> keys; // キーフレーム
};
using AnimationCurvePtr = std::shared_ptr<AnimationCurve>;

// アニメーションの対象となるデータ
enum class ANIMATION_TARGET
{
    POSITION, // 位置
    ROTATION, // 回転
    SCALE,    // 拡大率
};

/// <summary>
/// アニメーションカーブの集合
/// </summary>
class AnimationClip
{
public:
    // アニメーションカーブを追加する
    bool AddCurve(int target_bone, ANIMATION_TARGET target, const AnimationCurvePtr& curve);

    // アニメーションを反映する
    void Evaluate(GameObject& gameObject, float time);

    /// <summary>
    /// アニメーションの再生時間を取得する
    /// </summary>
    float Length() const { return length; }

    /// <summary>
    /// ループフラグを取得する
    /// </summary>
    /// <returns>true = ループする、false = ループしない</returns>
    bool IsLoop() const { return isLoop; }

    /// <summary>
    /// ループフラグを設定する
    /// </summary>
    /// <param name="flag">true = ループする、false = ループしない</param>
    void SetLoopFlag(bool flag) { isLoop = flag; }

private:
    // アニメーションカーブの設定情報
    struct ANIMATION_CURVE_INFO
    {
        int targetBone;          // アニメーションの対象となるボーン番号
        ANIMATION_TARGET target; // アニメーションの対象となるデータ
        AnimationCurvePtr curve; // アニメーションカーブ
    };
    std::vector<ANIMATION_CURVE_INFO> curves;

    float length = 0.0f; // アニメーションの再生時間(秒)
    bool isLoop = false; // アニメーションをtrue = ループする、false = ループしない
};
 using AnimationClipPtr = std::shared_ptr<AnimationClip>;

/// <summary>
/// アニメーションを制御するコンポーネント
/// </summary>
class Animator
    : public Component
{
public:
    // デフォルトコンストラクタ
    Animator() = default;
    // デフォルトデストラクタ
    virtual ~Animator() = default;

    // アニメーションを更新する
    virtual void Update(float deltaTime) override;

    // アニメーションクリップを追加する
    void AddClip(const char* name, const AnimationClipPtr& clip);

    // アニメーションを再生する
    void Play(const char* name);

    // アニメーションを停止する
    void Stop();
        
    /// <summary>
    /// 動作状態を取得する
    /// </summary>
    /// <returns>true = 再生中、false = 停止中</returns>
    bool IsPlaying() const { return isPlaying; }

    /// <summary>
    /// 再生中のアニメーションクリップ名を取得する
    /// </summary>
    const std::string& GetCurrentClipName() const { return current_clip_name; }

    /// <summary>
    /// アニメーション速度を取得する
    /// </summary>
    float GetSpeed() const { return speed; }

    /// <summary>
    /// アニメーション速度を設定する
    /// </summary>
    /// <param name="s">適用するアニメーション速度</param>
    void SetSpeed(float s) { speed = s; }

    /// <summary>
    /// 経過時間を取得する
    /// </summary>
    float GetTimer() const { return timer; }

    /// <summary>
    /// 経過時間を設定する
    /// </summary>
    /// <param name="t">適用する経過時間</param>
    void SetTimer(float t) { timer = t; }

private:
    std::unordered_map<std::string, AnimationClipPtr> clips; // クリップ配列
    std::string current_clip_name; // 再生中のアニメーションクリップ名
    AnimationClipPtr current_clip; // 設定されているアニメーションクリップ
    bool isPlaying = false;        // アニメーションがtrue = 再生中、false = 停止中
    float timer = 0.0f;            // アニメーションの経過時間
    float speed = 1.0f;            // アニメーション速度
};
using AnimationPtr = std::shared_ptr<Animator>;

 #endif // ANIMATION_H_INCLUDED