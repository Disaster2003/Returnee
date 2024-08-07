/**
* @file SkeletalMeshRenderer.cpp
*/
#include "SkeletalMeshRenderer.h"
#include "GameObject.h"
#include "Engine.h"

/// <summary>
/// ���W�ϊ��s����v�Z����
/// </summary>
/// <param name="m">�v�Z���ʂ��i�[����z��̃A�h���X</param>
void SkeletalMeshRenderer::CalculateJointMatrix(mat4* matrices)
{
    const size_t jointCount = skeletalMesh->joints.size();

    // �ʒu�A��]�A�g�嗦������W�ϊ��s����v�Z
    for (int i = 0; i < jointCount; ++i)
    {
        const auto& t = joint_transforms[i];
        joint_matrices[i] = GetTransformMatrix(t.scale, t.rotation, t.position);
    }

    // �t�o�C���h�|�[�Y�s��ƃo�C���h�|�[�Y�s��𔽉f
    std::vector<mat4> m = joint_matrices;
    for (int i = 0; i < jointCount; ++i)
    {
        const MESH_SKELETAL::JOINT& joint = skeletalMesh->joints[i];
        m[i] = joint.bindPose * joint_matrices[i] * joint.inverseBindPose;
    }

    // �e�{�[���̊֐ߍs��𔽉f
    for (int i = 0; i < jointCount; ++i)
    {
        matrices[i] = m[i];
        int32_t p = skeletalMesh->joints[i].parent;
        while (p >= 0)
        {
            matrices[i] = m[p] * matrices[i];
            p = skeletalMesh->joints[p].parent;
        }
    } // for i

    // ���̃N���X����g����悤�ɁA�v�Z�����֐ߍs���jointMatrices�ɃR�s�[����
    std::copy_n(matrices, jointCount, joint_matrices.data());
}

/// <summary>
/// �X�P���^�����b�V����`�悷��
/// </summary>
/// <param name="gameObject">�����_���[�̐e�I�u�W�F�N�g</param>
/// <param name="program">�`��Ɏg���v���O�����I�u�W�F�N�g</param>
/// <param name="ssbo">�֐ߍs�񂪃R�s�[���ꂽSSBO</param>
/// <param name="bindingPoint">SSBO�̃o�C���f�B���O�|�C���g</param>
void SkeletalMeshRenderer::Draw
(
    const GameObject& gameObject,
    ProgramObject& program,
    GLuint ssbo,
    GLuint number_binging
)
{
    const size_t jointCount = skeletalMesh->joints.size();

    // SSBO���o�C���h
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
        ssbo, gpuOffset, sizeof(mat4) * jointCount);

    // ���W�ϊ��s���GPU�������ɃR�s�[
    glProgramUniformMatrix4fv(program, locTransformMatrix,
        1, GL_FALSE, &gameObject.GetTransformMatrix()[0].x);

    // �ŗL�}�e���A��������ΌŗL�}�e���A�����A�Ȃ���΋��L�}�e���A�����g���ĕ`��
    const MaterialList* materials = &skeletalMesh->materials;
    if (!gameObject.materials.empty())
        materials = &gameObject.materials;

    if (program.ColorLocation() >= 0)
        ::Draw(skeletalMesh->drawParamsList, program, *materials, &gameObject.color);
    else
        ::Draw(skeletalMesh->drawParamsList, program, *materials, nullptr);
}