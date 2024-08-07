/**
* @file ShpereCollider.h
*/
#ifndef SPHERECOLLIDER_H_INCLUDED
#define SPHERECOLLIDER_H_INCLUDED
#include "Collider.h"
#include "Collision.h"

/// <summary>
/// 球体コライダー
/// </summary>
class SphereCollider
    : public Collider
{
public:
    // デフォルトコンストラクタ
    inline SphereCollider() = default;
    // デフォルトデストラクタ
    inline virtual ~SphereCollider() = default;

    /// <summary>
    /// 図形の種類を取得する
    /// </summary>
    /// <returns>SPHERE</returns>
    inline TYPE_SHAPE GetType() const override { return TYPE_SHAPE::SPHERE; }

    /// <summary>
    /// 座標を変更する
    /// </summary>
    /// <param name="translate">変更分の座標</param>
    inline void AddPosition(const vec3& translate) override
    {
        sphere.position += translate;
    }

    /// <summary>
    /// 座標変換したコライダーを取得する
    /// </summary>
    /// <param name="transform">座標変換行列</param>
    /// <returns>座標変換したコピー</returns>
    inline ColliderPtr GetTransformedCollider(const mat4& transform) const override
    {
        // 中心座標を座標変換する
        auto p = std::make_shared<SphereCollider>();
        p->sphere.position = vec3(transform * vec4(sphere.position, 1));

        // 球体の拡大率はオブジェクトのXYZ拡大率のうち最大のものとする(Unity準拠)
        const vec3 scale = ExtractScale(transform);
        const float maxScale = std::max({ scale.x, scale.y, scale.z });
        p->sphere.radius = sphere.radius * maxScale;
        return p;
    }

    /// <summary>
    /// 図形を取得する
    /// </summary>
    /// <returns>SPHERE</returns>
    inline const SPHERE& GetShape() const { return sphere; }

    SPHERE sphere = { vec3(0), 1 }; // 図形(球体)
};

#endif