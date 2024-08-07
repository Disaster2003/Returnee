/**
* @file SkeletalMeshRenderer.cpp
*/
#include "SkeletalMeshRenderer.h"
#include "GameObject.h"
#include "Engine.h"

/// <summary>
/// 座標変換行列を計算する
/// </summary>
/// <param name="m">計算結果を格納する配列のアドレス</param>
void SkeletalMeshRenderer::CalculateJointMatrix(mat4* matrices)
{
    const size_t jointCount = skeletalMesh->joints.size();

    // 位置、回転、拡大率から座標変換行列を計算
    for (int i = 0; i < jointCount; ++i)
    {
        const auto& t = joint_transforms[i];
        joint_matrices[i] = GetTransformMatrix(t.scale, t.rotation, t.position);
    }

    // 逆バインドポーズ行列とバインドポーズ行列を反映
    std::vector<mat4> m = joint_matrices;
    for (int i = 0; i < jointCount; ++i)
    {
        const MESH_SKELETAL::JOINT& joint = skeletalMesh->joints[i];
        m[i] = joint.bindPose * joint_matrices[i] * joint.inverseBindPose;
    }

    // 親ボーンの関節行列を反映
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

    // 他のクラスから使えるように、計算した関節行列をjointMatricesにコピーする
    std::copy_n(matrices, jointCount, joint_matrices.data());
}

/// <summary>
/// スケルタルメッシュを描画する
/// </summary>
/// <param name="gameObject">レンダラーの親オブジェクト</param>
/// <param name="program">描画に使うプログラムオブジェクト</param>
/// <param name="ssbo">関節行列がコピーされたSSBO</param>
/// <param name="bindingPoint">SSBOのバインディングポイント</param>
void SkeletalMeshRenderer::Draw
(
    const GameObject& gameObject,
    ProgramObject& program,
    GLuint ssbo,
    GLuint number_binging
)
{
    const size_t jointCount = skeletalMesh->joints.size();

    // SSBOをバインド
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
        ssbo, gpuOffset, sizeof(mat4) * jointCount);

    // 座標変換行列をGPUメモリにコピー
    glProgramUniformMatrix4fv(program, locTransformMatrix,
        1, GL_FALSE, &gameObject.GetTransformMatrix()[0].x);

    // 固有マテリアルがあれば固有マテリアルを、なければ共有マテリアルを使って描画
    const MaterialList* materials = &skeletalMesh->materials;
    if (!gameObject.materials.empty())
        materials = &gameObject.materials;

    if (program.ColorLocation() >= 0)
        ::Draw(skeletalMesh->drawParamsList, program, *materials, &gameObject.color);
    else
        ::Draw(skeletalMesh->drawParamsList, program, *materials, nullptr);
}