/**
* @file AabbCollider.h
*/
#ifndef AABBCOLLIDER_H_INCLUDED
#define AABBCOLLIDER_H_INCLUDED
#include "Collider.h"
#include "Collision.h"

/// <summary>
/// AABBコライダー
/// </summary>
class AabbCollider
    :public Collider
{
public:
    // デフォルトコンストラクタ
    inline AabbCollider() = default;
    // デフォルトデストラクタ
    inline virtual ~AabbCollider() = default;

    /// <summary>
    /// 図形の種類を取得する
    /// </summary>
    /// <returns>AABB</returns>
    inline TYPE_SHAPE GetType() const override { return TYPE_SHAPE::AABB; }

    /// <summary>
    /// 座標を変更する
    /// </summary>
    /// <param name="translate">追加分の座標</param>
    inline void AddPosition(const vec3& translate) override
    {
        aabb.min += translate;
        aabb.max += translate;
    }
    
    /// <summary>
    /// 座標変換したコライダーを取得する
    /// 回転角度は90度単位で指定すること。それ以外の角度では正しい交差判定が行えない。
    /// </summary>
    /// <param name="transform">座標変換行列</param>
    /// <returns>座標変換したコピー</returns>
    ColliderPtr GetTransformedCollider(const mat4& transform) const override
    {
        // 座標変換したコピーを作成
        auto p = std::make_shared<AabbCollider>();
        p->aabb.min = vec3(transform * vec4(aabb.min, 1));
        p->aabb.max = vec3(transform * vec4(aabb.max, 1));

        // minのほうが大きかったら入れ替える
        for (int i = 0; i < 3; ++i)
            if (p->aabb.min[i] > p->aabb.max[i])
            {
                const float tmp = p->aabb.min[i];
                p->aabb.min[i] = p->aabb.max[i];
                p->aabb.max[i] = tmp;
            }

        return p;
    }

   /// <summary>
   /// 図形を取得する
   /// </summary>
   /// <returns>AABB</returns>
   inline const AABB& GetShape() const { return aabb; }


    AABB aabb = { { -1, -1, -1 }, { 1, 1, 1 } }; // 図形(軸平行境界ボックス)
};
using AabbColliderPtr = std::shared_ptr<AabbCollider>;

#endif // !AABBCOLLIDER_H_INCLUDED