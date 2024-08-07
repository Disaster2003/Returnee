/**
* @file Collider.h
*/
#ifndef COLLIDER_H_INCLUDED
#define COLLIDER_H_INCLUDED
#include "Component.h"
#include "VecMath.h"

// ��s�錾
class Collider;
using ColliderPtr = std::shared_ptr<Collider>;

/// <summary>
/// �R���C�_�[�̊��N���X
/// </summary>
class Collider
    : public Component
{
public:
    // �f�t�H���g�R���X�g���N�^
    inline Collider() = default;
    // �f�t�H���g�f�X�g���N�^
    inline virtual ~Collider() = default;

    // �}�`�̎��
    enum class TYPE_SHAPE
    {
        AABB,   // �����s���E�{�b�N�X
        SPHERE, // ����
        BOX,    // �L�����E�{�b�N�X
    };
    /// <summary>
    /// �}�`�̎�ނ��擾����
    /// 
    /// �h���N���X�ɂ���ď������قȂ�A
    /// �K���I�[�o�[���C�h����K�v�����邽�߁A
    /// �������z�֐��ɂ���
    /// </summary>
    inline virtual TYPE_SHAPE GetType() const = 0;

    /// <summary>
    /// ���W��ύX����
    /// 
    /// �h���N���X�ɂ���ď������قȂ�A
    /// �K���I�[�o�[���C�h����K�v�����邽�߁A
    /// �������z�֐��ɂ���
    /// </summary>
    /// <param name="translate">�ǉ����̍��W</param>
    inline virtual void AddPosition(const vec3& translate) = 0;

    /// <summary>
    /// ���W�ϊ������R���C�_�[���擾����
    /// 
    /// �h���N���X�ɂ���ď������قȂ�A
    /// �K���I�[�o�[���C�h����K�v�����邽�߁A
    /// �������z�֐��ɂ���
    /// </summary>
    /// <param name="transform">���W�ϊ��s��</param>
    /// <returns>���W�ϊ������R�s�[</returns>
    inline virtual ColliderPtr GetTransformedCollider(const mat4& transform) const = 0;

    bool isTrigger = false; // �d����true = ���A�@�@�@false = �֎~
    bool isStatic = false;  // ���̂�true = �����Ȃ����Afalse = ������
};

#endif // COLLIDER_H_INCLUDED