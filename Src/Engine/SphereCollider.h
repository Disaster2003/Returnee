/**
* @file ShpereCollider.h
*/
#ifndef SPHERECOLLIDER_H_INCLUDED
#define SPHERECOLLIDER_H_INCLUDED
#include "Collider.h"
#include "Collision.h"

/// <summary>
/// ���̃R���C�_�[
/// </summary>
class SphereCollider
    : public Collider
{
public:
    // �f�t�H���g�R���X�g���N�^
    inline SphereCollider() = default;
    // �f�t�H���g�f�X�g���N�^
    inline virtual ~SphereCollider() = default;

    /// <summary>
    /// �}�`�̎�ނ��擾����
    /// </summary>
    /// <returns>SPHERE</returns>
    inline TYPE_SHAPE GetType() const override { return TYPE_SHAPE::SPHERE; }

    /// <summary>
    /// ���W��ύX����
    /// </summary>
    /// <param name="translate">�ύX���̍��W</param>
    inline void AddPosition(const vec3& translate) override
    {
        sphere.position += translate;
    }

    /// <summary>
    /// ���W�ϊ������R���C�_�[���擾����
    /// </summary>
    /// <param name="transform">���W�ϊ��s��</param>
    /// <returns>���W�ϊ������R�s�[</returns>
    inline ColliderPtr GetTransformedCollider(const mat4& transform) const override
    {
        // ���S���W�����W�ϊ�����
        auto p = std::make_shared<SphereCollider>();
        p->sphere.position = vec3(transform * vec4(sphere.position, 1));

        // ���̂̊g�嗦�̓I�u�W�F�N�g��XYZ�g�嗦�̂����ő�̂��̂Ƃ���(Unity����)
        const vec3 scale = ExtractScale(transform);
        const float maxScale = std::max({ scale.x, scale.y, scale.z });
        p->sphere.radius = sphere.radius * maxScale;
        return p;
    }

    /// <summary>
    /// �}�`���擾����
    /// </summary>
    /// <returns>SPHERE</returns>
    inline const SPHERE& GetShape() const { return sphere; }

    SPHERE sphere = { vec3(0), 1 }; // �}�`(����)
};

#endif