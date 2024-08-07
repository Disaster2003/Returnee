/**
* @file AabbCollider.h
*/
#ifndef AABBCOLLIDER_H_INCLUDED
#define AABBCOLLIDER_H_INCLUDED
#include "Collider.h"
#include "Collision.h"

/// <summary>
/// AABB�R���C�_�[
/// </summary>
class AabbCollider
    :public Collider
{
public:
    // �f�t�H���g�R���X�g���N�^
    inline AabbCollider() = default;
    // �f�t�H���g�f�X�g���N�^
    inline virtual ~AabbCollider() = default;

    /// <summary>
    /// �}�`�̎�ނ��擾����
    /// </summary>
    /// <returns>AABB</returns>
    inline TYPE_SHAPE GetType() const override { return TYPE_SHAPE::AABB; }

    /// <summary>
    /// ���W��ύX����
    /// </summary>
    /// <param name="translate">�ǉ����̍��W</param>
    inline void AddPosition(const vec3& translate) override
    {
        aabb.min += translate;
        aabb.max += translate;
    }
    
    /// <summary>
    /// ���W�ϊ������R���C�_�[���擾����
    /// ��]�p�x��90�x�P�ʂŎw�肷�邱�ƁB����ȊO�̊p�x�ł͐������������肪�s���Ȃ��B
    /// </summary>
    /// <param name="transform">���W�ϊ��s��</param>
    /// <returns>���W�ϊ������R�s�[</returns>
    ColliderPtr GetTransformedCollider(const mat4& transform) const override
    {
        // ���W�ϊ������R�s�[���쐬
        auto p = std::make_shared<AabbCollider>();
        p->aabb.min = vec3(transform * vec4(aabb.min, 1));
        p->aabb.max = vec3(transform * vec4(aabb.max, 1));

        // min�̂ق����傫�����������ւ���
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
   /// �}�`���擾����
   /// </summary>
   /// <returns>AABB</returns>
   inline const AABB& GetShape() const { return aabb; }


    AABB aabb = { { -1, -1, -1 }, { 1, 1, 1 } }; // �}�`(�����s���E�{�b�N�X)
};
using AabbColliderPtr = std::shared_ptr<AabbCollider>;

#endif // !AABBCOLLIDER_H_INCLUDED