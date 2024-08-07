/**
* @file shadow_skeletal.vert
*/
#version 450

// シェーダへの入力
layout(location = 0) in vec3 inPosition; // 頂点座標
layout(location = 1) in vec2 inTexcoord; // テクスチャ座標
layout(location = 4) in uvec4 inJoints;  // 座標変換行列の番号
layout(location = 5) in vec4 inWeights;  // 座標変換行列の影響度

// シェーダからの出力
layout(location = 1) out vec2 outTexcoord; // テクスチャ座標

// プログラムからの入力
layout(location = 0) uniform mat4 transformMatrix;      // 座標変換行列
layout(location = 2) uniform mat4 viewProjectionMatrix; // ビュープロジェクション行列

// 関節データ用のSSBO
layout(std430, binding = 0) buffer JointDataBlock
{
    mat4 jointMatrices[]; // 関節の座標変換行列
};

// エントリーポイント
void main()
{
    // テクスチャ座標を設定する
    outTexcoord = inTexcoord;
    
    // 姿勢行列にウェイトを掛けて加算合成し、モデル行列を作成する
    mat4 jointMatrix =
      jointMatrices[inJoints.x] * inWeights.x +
      jointMatrices[inJoints.y] * inWeights.y +
      jointMatrices[inJoints.z] * inWeights.z +
      jointMatrices[inJoints.w] * inWeights.w;
    
    // ローカル座標系からワールド座標系に変換する
    gl_Position = transformMatrix * (jointMatrix * vec4(inPosition, 1));
    
    // ワールド座標系からクリップ座標系に変換する
    gl_Position = viewProjectionMatrix * gl_Position;
}