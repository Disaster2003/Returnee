/**
* @file Mesh.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "Mesh.h"
#include "Debug.h"
#include "../MikkTSpace/mikktspace.h"
#include <numeric>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdio.h>

/// <summary>
/// MikkTSpaceのインターフェイス実装
/// </summary>
class MikkTSpace
{
public:
	// MikkTSpace用のユーザーデータ
	struct UserData
	{
		std::vector<uint16_t>& indices;
		std::vector<VERTEX>& vertices;
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	MikkTSpace()
	{
		// モデルの総ポリゴン数を返す
		interface.m_getNumFaces = [](const SMikkTSpaceContext* pContext)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				return static_cast<int>(p->indices.size() / 3);
			};

		// ポリゴンの頂点数を返す
		interface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext* pContext,
			int iFace)
			{
				return 3;
			};

		// 頂点の座標を返す
		interface.m_getPosition = [](const SMikkTSpaceContext* pContext,
			float fvPosOut[], int iFace, int iVert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				std::copy_n(&p->vertices[index].position.x, 3, fvPosOut);
			};

		// 頂点の法線を返す
		interface.m_getNormal = [](const SMikkTSpaceContext* pContext,
			float fvNormOut[], int iFace, int iVert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				std::copy_n(&p->vertices[index].normal.x, 3, fvNormOut);
			};

		// 頂点のテクスチャ座標を返す
		interface.m_getTexCoord = [](const SMikkTSpaceContext* pContext,
			float fvTexcOut[], int iFace, int iVert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				std::copy_n(&p->vertices[index].texcoord.x, 2, fvTexcOut);
			};

		// 「タンジェントベクトル」と「バイタンジェントベクトルの向き」を受け取る
		interface.m_setTSpaceBasic = [](const SMikkTSpaceContext* pContext,
			const float tangent[], float sign, int face, int vert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[face * 3 + vert];
				p->vertices[index].tangent =
					vec4(tangent[0], tangent[1], tangent[2], sign);
			};

		// 使わないのでnullptrを設定
		interface.m_setTSpace = nullptr;
	}

	// デフォルトデストラクタ
	virtual ~MikkTSpace() = default;

	// メンバ変数
	SMikkTSpaceInterface interface;
};

/// <summary>
/// メッシュを描画する
/// </summary>
void Draw
(
	const MESH_STATIC& mesh,
	ProgramObject& program,
	const MaterialList& materials
)
{
	//カラーパラメータを取得する
	vec4 objectColor;
	bool hasUniformColor = false; // カラー系ユニフォーム変数がtrue = 有る、false = 無い
	if (program)
	{
		hasUniformColor = glGetUniformLocation(program, "color") >= 0;
		if (hasUniformColor)
		{
			glGetUniformfv(program, 100, &objectColor.x);
		}
	}
	if (hasUniformColor)
	{
		Draw(mesh.drawParamsList, program, materials, &objectColor);
	}
	else
	{
		Draw(mesh.drawParamsList, program, materials, nullptr);
	}
}


/// <summary>
/// 描画パラメータ配列を描画する
/// </summary>
void Draw
(
	const std::vector<DRAW_PARAMS>& drawParamsList,
	ProgramObject& program,
	const MaterialList& materials,
	const vec4* objectColor
)
{
	const GLint locRoughnessAndMetallic = program.RoughnessAndMetallicLocation();

	for (const auto& e : drawParamsList)
	{
		// マテリアルを設定する
		if (e.materialNo >= 0 && e.materialNo < materials.size())
		{
			const MATERIAL & material = *materials[e.materialNo];
			if (objectColor)
			{
				const vec4 color =
					*objectColor * material.baseColor;
				glProgramUniform4fv(program, 100, 1, &color.x);
				glProgramUniform4f(program, 101, material.emission.x, material.emission.y, material.emission.z,
					static_cast<bool>(material.texEmission));
			}
			// ラフネスとメタリックを設定
			if (locRoughnessAndMetallic >= 0)
			{
				glProgramUniform2f(program, locRoughnessAndMetallic,
					material.roughness, material.metallic);
			}
			if (material.texBaseColor)
			{
				const GLuint tex = *material.texBaseColor;
				glBindTextures(0, 1, &tex);
			}
			if (material.texEmission)
			{
				const GLuint tex = *material.texEmission;
				glBindTextures(1, 1, &tex);
			}
			else
			{
				glBindTextures(1, 1, nullptr); // テクスチャ1を未設定にする
			}
			if (material.texNormal)
			{
				const GLuint tex = *material.texNormal;
				glBindTextures(3, 1, &tex);
			}
			else
			{
				glBindTextures(3, 1, nullptr);
			}
		}

		glDrawElementsBaseVertex
		(e.mode, e.count, GL_UNSIGNED_SHORT, e.indices, e.baseVertex);
	}
}

/// <summary>
/// MTLファイルを読み込む
/// </summary>
/// <param name="foldername">OBJファイルのあるフォルダ名</param>
/// <param name="filename">MTLファイル名</param>
/// <returns>MTLファイルに含まれるマテリアルの配列</returns>
std::vector<MaterialPtr> MeshBuffer::LoadMTL
(
	const std::string& foldername,
	const char* filename
)
{
	// MTLファイルを開く
	const std::string fullpath = foldername + filename;
	std::ifstream file(fullpath);
	if (!file)
	{
		LOG_ERROR("%sを開けません", fullpath.c_str());
		return{};
	}

	// MTLファイルを解析する
	std::vector<MaterialPtr> materials;
	MaterialPtr pMaterial;
	vec3 specularColor = vec3(1); // スペキュラ色
	float specularPower = 12;     // スペキュラ係数
	while (!file.eof())
	{
		std::string line;
		std::getline(file, line);
		const char* p = line.c_str();

		// マテリアル定義の読み取りを試みる
		char name[1000] = { 0 };
		if (sscanf(line.data(), " newmtl %999s", name) == 1)
		{
			if (pMaterial)
			{
				// スペキュラの2つのパラメータからラフネスを計算する
				specularPower *=
					std::max(std::max(specularColor.x, specularColor.y), specularColor.z);
				pMaterial->roughness = std::clamp(1 - log2(specularPower) / 12, 0.0001f, 1.0f);

				// スペキュラパラメータを初期値に戻す
				specularColor = vec3(1);
				specularPower = 12;

				// テクスチャが設定されていないマテリアルの場合、white.tgaを設定しておく
				if (!pMaterial->texBaseColor)
				{
					pMaterial->texBaseColor = textureCallback("Res/white.tga");
				}
			}
			pMaterial = std::make_shared<MATERIAL>();
			pMaterial->name = name;
			materials.push_back(pMaterial);
			continue;
		}

		// マテリアルが定義されていない場合は行を無視する
		if (!pMaterial)
		{
			continue;
		}

		// 基本色の読み取りを試みる
		if (sscanf(line.data(), " Kd %f %f %f",
			&pMaterial->baseColor.x, &pMaterial->baseColor.y, &pMaterial->baseColor.z) == 3)
		{
			continue;
		}

		// 不透明度の読み取りを試みる
		if (sscanf(line.data(), " d %f", &pMaterial->baseColor.w) == 1)
		{
			continue;
		}

		// 基本色テクスチャ名の読み取りを試みる
		char textureName[1000] = { 0 };
		if (sscanf(line.data(), " map_Kd %999s", &textureName) == 1)
		{
			pMaterial->texBaseColor = textureCallback((foldername + textureName).c_str());

			continue;
		}

		// 発光色の読み取りを試みる
		if (sscanf(line.data(), " Ke %f %f %f",
			&pMaterial->emission.x, &pMaterial->emission.y,
			&pMaterial->emission.z) == 3)
		{
			continue;
		}

		// 発光色テクスチャ名の読み取りを試みる
		if (sscanf(line.data(), " map_Ke %999s", &textureName) == 1)
		{
			const std::string filename = foldername + textureName;
			if (std::filesystem::exists(filename))
			{
				pMaterial->texEmission =
					textureCallback(filename.c_str());
			}
			else
			{
				LOG_WARNING("%sを開けません", filename.c_str());
			}

			continue;
		}

		// 法線テクスチャ名の読み取りを試みる
		if (sscanf(line.data(), " map_%*[Bb]ump %[^\n]s", &textureName) == 1 ||
			sscanf(line.data(), " %*[Bb]ump %[^\n]s", &textureName) == 1)
		{
			const std::string filename = foldername + textureName;
			if (std::filesystem::exists(filename))
			{
				pMaterial->texNormal = textureCallback(filename.c_str());
			}
			else
			{
				LOG_WARNING("%sを開けません", filename.c_str());
			}
			continue;
		} // map_bump

		// スペキュラ色の読み取りを試みる
		if (sscanf(line.data(), " Ks %f %f %f",
			&specularColor.x, &specularColor.y, &specularColor.z) == 3)
		{
			continue;
		}

		// スペキュラ係数の読み取りを試みる
		if (sscanf(line.data(), " Ns %f", &pMaterial->roughness) == 1)
		{
			continue;
		}

		// メタリックの読み取りを試みる
		if (sscanf(line.data(), " Pm %f", &pMaterial->metallic) == 1)
		{
			continue;
		}
	}

	// 最後のマテリアルのスペキュラパラメータを設定
	if (pMaterial)
	{
		// スペキュラの2つのパラメータからラフネスを計算する
		specularPower *=
			std::max(std::max(specularColor.x, specularColor.y), specularColor.z);
		pMaterial->roughness = std::clamp(1 - log2(specularPower) / 12, 0.0001f, 1.0f);

		// テクスチャが設定されていないマテリアルの場合、white.tgaを設定しておく
		if (!pMaterial->texBaseColor)
		{
			pMaterial->texBaseColor = textureCallback("Res/white.tga");
		}
	}

	// 読み込んだマテリアルの配列を返す
	return materials;
}

/// <summary>
/// コンストラクタ
/// </summary>
MeshBuffer::MeshBuffer(size_t buffer_size)
{
	// バッファオブジェクトを作成する
	buffer = BufferObject::Create(buffer_size, nullptr, GL_DYNAMIC_STORAGE_BIT);

	// VAOを作成する
	vao = VertexArrayObject::Create();

	// VBOとIBOをVAOにバインド
	glBindVertexArray(*vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *buffer);
	glBindBuffer(GL_ARRAY_BUFFER, *buffer);

	// 頂点アトリビュートを設定する
	vao->SetAttribute(0, 3, sizeof(VERTEX), offsetof(VERTEX, position));
	vao->SetAttribute(1, 2, sizeof(VERTEX), offsetof(VERTEX, texcoord));
	vao->SetAttribute(2, 3, sizeof(VERTEX), offsetof(VERTEX, normal));
	vao->SetAttribute(3, 4, sizeof(VERTEX), offsetof(VERTEX, tangent));

	// スケルタルメッシュ用のVAOを作成する
	vao_skeletal = VertexArrayObject::Create();
	glBindVertexArray(*vao_skeletal);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *buffer);
	glBindBuffer(GL_ARRAY_BUFFER, *buffer);
	vao_skeletal->SetAttribute(0, 3, sizeof(VERTEX_SKELETAL),
		offsetof(VERTEX_SKELETAL, position));
	vao_skeletal->SetAttribute(1, 2, sizeof(VERTEX_SKELETAL),
		offsetof(VERTEX_SKELETAL, texcoord));
	vao_skeletal->SetAttribute(2, 3, sizeof(VERTEX_SKELETAL),
		offsetof(VERTEX_SKELETAL, normal));
	vao_skeletal->SetAttribute(3, 4, sizeof(VERTEX_SKELETAL),
		offsetof(VERTEX_SKELETAL, tangent));
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 4, GL_UNSIGNED_SHORT, sizeof(VERTEX_SKELETAL),
		reinterpret_cast<const void*>(offsetof(VERTEX_SKELETAL, joints)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(VERTEX_SKELETAL),
		reinterpret_cast<const void*>(offsetof(VERTEX_SKELETAL, weights)));

	// 誤った操作が行われないようにバインドを解除する
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// スタティックメッシュの容量を予約する
	meshes.reserve(100);

	// 描画パラメータの容量を予約
	drawParamsList.reserve(100);

	// タンジェント空間計算用のオブジェクトを作成する
	mikkTSpace = std::make_shared<MikkTSpace>();
	if (!mikkTSpace)
	{
		LOG_ERROR("MikkTSpaceの作成に失敗");
	}
}

/// <summary>
/// ObJファイルを読み込む
/// </summary>
/// <param name="filename">OBJファイル名</param>
/// <returns>filenameから作成したメッシュ</returns>
StaticMeshPtr MeshBuffer::LoadOBJ(const char* filename)
{
	// 以前に読み込んだファイルなら、作成済みのメッシュを返す
	auto itr = meshes.find(filename);
	if (itr != meshes.end())
	{
		return itr->second;
	}

	// OBJファイルからメッシュデータを作成する
	MESH_DATA meshData = CreateMeshDataFromOBJ(filename);
	if (meshData.vertices.empty())
	{
		return nullptr; // 読み込み失敗
	}


	// 変換したデータをバッファに追加する
	AddVertexData
	(
		meshData.vertices.data(),
		meshData.vertices.size() * sizeof(VERTEX),
		meshData.indices.data(),
		meshData.indices.size() * sizeof(uint16_t)
	);

	//メッシュを作成する
	auto pMesh = std::make_shared<MESH_STATIC>();

	pMesh->drawParamsList.swap(meshData.drawParamsList);
	pMesh->materials.swap(meshData.materials);
	pMesh->name = filename;
	meshes.emplace(filename, pMesh);

	// AddVertexDataでコピーしたデータの位置を、描画パラメータに反映する
	const intptr_t baseIndexOffset =
		reinterpret_cast<intptr_t>(drawParamsList.back().indices);
	const GLint baseVertex = drawParamsList.back().baseVertex;
	for (auto& e : pMesh->drawParamsList)
	{
		e.baseVertex = baseVertex;
		e.indices = reinterpret_cast<const void*>(
			baseIndexOffset + reinterpret_cast<intptr_t>(e.indices));
	}

	LOG("%sを読み込みました(頂点数=%d,インデックス数=%d)", filename, meshData.vertices.size(), meshData.indices.size());

	// 作成したメッシュを返す
	return pMesh;
}

/// <summary>
/// OBJファイルを読み込み関節を設定する
/// </summary>
/// <param name="filename">OBJファイル名</param>
/// <param name="skeleton">関節データ</param>
/// <returns>filenameから作成した関節付きメッシュ</returns>
SkeletalMeshPtr MeshBuffer::LoadOBJ
(
	const char* filename,
	const SKELETON& skeleton
)
{
	// 以前に読み込んだファイルなら、作成済みのメッシュを返す
	{
		auto itr = skeletalMeshes.find(filename);
		if (itr != skeletalMeshes.end())
		{
			return itr->second;
		}
	}

	// OBJファイルからメッシュデータを作成する
	MESH_DATA meshData = CreateMeshDataFromOBJ(filename);
	if (meshData.vertices.empty())
	{
		return nullptr; // 読み込み失敗
	}

	// Vertexの共通部分をSkeletaVertexにコピー
	std::vector<VERTEX_SKELETAL> skeletalVertices(meshData.vertices.size());
	for (int i = 0; i < meshData.vertices.size(); ++i)
	{
		skeletalVertices[i].position = meshData.vertices[i].position;
		skeletalVertices[i].texcoord = meshData.vertices[i].texcoord;
		skeletalVertices[i].normal = meshData.vertices[i].normal;
		skeletalVertices[i].tangent = meshData.vertices[i].tangent;
	}

	// スケルトンを参照して関節番号を設定する
	for (auto& vertex : skeletalVertices)
	{
		std::fill_n(vertex.joints, 4, 0);
		std::fill_n(vertex.weights, 4, 0);

		// 全てのボーンとの最短距離を求める
		struct Distance { int index; float distance; };
		std::vector<Distance> distanceList(skeleton.bones.size());
		for (int i = 0; i < skeleton.bones.size(); ++i)
		{
			// ボーンの始点と終点と楕円の焦点、頂点を楕円上の点として短軸の長さを求める
			const vec3& P = vertex.position;
			const vec3& F0 = skeleton.bones[i].start;
			const vec3& F1 = skeleton.bones[i].end;
			const float a = (length(P - F0) + length(P - F1)) * 0.5f;
			const vec3 c = (F0 - F1) * 0.5f;
			const float b = sqrt(a * a - dot(c, c));
			distanceList[i].distance = b;
			distanceList[i].index = i; // 関節番号
		}

		// 距離の短い順に整列させる
		std::sort
		(
			distanceList.begin(),
			distanceList.end(),
			[](const Distance& lhs, const Distance& rhs)
			{
				return lhs.distance < rhs.distance;
			}
		);

		// 短いほうから最大4個残す(4個未満ならゼロを追加して4個にする)
		distanceList.resize(4, Distance{ 0, 0 });

		// 距離を影響度に変換する
		float total = 0; // 影響度の合計
		for (auto& e : distanceList)
		{
			// 影響範囲を超えさせないための係数を計算する(ライトの範囲制限と同じ式)
			const float radius = skeleton.bones[e.index].influenceRadius;
			const float f = std::clamp(1 - pow(e.distance / radius, 4.0f), 0.0f, 1.0f);

			// 逆２乗の法則で影響度を計算する
			e.distance = f * f / (e.distance * e.distance + 0.0001f);
			total += e.distance;
		}

		// 影響度の合計が1になるように正規化する
		if (total > 0)
		{
			for (auto& e : distanceList)
			{
				e.distance /= total;
			}
		}
		else
		{
			// すべてのdistanceが0の(どのボーンからも遠すぎる)場合、
			// 一番近いボーンのみ影響するように設定する
			distanceList[0].distance = 1;
		}

		// 頂点に関節データを設定する
		for (int i = 0; i < 4; ++i)
		{
			vertex.weights[i] = static_cast<uint16_t>(distanceList[i].distance * 0xffff);
			vertex.joints[i] = static_cast<uint16_t>(distanceList[i].index);
		}
	}	// for vertex

	// 変換したデータをプリミティブバッファに追加する
	AddVertexData
	(
		skeletalVertices.data(),
		skeletalVertices.size() * sizeof(VERTEX_SKELETAL),
		meshData.indices.data(),
		meshData.indices.size() * sizeof(uint16_t),
		sizeof(VERTEX_SKELETAL)
	);

	// スケルタルメッシュを作成する
	auto pMesh = std::make_shared<MESH_SKELETAL>();
	pMesh->drawParamsList.swap(meshData.drawParamsList);
	pMesh->materials.swap(meshData.materials);
	pMesh->name = filename;
	skeletalMeshes.emplace(filename, pMesh);

	// AddVertexDataでコピーしたデータの位置を反映する
	const intptr_t baseIndexOffset =
		reinterpret_cast<intptr_t>(drawParamsList.back().indices);
	const GLint baseVertex = drawParamsList.back().baseVertex;
	for (auto& e : pMesh->drawParamsList)
	{
		e.baseVertex = baseVertex;
		e.indices = reinterpret_cast<void*>(
			baseIndexOffset + reinterpret_cast<intptr_t>(e.indices));
	}

	// ジョイント設定
	pMesh->joints.resize(skeleton.bones.size());
	for (int i = 0; i < pMesh->joints.size(); ++i)
	{
		// ボーンの始点座標を関節とみなして行列を計算する
		MESH_SKELETAL::JOINT& joint = pMesh->joints[i];
		const SKELETON::BONE& bone = skeleton.bones[i];
		joint.bindPose = GetTransformMatrix(vec3(1), vec3(0), bone.start);
		joint.inverseBindPose = GetTransformMatrix(vec3(1), vec3(0), -bone.start);

		// 親関節の番号を設定する
		joint.parent = bone.parent;
	} // for i

	LOG("%sを読み込みました(頂点数=%d, インデックス数=%d)",
		filename, meshData.vertices.size(), meshData.indices.size());

	// 作成したメッシュを返す
	return pMesh;
}

/// <summary>
/// OBJファイルをメッシュデータに変換する
/// </summary>
/// <param name="filename">OBJファイル名</param>
/// <returns>filenameから作成したメッシュデータ</returns>
MeshBuffer::MESH_DATA MeshBuffer::CreateMeshDataFromOBJ(const char* filename)
{
	// OBJファイルを開く
	std::ifstream file(filename);
	if (!file)
	{
		LOG_ERROR("%sを開けません", filename);
		return {};
	}

	// フォルダ名を取得する
	std::string foldername(filename);
	{
		const size_t p = foldername.find_last_of("￥￥/");
		if (p != std::string::npos)
		{
			foldername.resize(p + 1);
		}
	}

	// OBJファイルを解析して、頂点データとインデックスデータを読み込む
	std::vector<vec3> positions;
	std::vector<vec2> texcoords;
	std::vector<vec3> normals;
	struct IndexSet { int v, vt, vn; };
	std::vector<IndexSet> faceIndexSet;

	positions.reserve(20'000);
	texcoords.reserve(20'000);
	normals.reserve(20'000);
	faceIndexSet.reserve(20'000 * 3);

	// マテリアル
	std::vector<MaterialPtr> materials;
	materials.reserve(100);

	// マテリアルの使用範囲
	struct USE_MATERIAL
	{
		std::string name;	// マテリアル名
		size_t startOffset;	// 割り当て範囲の先頭位置
	};
	std::vector<USE_MATERIAL> usemtls;
	usemtls.reserve(100);

	// 仮データを追加する(マテリアル指定がないファイル対策)
	usemtls.push_back({ std::string(),0 });

	while (!file.eof())
	{
		std::string line;
		std::getline(file, line);
		const char* p = line.c_str();

		// 頂点座標の読み取りを試みる
		vec3 v;
		if (sscanf(p, "v %f %f %f", &v.x, &v.y, &v.z) == 3)
		{
			positions.push_back(v);
			continue;
		}

		// テクスチャ座標の読み取りを試みる
		vec2 vt;
		if (sscanf(p, "vt %f %f", &vt.x, &vt.y) == 2)
		{
			texcoords.push_back(vt);
			continue;
		}

		// 法線の読み取りを試みる
		vec3 vn;
		if (sscanf(p, "vn %f %f %f", &vn.x, &vn.y, &vn.z) == 3)
		{
			vn = normalize(vn); // 正規化されていないことがあるので正規化する
			normals.push_back(vn);
			continue;
		}

		// インデックスデータの読み取りを試みる
		IndexSet f0, f1, f2;
		int readByte;
		// 頂点座標 + テクスチャ座標 + 法線
		if (sscanf(p, "f %u/%u/%u %u/%u/%u%n", &f0.v, &f0.vt, &f0.vn, &f1.v, &f1.vt, &f1.vn, &readByte) == 6)
		{
			p += readByte; // 読み取り位置を更新
			for (;;)
			{
				if (sscanf(p, "%u/%u/%u%n", &f2.v, &f2.vt, &f2.vn, &readByte) != 3)
				{
					break;
				}
				p += readByte; // 読み取り位置を更新する
				faceIndexSet.push_back(f0);
				faceIndexSet.push_back(f1);
				faceIndexSet.push_back(f2);
				f1 = f2;	   // 次の三角形のためにデータを移動する
			}
			continue;
		}

		// 頂点座標 + テクスチャ座標
		if (sscanf(p, "f %u/%u %u/%u%n", &f0.v, &f0.vt, &f1.v, &f1.vt, &readByte) == 4)
		{
			f0.vn = f1.vn = 0; // 法線なし
			p += readByte;	   // 読み取り位置を更新する
			for (;;)
			{
				if (sscanf(p, "%u/%u%n", &f2.v, &f2.vt, &readByte) != 2)
				{
					break;
				}
				f2.vn = 0;	   // 法線なし
				p += readByte; // 読み取り位置を更新する
				faceIndexSet.push_back(f0);
				faceIndexSet.push_back(f1);
				faceIndexSet.push_back(f2);
				f1 = f2;	   // 次の三角形のためにデータを移動する
			}
			continue;
		}

		// MTLファイルの読み取りを試みる
		char mtlFilename[1000];
		if (sscanf(line.data(), " mtllib %999s", mtlFilename) == 1)
		{
			const auto tmp = LoadMTL(foldername, mtlFilename);
			materials.insert(materials.end(), tmp.begin(), tmp.end());
			continue;
		}

		// 使用マテリアル名の読み取りを試みる
		char mtlName[1000];
		if (sscanf(line.data(), " usemtl %999s", mtlName) == 1)
		{
			usemtls.push_back({ mtlName,faceIndexSet.size() });
			continue;
		}
	} // while eof

	// 末尾に番兵を追加する
	usemtls.push_back({ std::string(),faceIndexSet.size() });

	// OBJファイルのf構文とOpenGLの頂点インデックス配列の対応表
	std::unordered_map<uint64_t, uint16_t> indexMap;
	indexMap.reserve(10'000);

	// 読み込んだデータを、OpenGLで使えるデータに変換する
	MESH_DATA meshData;
	std::vector<VERTEX>& vertices = meshData.vertices;
	vertices.reserve(faceIndexSet.size());
	std::vector<uint16_t>& indices = meshData.indices;
	indices.reserve(faceIndexSet.size());
	for (const auto& e : faceIndexSet)
	{
		// f構文の値を64ビットの「キー」に変換する
		const uint64_t key = static_cast<uint64_t>(e.v) +
			(static_cast<uint64_t>(e.vt) << 20) + (static_cast<uint64_t>(e.vn) << 40);

		// 対応表からキーに一致するデータを検索する
		const auto itr = indexMap.find(key);
		if (itr != indexMap.end())
		{
			// 対応表にあるので既存の頂点インデックスを使う
			indices.push_back(itr->second);
		}
		else
		{
			// 対応表にないので新しい頂点データを作成し、頂点配列に追加する
			VERTEX v;
			v.position = positions[e.v - 1];
			v.texcoord = texcoords[e.vt - 1];
			// 法線が設定されていない場合は0を設定する(後で計算)
			(e.vn == 0) ? v.normal = { 0,0,0 } : v.normal = normals[e.vn - 1];
			vertices.push_back(v);

			// 新しい頂点データのインデックスを、頂点インデックス配列に追加する
			const uint16_t index = static_cast<uint16_t>(vertices.size() - 1);
			indices.push_back(index);

			// キーと頂点インデックスのペアを対応表に追加する
			indexMap.emplace(key, index);
		}
	}

	// 設定されていない法線を補う
	FillMissingNormals(vertices.data(), vertices.size(), indices.data(), indices.size());

	// タンジェントベクトルを計算
	if (mikkTSpace)
	{
		// MikkTSpaceライブラリでタンジェントを計算
		MikkTSpace::UserData userData = { indices, vertices };
		SMikkTSpaceContext context = { &mikkTSpace->interface, &userData };
		genTangSpaceDefault(&context);
	}

	// データの位置を初期化する
	const void* indexOffset = 0;
	const GLint baseVertex = 0;

	// マテリアルに対応した描画パラメータを作成する
	// 仮データと番兵以外のマテリアルがある場合、仮データを飛ばす
	size_t i = 0;
	if (usemtls.size() > 2)
	{
		i = 1; // 仮データと番兵以外のマテリアルがある場合、仮データを飛ばす
	}
	for (; i < usemtls.size() - 1; ++i)
	{
		const USE_MATERIAL& cur = usemtls[i];	   // 使用中のマテリアル
		const USE_MATERIAL& next = usemtls[i + 1]; // 次のマテリアル
		if (next.startOffset == cur.startOffset)
		{
			continue; // インデックスデータがない場合は飛ばす
		}

		// 描画パラメータを作成
		DRAW_PARAMS params;
		params.mode = GL_TRIANGLES;
		params.count = static_cast<GLsizei>(next.startOffset - cur.startOffset);
		params.indices = indexOffset;
		params.baseVertex = baseVertex;
		params.materialNo = 0; // デフォルト値を設定
		for (int i = 0; i < materials.size(); ++i)
		{
			if (materials[i]->name == cur.name)
			{
				// 名前の一致するマテリアルを設定する
				params.materialNo = i;
				break;
			}
		}
		meshData.drawParamsList.push_back(params);

		// インデックスオフセットを変更する
		indexOffset
			= reinterpret_cast<void*>(reinterpret_cast<size_t>(indexOffset) + sizeof(uint16_t) * params.count);
	}

	// マテリアル配列が空の場合、デフォルトマテリアルを追加する
	if (materials.empty())
	{
		meshData.materials.push_back(std::make_shared<MATERIAL>());
	}
	else
	{
		meshData.materials.assign(materials.begin(), materials.end());
	}

	return meshData;
}

/// <summary>
/// 頂点データを追加する
/// </summary>
/// <param name="vertices">GPUメモリにコピーする頂点データ配列</param>
/// <param name="vertexBytes">verticesのバイト数</param>
/// <param name="indices">GPUメモリにコピーするインデックスデータ配列</param>
/// <param name="indexBytes">indicesのバイト数</param>
/// <param name="stride">頂点データの間隔(バイト数)</param>
/// <param name="mode">プリミティブの種類</param>
void MeshBuffer::AddVertexData
(
	const void* vertices,
	size_t vertexBytes,
	const uint16_t* indices,
	size_t indexBytes,
	size_t stride,
	GLenum mode
)
{
	// 空き容量が足りていることを確認する
	// 必要なバイト数が空きバイト数より大きい場合は追加できない
	const size_t totalBytes = vertexBytes + indexBytes;
	const size_t freeBytes = buffer->GetSize() - bytes_used;
	if (totalBytes > freeBytes)
	{
		LOG_ERROR
		(
			"容量が足りません(要求サイズ=%d,使用量/最大容量=%d/%d)",
			vertexBytes + indexBytes,
			bytes_used,
			buffer->GetSize()
		);
		return;
	}

	// 頂点データをGPUメモリにコピー
	GLuint tmp[2];
	glCreateBuffers(2, tmp);
	glNamedBufferStorage(tmp[0], vertexBytes, vertices, 0);
	glNamedBufferStorage(tmp[1], indexBytes, indices, 0);
	glCopyNamedBufferSubData(tmp[0], *buffer, 0, bytes_used, vertexBytes);
	glCopyNamedBufferSubData(tmp[1], *buffer, 0, bytes_used + vertexBytes, indexBytes);
	glDeleteBuffers(2, tmp);

	// 追加した図形の描画パラメータを作成する
	DRAW_PARAMS newParams;
	newParams.mode = mode;
	newParams.count = static_cast<GLsizei>(indexBytes / sizeof(uint16_t));
	// インデックスデータ位置(バイト数で指定)は頂点データの直後
	newParams.indices = reinterpret_cast<void*>(bytes_used + vertexBytes);
	// 頂点データの位置は頂点データ数で指定する
	newParams.baseVertex = static_cast<GLint>(bytes_used / stride);
	drawParamsList.push_back(newParams);

	// 次のデータ格納開始位置を計算する
	constexpr size_t a = std::lcm(sizeof(VERTEX_SKELETAL),
		std::lcm(sizeof(uint16_t), sizeof(VERTEX)));
	bytes_used += ((totalBytes + a - 1) / a) * a;
}

/// <summary>
/// 既存の描画パラメータとテクスチャから新しいスタティックメッシュを作成する
/// </summary>
/// <param name="name">メッシュ名</param>
/// <param name="params">メッシュの元になる描画パラメータ</param>
/// <param name="texture_color_base">メッシュに設定するベースカラーテクスチャ</param>
/// <returns>作成したスタティックメッシュ</returns>
StaticMeshPtr MeshBuffer::CreateStaticMesh
(
	const char* name,
	const DRAW_PARAMS& params,
	const TexturePtr& texture_color_base
)
{
	auto p = std::make_shared<MESH_STATIC>();
	p->name = name;
	p->drawParamsList.push_back(params);
	p->drawParamsList[0].materialNo = 0;
	p->materials.push_back(std::make_shared<MATERIAL>());
	if (texture_color_base)
	{
		p->materials[0]->texBaseColor = texture_color_base;
	}
	meshes.emplace(name, p);
	return p;
}

/// <summary>
/// 全ての頂点データを削除する
/// </summary>
void MeshBuffer::Clear()
{
	bytes_used = 0;
	meshes.clear();
	drawParamsList.clear();
}

/// <summary>
/// 欠けている法線を補う
/// </summary>
/// <param name="vertices">頂点配列</param>
/// <param name="vertexCount">頂点配列の要素数</param>
/// <param name="indices">インデックス配列</param>
/// <param name="indexCount">インデックス配列の要素数</param>
void FillMissingNormals
(
	VERTEX* vertices,
	size_t vertexCount,
	const uint16_t* indices,
	size_t indexCount
)
{
	// 法線が設定されていない頂点を見つける
	std::vector<bool> missingNomals(vertexCount, false);
	for (int i = 0; i < vertexCount; ++i)
	{
		// 法線の長さが0の場合を「設定されていない」とみなす
		const vec3& n = vertices[i].normal;
		if (n.x == 0 && n.y == 0 && n.z == 0)
		{
			missingNomals[i] = true;
		}
	}

	// 法線を計算する
	for (int i = 0; i < indexCount; i += 3)
	{
		// 面を構成する2辺a,bを求める
		const int i0 = indices[i + 0];
		const int i1 = indices[i + 1];
		const int i2 = indices[i + 2];
		const vec3& v0 = vertices[i0].position;
		const vec3& v1 = vertices[i1].position;
		const vec3& v2 = vertices[i2].position;
		const vec3 a = { v1.x - v0.x,v1.y - v0.y,v1.z - v0.z };
		const vec3 b = { v2.x - v0.x,v2.y - v0.y,v2.z - v0.z };

		// 外積によってaとbに垂直なベクトル(法線)を求める
		const float cx = a.y * b.z - a.z * b.y;
		const float cy = a.z * b.x - a.x * b.z;
		const float cz = a.x * b.y - a.y * b.x;

		// 法線を正規化して単位ベクトルにする
		const float l = sqrt(cx * cx + cy * cy + cz * cz);
		const vec3 normal = { cx / l,cy / l,cz / l };

		// 法線が設定されていない頂点にだけ法線を加算する
		if (missingNomals[i0])
		{
			vertices[i0].normal += normal;
		}
		if (missingNomals[i1])
		{
			vertices[i1].normal += normal;
		}
		if (missingNomals[i2])
		{
			vertices[i2].normal += normal;
		}
	}

	// 法線を正規化する
	for (int i = 0; i < vertexCount; ++i)
	{
		if (missingNomals[i])
		{
			vec3& n = vertices[i].normal;
			const float l = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
			n = { n.x / l,n.y / l,n.z / l };
		}
	}
}

/// <summary>
/// スタティックメッシュを取得する
/// </summary>
/// <param name="name">スタティックメッシュの名前</param>
/// <returns>名前がnameと一致するスタティックメッシュ</returns>
StaticMeshPtr MeshBuffer::GetStaticMesh(const char* name) const
{
	auto itr = meshes.find(name);
	if (itr != meshes.end())
	{
		return itr->second;
	}

	return nullptr;
}

/// <summary>
/// スケルタルメッシュを取得する
/// </summary>
/// <param name="name">スケルタルメッシュの名前</param>
/// <returns>名前がnameと一致するスケルタルメッシュ</returns>
SkeletalMeshPtr MeshBuffer::GetSkeletalMesh(const char* name) const
{
	auto itr = skeletalMeshes.find(name);
	if (itr != skeletalMeshes.end())
	{
		return itr->second;
	}

	return nullptr;
}