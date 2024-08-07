/**
* @file SkeletalMeshRenderer.h
*/
#ifndef SKELETALMESHRENDERER_H_INCLUDED
#define SKELETALMESHRENDERER_H_INCLUDED
#include "glad/glad.h"
#include "Mesh.h"

// 先行宣言
class GameObject;

// 関節の座標変換パラメータ
struct JOINT_TRANSFORM
{
  vec3 position = vec3(0); // 位置
  vec3 rotation = vec3(0); // 回転
  vec3 scale = vec3(1);    // 拡大率
};

/// <summary>
/// スケルタルメッシュの描画を補助するクラス
/// </summary>
class SkeletalMeshRenderer
{
public:
    friend class Engine;
    // デフォルトコンストラクタ
    SkeletalMeshRenderer() = default;
    // デフォルトデストラクタ
    virtual ~SkeletalMeshRenderer() = default;

    /// <summary>
    /// 描画するスケルタルメッシュを設定する
    /// </summary>
    /// <param name="mesh">描画予定のスケルタルメッシュ</param>
    void SetMesh(const SkeletalMeshPtr& mesh)
    {
        skeletalMesh = mesh;
        joint_matrices.resize(skeletalMesh->joints.size(), mat4(1));
        joint_transforms.resize(skeletalMesh->joints.size());
    }

    /// <summary>
    /// 設定されているスケルタルメッシュを取得する
    /// </summary>
    const SkeletalMeshPtr& GetMesh() const { return skeletalMesh; }

    /// <summary>
    /// 関節の座標変換行列の数を取得する
    /// </summary>
    size_t GetJointMatrixCount() const { return joint_matrices.size(); }

    /// <summary>
    /// 関節の座標変換行列を取得する
    /// </summary>
    /// <param name="i">関節番号</param>
    const mat4& GetJointMatrix(int i) const { return joint_matrices[i]; }

    // 関節の座標変換パラメータを取得する
    JOINT_TRANSFORM& GetJointTransform(int i)
    {
        return joint_transforms[i];
    }
    const JOINT_TRANSFORM& GetJointTransform(int i) const
    {
        return joint_transforms[i];
    }

    /// <summary>
    /// 関節の座標変換パラメータを初期値に戻す
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

    // 座標変換行列を計算する
    void CalculateJointMatrix(mat4* m);

    // スケルタルメッシュを描画する
    void Draw(const GameObject& gameObject,
        ProgramObject& program, GLuint ssbo, GLuint number_binging);

private:
    SkeletalMeshPtr skeletalMesh;     // 描画するスケルタルメッシュ
    std::vector<JOINT_TRANSFORM> joint_transforms; // 関節の座標変換パラメータ
    std::vector<mat4> joint_matrices; // 関節行列の配列
    size_t gpuOffset = 0;             // 関節行列のGPUメモリ上の位置
};
using SkeletalMeshRendererPtr = std::shared_ptr<SkeletalMeshRenderer>;

#endif // SKELETALMESHRENDERER_H_INCLUDED