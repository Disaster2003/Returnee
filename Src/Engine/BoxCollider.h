/**
* @file BoxCollider.h
*/
#ifndef BOXCOLLIDER_H_INCLUDED
#define BOXCOLLIDER_H_INCLUDED
#include "Collider.h"
#include "Collision.h"

/// <summary>
/// Boxコライダー
/// </summary>
class BoxCollider
    : public Collider
{
public:
    // デフォルトコンストラクタ
    inline BoxCollider() = default;
    // デフォルトデストラクタ
    inline virtual ~BoxCollider() = default;

    /// <summary>
    /// 図形の種類を取得する
    /// </summary>
    /// <returns>BOX</returns>
    inline TYPE_SHAPE GetType() const override { return TYPE_SHAPE::BOX; }

    /// <summary>
    /// 座標を変更する
    /// </summary>
    /// <param name="translate">追加分の座標</param>
    inline void AddPosition(const vec3& translate) override
    {
        box.position += translate;
    }

    /// <summary>
    /// 座標変換したコライダーを取得する
    /// </summary>
    /// <param name="transform">座標変換行列</param>
    /// <returns>座標変換したコピーを作成</returns>
    ColliderPtr GetTransformedCollider(const mat4& transform) const override
    {
        // 座標変換行列を分解
        vec3 translate; // 未使用
        vec3 scale;
        mat3 rotation;
        Decompose(transform, translate, scale, rotation);

        // 座標変換したコピーを作成
        auto p = std::make_shared<BoxCollider>();
        p->box.position = vec3(transform * vec4(box.position, 1));
        for (int i = 0; i < 3; ++i)
        {
            p->box.axis[i] = rotation * box.axis[i];   // 軸ベクトル
            p->box.scale[i] = box.scale[i] * scale[i]; // 拡大率
        }

        return p;
    }

    /// <summary>
    /// 図形を取得する
    /// </summary>
    /// <returns>BOX</returns>
    inline const BOX& GetShape() const { return box; }

    BOX box =
    {
      { 0, 0, 0 },
      { { 1, 0, 0}, { 0, 1, 0 }, { 0, 0, 1} },
      { 1, 1, 1 }
    };
};
using BoxColliderPtr = std::shared_ptr<BoxCollider>;

#endif // BOXCOLLIDER_H_INCLUDED