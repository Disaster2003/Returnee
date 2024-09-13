/**
* @file Mesh.h
*/
#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED
#include "Texture.h"
#include "BufferObject.h"
#include "VertexArrayObject.h"
#include "VecMath.h"
#include "ProgramObject.h"
#include <vector>
#include <unordered_map>
#include <functional>

//先行宣言
class MeshBuffer;
using MeshBufferPtr = std::shared_ptr<MeshBuffer>;


// 頂点データ形式
struct VERTEX
{
	vec3 position; // 頂点座標
	vec2 texcoord; // テクスチャ座標
	vec3 normal;   // 法線ベクトル
	vec4 tangent;  // 接線ベクトル
};

// 描画パラメータ
struct DRAW_PARAMS
{
	GLenum mode = GL_TRIANGLES;	// プリミティブの種類
	GLsizei count = 0;			// 描画するインデックス数
	const void* indices = 0;	// 描画開始インデックスのバイトオフセット
	GLint baseVertex = 0;		// インデックス0となる頂点配列内の位置
	int materialNo = -1;		// マテリアルインデックス
};

// マテリアル
struct MATERIAL
{
	std::string name = "<Default>";	// マテリアル名
	vec4 baseColor = { 1,1,1,1 };	// 基本色+アルファ
	vec3 emission = { 0,0,0 };		// 発光色
	float roughness = 0.7f;         // 表面の粗さ
	float metallic = 0.0f;          // 0 = 非金属、1 = 金属
	TexturePtr texBaseColor;		// 基本色テクスチャ
	TexturePtr texEmission;			// 発光色テクスチャ
	TexturePtr texNormal;           // 法線テクスチャ
};
using MaterialPtr = std::shared_ptr<MATERIAL>;
using MaterialList = std::vector<MaterialPtr>; // マテリアル配列型

// 3Dモデル
struct MESH_STATIC
{
	std::string name;		//メッシュ名
	std::vector<DRAW_PARAMS> drawParamsList; //描画パラメータ
	MaterialList materials; // 共有マテリアル配列
};
using StaticMeshPtr = std::shared_ptr<MESH_STATIC>;

// メッシュを描画する
void Draw(const MESH_STATIC & mesh, ProgramObject& program, const MaterialList & materials);
// 描画パラメータ配列を描画する
void Draw(const std::vector<DRAW_PARAMS>& drawParamsList, ProgramObject& program,
	const MaterialList & materials, const vec4* objectColor);

// 関節付き頂点データ形式
struct VERTEX_SKELETAL
{
	vec3 position;		 // 頂点座標
	vec2 texcoord;		 // テクスチャ座標
	vec3 normal;		 // 法線ベクトル
	vec4 tangent;		 // 接線ベクトル
	uint16_t joints[4];	 // 影響を受ける関節の番号
	uint16_t weights[4]; // 各関節の影響度
};

// 関節付きメッシュ
struct MESH_SKELETAL
{
	std::string name;		// メッシュ名
	std::vector<DRAW_PARAMS> drawParamsList; // 描画パラメータ
	MaterialList materials; // 共有マテリアル

	// 関節データ
	struct JOINT
	{
		mat4 bindPose;        // バインドポーズ行列
		mat4 inverseBindPose; // 逆バインドポーズ行列
		int32_t parent;       // 親関節の番号
	};
	std::vector<JOINT> joints;
};
using SkeletalMeshPtr = std::shared_ptr<MESH_SKELETAL>;

// 

/// <summary>
/// 共有マテリアル配列を複製する
/// </summary>
/// <param name="original">マテリアル配列元</param>
/// <returns>複製したもの</returns>
inline MaterialList CloneMaterialList(const StaticMeshPtr& original)
{
	MaterialList clone(original->materials.size());
	for (int i = 0; i < clone.size(); ++i)
	{
		clone[i] = std::make_shared<MATERIAL>(*original->materials[i]);
	}

	return clone;
}

/// <summary>
/// 頂点データを管理するクラス
/// </summary>
class MeshBuffer
{
public:
	/// <summary>
	/// メッシュバッファを作成する
	/// </summary>
	/// <param name="buffer_size">格納できる頂点データのサイズ(バイト数)</param>
	static MeshBufferPtr Create(size_t buffer_size)
	{
		return std::make_shared<MeshBuffer>(buffer_size);
	}

	// コンストラクタ
	MeshBuffer(size_t buffer_size);
	// デフォルトデストラクタ
	virtual ~MeshBuffer() = default;

	// コピーと代入を禁止する
	MeshBuffer(const MeshBuffer&) = delete;
	MeshBuffer& operator=(const MeshBuffer&) = delete;

	/// <summary>
	/// テクスチャ作成コールバック型
	/// ※引数に指定されたファイル名からテクスチャを作成する型
	/// </summary>
	using TextureCallback = std::function<TexturePtr(const char*)>;

	/// <summary>
	/// テクスチャ作成コールバックを設定する
	/// </summary>
	/// <param name="callback">設定するコールバックオブジェクト</param>
	void SetTextureCallback(const TextureCallback& callback)
	{
		textureCallback = callback;
	}

	// OBJファイルを読み込む
	StaticMeshPtr LoadOBJ(const char* filename);

	// 関節データ
	struct SKELETON
	{
		// 関節をつなぐ骨
		struct BONE
		{
			vec3 start;      // 関節の始点
			vec3 end;        // 関節の終点
			int parent = -1; // 親関節の番号
			float influenceRadius = 1; // 関節の影響範囲
		};
		std::vector<BONE> bones; // 関節リスト
	};

	// OBJファイルを読み込み関節を設定する
	SkeletalMeshPtr LoadOBJ(const char* filename, const SKELETON& skeleton);

	// 頂点データを追加する
	void AddVertexData(const void* vertices, size_t vertexBytes,
		const uint16_t* indices, size_t indexBytes,
		size_t stride = sizeof(VERTEX), GLenum mode = GL_TRIANGLES);

	// 既存の描画パラメータとテクスチャから新しいスタティックメッシュを作成する
	StaticMeshPtr CreateStaticMesh(const char* name,
		const DRAW_PARAMS& params, const TexturePtr& texture_color_base);

	// 全ての頂点データを削除する
	void Clear();

	// スタティックメッシュを取得する
	StaticMeshPtr GetStaticMesh(const char* name) const;

	// スケルタルメッシュを取得する
	SkeletalMeshPtr GetSkeletalMesh(const char* name) const;

	/// <summary>
	/// 描画パラメータを取得する
	/// </summary>
	/// <param name="index">配列番号</param>
	const DRAW_PARAMS& GetDrawParams(size_t index) const
	{
		return drawParamsList[index];
	}

	/// <summary>
	/// 描画パラメータの数を取得する
	/// </summary>
	size_t GetDrawParamsCount() const { return drawParamsList.size(); }

	/// <summary>
	/// VAOを取得する
	/// </summary>
	VertexArrayObjectPtr GetVAO() const { return vao; }
	/// <summary>
	/// スケルタルメッシュ用のVAOを取得する
	/// </summary>
	VertexArrayObjectPtr GetVAOSkeletal() const { return vao_skeletal; }

private:
	// MTLファイルを読み込む
	std::vector<MaterialPtr> LoadMTL(
		const std::string& foldername, const char* filename);

	// メッシュ構築用の中間データ
	struct MESH_DATA
	{
		std::vector<VERTEX> vertices;  // 頂点データ
		std::vector<uint16_t> indices; // インデックスデータ
		std::vector<DRAW_PARAMS> drawParamsList; // 描画パラメータ配列
		MaterialList materials;		   // マテリアル配列
	};
	// OBJファイルをメッシュデータに変換する
	MESH_DATA CreateMeshDataFromOBJ(const char* filename);

	std::unordered_map<std::string, StaticMeshPtr> meshes;
	std::unordered_map<std::string, SkeletalMeshPtr> skeletalMeshes;
	std::vector<DRAW_PARAMS> drawParamsList; // 描画パラメータ配列
	VertexArrayObjectPtr vao;				 // 頂点アトリビュート配列
	VertexArrayObjectPtr vao_skeletal;		 // スケルタルメッシュ用の頂点配列
	BufferObjectPtr buffer;					 // 頂点データおよびインデックスデータ
	size_t bytes_used = 0;					 // バッファの使用済み容量(バイト)
	TextureCallback textureCallback;		 // テクスチャ作成コールバック
	std::shared_ptr<class MikkTSpace> mikkTSpace;
};

// 欠けている法線を補う
void FillMissingNormals(VERTEX* vertices, size_t vertexCount,
	const uint16_t* indices, size_t indexCount);

#endif // !MESH_H_INCLUDED