/**
* @file BoxCollider.h
*/
#ifndef BOXCOLLIDER_H_INCLUDED
#define BOXCOLLIDER_H_INCLUDED
#include "Collider.h"
#include "Collision.h"

/// <summary>
/// Box�R���C�_�[
/// </summary>
class BoxCollider
    : public Collider
{
public:
    // �f�t�H���g�R���X�g���N�^
    inline BoxCollider() = default;
    // �f�t�H���g�f�X�g���N�^
    inline virtual ~BoxCollider() = default;

    /// <summary>
    /// �}�`�̎�ނ��擾����
    /// </summary>
    /// <returns>BOX</returns>
    inline TYPE_SHAPE GetType() const override { return TYPE_SHAPE::BOX; }

    /// <summary>
    /// ���W��ύX����
    /// </summary>
    /// <param name="translate">�ǉ����̍��W</param>
    inline void AddPosition(const vec3& translate) override
    {
        box.position += translate;
    }

    /// <summary>
    /// ���W�ϊ������R���C�_�[���擾����
    /// </summary>
    /// <param name="transform">���W�ϊ��s��</param>
    /// <returns>���W�ϊ������R�s�[���쐬</returns>
    ColliderPtr GetTransformedCollider(const mat4& transform) const override
    {
        // ���W�ϊ��s��𕪉�
        vec3 translate; // ���g�p
        vec3 scale;
        mat3 rotation;
        Decompose(transform, translate, scale, rotation);

        // ���W�ϊ������R�s�[���쐬
        auto p = std::make_shared<BoxCollider>();
        p->box.position = vec3(transform * vec4(box.position, 1));
        for (int i = 0; i < 3; ++i)
        {
            p->box.axis[i] = rotation * box.axis[i];   // ���x�N�g��
            p->box.scale[i] = box.scale[i] * scale[i]; // �g�嗦
        }

        return p;
    }

    /// <summary>
    /// �}�`���擾����
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