/**
* @file SkeletalMeshRenderer.h
*/
#ifndef SKELETALMESHRENDERER_H_INCLUDED
#define SKELETALMESHRENDERER_H_INCLUDED
#include "glad/glad.h"
#include "Mesh.h"

// ��s�錾
class GameObject;

// �֐߂̍��W�ϊ��p�����[�^
struct JOINT_TRANSFORM
{
  vec3 position = vec3(0); // �ʒu
  vec3 rotation = vec3(0); // ��]
  vec3 scale = vec3(1);    // �g�嗦
};

/// <summary>
/// �X�P���^�����b�V���̕`���⏕����N���X
/// </summary>
class SkeletalMeshRenderer
{
public:
    friend class Engine;
    // �f�t�H���g�R���X�g���N�^
    SkeletalMeshRenderer() = default;
    // �f�t�H���g�f�X�g���N�^
    virtual ~SkeletalMeshRenderer() = default;

    /// <summary>
    /// �`�悷��X�P���^�����b�V����ݒ肷��
    /// </summary>
    /// <param name="mesh">�`��\��̃X�P���^�����b�V��</param>
    void SetMesh(const SkeletalMeshPtr& mesh)
    {
        skeletalMesh = mesh;
        joint_matrices.resize(skeletalMesh->joints.size(), mat4(1));
        joint_transforms.resize(skeletalMesh->joints.size());
    }

    /// <summary>
    /// �ݒ肳��Ă���X�P���^�����b�V�����擾����
    /// </summary>
    const SkeletalMeshPtr& GetMesh() const { return skeletalMesh; }

    /// <summary>
    /// �֐߂̍��W�ϊ��s��̐����擾����
    /// </summary>
    size_t GetJointMatrixCount() const { return joint_matrices.size(); }

    /// <summary>
    /// �֐߂̍��W�ϊ��s����擾����
    /// </summary>
    /// <param name="i">�֐ߔԍ�</param>
    const mat4& GetJointMatrix(int i) const { return joint_matrices[i]; }

    // �֐߂̍��W�ϊ��p�����[�^���擾����
    JOINT_TRANSFORM& GetJointTransform(int i)
    {
        return joint_transforms[i];
    }
    const JOINT_TRANSFORM& GetJointTransform(int i) const
    {
        return joint_transforms[i];
    }

    /// <summary>
    /// �֐߂̍��W�ϊ��p�����[�^�������l�ɖ߂�
    /// </summary>
    void ClearJointTransforms()
    {
        for (auto& e : joint_transforms)
        {
            e.position = vec3(0);
            e.rotation = vec3(0);
            e.scale = vec3(1);
        } // for jointTransforms
    }

    // ���W�ϊ��s����v�Z����
    void CalculateJointMatrix(mat4* m);

    // �X�P���^�����b�V����`�悷��
    void Draw(const GameObject& gameObject,
        ProgramObject& program, GLuint ssbo, GLuint number_binging);

private:
    SkeletalMeshPtr skeletalMesh;     // �`�悷��X�P���^�����b�V��
    std::vector<JOINT_TRANSFORM> joint_transforms; // �֐߂̍��W�ϊ��p�����[�^
    std::vector<mat4> joint_matrices; // �֐ߍs��̔z��
    size_t gpuOffset = 0;             // �֐ߍs���GPU��������̈ʒu
};
using SkeletalMeshRendererPtr = std::shared_ptr<SkeletalMeshRenderer>;

#endif // SKELETALMESHRENDERER_H_INCLUDED