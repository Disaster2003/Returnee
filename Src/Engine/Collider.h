/**
* @file Collider.h
*/
#ifndef COLLIDER_H_INCLUDED
#define COLLIDER_H_INCLUDED
#include "Component.h"
#include "VecMath.h"

// 先行宣言
class Collider;
using ColliderPtr = std::shared_ptr<Collider>;

/// <summary>
/// コライダーの基底クラス
/// </summary>
class Collider
    : public Component
{
public:
    // デフォルトコンストラクタ
    inline Collider() = default;
    // デフォルトデストラクタ
    inline virtual ~Collider() = default;

    // 図形の種類
    enum class TYPE_SHAPE
    {
        AABB,   // 軸平行境界ボックス
        SPHERE, // 球体
        BOX,    // 有向境界ボックス
    };
    /// <summary>
    /// 図形の種類を取得する
    /// 
    /// 派生クラスによって処理が異なり、
    /// 必ずオーバーライドする必要があるため、
    /// 純粋仮想関数にする
    /// </summary>
    inline virtual TYPE_SHAPE GetType() const = 0;

    /// <summary>
    /// 座標を変更する
    /// 
    /// 派生クラスによって処理が異なり、
    /// 必ずオーバーライドする必要があるため、
    /// 純粋仮想関数にする
    /// </summary>
    /// <param name="translate">追加分の座標</param>
    inline virtual void AddPosition(const vec3& translate) = 0;

    /// <summary>
    /// 座標変換したコライダーを取得する
    /// 
    /// 派生クラスによって処理が異なり、
    /// 必ずオーバーライドする必要があるため、
    /// 純粋仮想関数にする
    /// </summary>
    /// <param name="transform">座標変換行列</param>
    /// <returns>座標変換したコピー</returns>
    inline virtual ColliderPtr GetTransformedCollider(const mat4& transform) const = 0;

    bool isTrigger = false; // 重複をtrue = 許可、　　　false = 禁止
    bool isStatic = false;  // 物体をtrue = 動かなさい、false = 動かす
};

#endif // COLLIDER_H_INCLUDED