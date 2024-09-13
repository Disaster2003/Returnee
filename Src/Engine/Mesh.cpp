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
/// MikkTSpace�̃C���^�[�t�F�C�X����
/// </summary>
class MikkTSpace
{
public:
	// MikkTSpace�p�̃��[�U�[�f�[�^
	struct UserData
	{
		std::vector<uint16_t>& indices;
		std::vector<VERTEX>& vertices;
	};

	/// <summary>
	/// �R���X�g���N�^
	/// </summary>
	MikkTSpace()
	{
		// ���f���̑��|���S������Ԃ�
		interface.m_getNumFaces = [](const SMikkTSpaceContext* pContext)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				return static_cast<int>(p->indices.size() / 3);
			};

		// �|���S���̒��_����Ԃ�
		interface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext* pContext,
			int iFace)
			{
				return 3;
			};

		// ���_�̍��W��Ԃ�
		interface.m_getPosition = [](const SMikkTSpaceContext* pContext,
			float fvPosOut[], int iFace, int iVert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				std::copy_n(&p->vertices[index].position.x, 3, fvPosOut);
			};

		// ���_�̖@����Ԃ�
		interface.m_getNormal = [](const SMikkTSpaceContext* pContext,
			float fvNormOut[], int iFace, int iVert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				std::copy_n(&p->vertices[index].normal.x, 3, fvNormOut);
			};

		// ���_�̃e�N�X�`�����W��Ԃ�
		interface.m_getTexCoord = [](const SMikkTSpaceContext* pContext,
			float fvTexcOut[], int iFace, int iVert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[iFace * 3 + iVert];
				std::copy_n(&p->vertices[index].texcoord.x, 2, fvTexcOut);
			};

		// �u�^���W�F���g�x�N�g���v�Ɓu�o�C�^���W�F���g�x�N�g���̌����v���󂯎��
		interface.m_setTSpaceBasic = [](const SMikkTSpaceContext* pContext,
			const float tangent[], float sign, int face, int vert)
			{
				UserData* p = static_cast<UserData*>(pContext->m_pUserData);
				const int index = p->indices[face * 3 + vert];
				p->vertices[index].tangent =
					vec4(tangent[0], tangent[1], tangent[2], sign);
			};

		// �g��Ȃ��̂�nullptr��ݒ�
		interface.m_setTSpace = nullptr;
	}

	// �f�t�H���g�f�X�g���N�^
	virtual ~MikkTSpace() = default;

	// �����o�ϐ�
	SMikkTSpaceInterface interface;
};

/// <summary>
/// ���b�V����`�悷��
/// </summary>
void Draw
(
	const MESH_STATIC& mesh,
	ProgramObject& program,
	const MaterialList& materials
)
{
	//�J���[�p�����[�^���擾����
	vec4 objectColor;
	bool hasUniformColor = false; // �J���[�n���j�t�H�[���ϐ���true = �L��Afalse = ����
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
/// �`��p�����[�^�z���`�悷��
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
		// �}�e���A����ݒ肷��
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
			// ���t�l�X�ƃ��^���b�N��ݒ�
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
				glBindTextures(1, 1, nullptr); // �e�N�X�`��1�𖢐ݒ�ɂ���
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
/// MTL�t�@�C����ǂݍ���
/// </summary>
/// <param name="foldername">OBJ�t�@�C���̂���t�H���_��</param>
/// <param name="filename">MTL�t�@�C����</param>
/// <returns>MTL�t�@�C���Ɋ܂܂��}�e���A���̔z��</returns>
std::vector<MaterialPtr> MeshBuffer::LoadMTL
(
	const std::string& foldername,
	const char* filename
)
{
	// MTL�t�@�C�����J��
	const std::string fullpath = foldername + filename;
	std::ifstream file(fullpath);
	if (!file)
	{
		LOG_ERROR("%s���J���܂���", fullpath.c_str());
		return{};
	}

	// MTL�t�@�C������͂���
	std::vector<MaterialPtr> materials;
	MaterialPtr pMaterial;
	vec3 specularColor = vec3(1); // �X�y�L�����F
	float specularPower = 12;     // �X�y�L�����W��
	while (!file.eof())
	{
		std::string line;
		std::getline(file, line);
		const char* p = line.c_str();

		// �}�e���A����`�̓ǂݎ������݂�
		char name[1000] = { 0 };
		if (sscanf(line.data(), " newmtl %999s", name) == 1)
		{
			if (pMaterial)
			{
				// �X�y�L������2�̃p�����[�^���烉�t�l�X���v�Z����
				specularPower *=
					std::max(std::max(specularColor.x, specularColor.y), specularColor.z);
				pMaterial->roughness = std::clamp(1 - log2(specularPower) / 12, 0.0001f, 1.0f);

				// �X�y�L�����p�����[�^�������l�ɖ߂�
				specularColor = vec3(1);
				specularPower = 12;

				// �e�N�X�`�����ݒ肳��Ă��Ȃ��}�e���A���̏ꍇ�Awhite.tga��ݒ肵�Ă���
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

		// �}�e���A������`����Ă��Ȃ��ꍇ�͍s�𖳎�����
		if (!pMaterial)
		{
			continue;
		}

		// ��{�F�̓ǂݎ������݂�
		if (sscanf(line.data(), " Kd %f %f %f",
			&pMaterial->baseColor.x, &pMaterial->baseColor.y, &pMaterial->baseColor.z) == 3)
		{
			continue;
		}

		// �s�����x�̓ǂݎ������݂�
		if (sscanf(line.data(), " d %f", &pMaterial->baseColor.w) == 1)
		{
			continue;
		}

		// ��{�F�e�N�X�`�����̓ǂݎ������݂�
		char textureName[1000] = { 0 };
		if (sscanf(line.data(), " map_Kd %999s", &textureName) == 1)
		{
			pMaterial->texBaseColor = textureCallback((foldername + textureName).c_str());

			continue;
		}

		// �����F�̓ǂݎ������݂�
		if (sscanf(line.data(), " Ke %f %f %f",
			&pMaterial->emission.x, &pMaterial->emission.y,
			&pMaterial->emission.z) == 3)
		{
			continue;
		}

		// �����F�e�N�X�`�����̓ǂݎ������݂�
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
				LOG_WARNING("%s���J���܂���", filename.c_str());
			}

			continue;
		}

		// �@���e�N�X�`�����̓ǂݎ������݂�
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
				LOG_WARNING("%s���J���܂���", filename.c_str());
			}
			continue;
		} // map_bump

		// �X�y�L�����F�̓ǂݎ������݂�
		if (sscanf(line.data(), " Ks %f %f %f",
			&specularColor.x, &specularColor.y, &specularColor.z) == 3)
		{
			continue;
		}

		// �X�y�L�����W���̓ǂݎ������݂�
		if (sscanf(line.data(), " Ns %f", &pMaterial->roughness) == 1)
		{
			continue;
		}

		// ���^���b�N�̓ǂݎ������݂�
		if (sscanf(line.data(), " Pm %f", &pMaterial->metallic) == 1)
		{
			continue;
		}
	}

	// �Ō�̃}�e���A���̃X�y�L�����p�����[�^��ݒ�
	if (pMaterial)
	{
		// �X�y�L������2�̃p�����[�^���烉�t�l�X���v�Z����
		specularPower *=
			std::max(std::max(specularColor.x, specularColor.y), specularColor.z);
		pMaterial->roughness = std::clamp(1 - log2(specularPower) / 12, 0.0001f, 1.0f);

		// �e�N�X�`�����ݒ肳��Ă��Ȃ��}�e���A���̏ꍇ�Awhite.tga��ݒ肵�Ă���
		if (!pMaterial->texBaseColor)
		{
			pMaterial->texBaseColor = textureCallback("Res/white.tga");
		}
	}

	// �ǂݍ��񂾃}�e���A���̔z���Ԃ�
	return materials;
}

/// <summary>
/// �R���X�g���N�^
/// </summary>
MeshBuffer::MeshBuffer(size_t buffer_size)
{
	// �o�b�t�@�I�u�W�F�N�g���쐬����
	buffer = BufferObject::Create(buffer_size, nullptr, GL_DYNAMIC_STORAGE_BIT);

	// VAO���쐬����
	vao = VertexArrayObject::Create();

	// VBO��IBO��VAO�Ƀo�C���h
	glBindVertexArray(*vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *buffer);
	glBindBuffer(GL_ARRAY_BUFFER, *buffer);

	// ���_�A�g���r���[�g��ݒ肷��
	vao->SetAttribute(0, 3, sizeof(VERTEX), offsetof(VERTEX, position));
	vao->SetAttribute(1, 2, sizeof(VERTEX), offsetof(VERTEX, texcoord));
	vao->SetAttribute(2, 3, sizeof(VERTEX), offsetof(VERTEX, normal));
	vao->SetAttribute(3, 4, sizeof(VERTEX), offsetof(VERTEX, tangent));

	// �X�P���^�����b�V���p��VAO���쐬����
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

	// ��������삪�s���Ȃ��悤�Ƀo�C���h����������
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// �X�^�e�B�b�N���b�V���̗e�ʂ�\�񂷂�
	meshes.reserve(100);

	// �`��p�����[�^�̗e�ʂ�\��
	drawParamsList.reserve(100);

	// �^���W�F���g��Ԍv�Z�p�̃I�u�W�F�N�g���쐬����
	mikkTSpace = std::make_shared<MikkTSpace>();
	if (!mikkTSpace)
	{
		LOG_ERROR("MikkTSpace�̍쐬�Ɏ��s");
	}
}

/// <summary>
/// ObJ�t�@�C����ǂݍ���
/// </summary>
/// <param name="filename">OBJ�t�@�C����</param>
/// <returns>filename����쐬�������b�V��</returns>
StaticMeshPtr MeshBuffer::LoadOBJ(const char* filename)
{
	// �ȑO�ɓǂݍ��񂾃t�@�C���Ȃ�A�쐬�ς݂̃��b�V����Ԃ�
	auto itr = meshes.find(filename);
	if (itr != meshes.end())
	{
		return itr->second;
	}

	// OBJ�t�@�C�����烁�b�V���f�[�^���쐬����
	MESH_DATA meshData = CreateMeshDataFromOBJ(filename);
	if (meshData.vertices.empty())
	{
		return nullptr; // �ǂݍ��ݎ��s
	}


	// �ϊ������f�[�^���o�b�t�@�ɒǉ�����
	AddVertexData
	(
		meshData.vertices.data(),
		meshData.vertices.size() * sizeof(VERTEX),
		meshData.indices.data(),
		meshData.indices.size() * sizeof(uint16_t)
	);

	//���b�V�����쐬����
	auto pMesh = std::make_shared<MESH_STATIC>();

	pMesh->drawParamsList.swap(meshData.drawParamsList);
	pMesh->materials.swap(meshData.materials);
	pMesh->name = filename;
	meshes.emplace(filename, pMesh);

	// AddVertexData�ŃR�s�[�����f�[�^�̈ʒu���A�`��p�����[�^�ɔ��f����
	const intptr_t baseIndexOffset =
		reinterpret_cast<intptr_t>(drawParamsList.back().indices);
	const GLint baseVertex = drawParamsList.back().baseVertex;
	for (auto& e : pMesh->drawParamsList)
	{
		e.baseVertex = baseVertex;
		e.indices = reinterpret_cast<const void*>(
			baseIndexOffset + reinterpret_cast<intptr_t>(e.indices));
	}

	LOG("%s��ǂݍ��݂܂���(���_��=%d,�C���f�b�N�X��=%d)", filename, meshData.vertices.size(), meshData.indices.size());

	// �쐬�������b�V����Ԃ�
	return pMesh;
}

/// <summary>
/// OBJ�t�@�C����ǂݍ��݊֐߂�ݒ肷��
/// </summary>
/// <param name="filename">OBJ�t�@�C����</param>
/// <param name="skeleton">�֐߃f�[�^</param>
/// <returns>filename����쐬�����֐ߕt�����b�V��</returns>
SkeletalMeshPtr MeshBuffer::LoadOBJ
(
	const char* filename,
	const SKELETON& skeleton
)
{
	// �ȑO�ɓǂݍ��񂾃t�@�C���Ȃ�A�쐬�ς݂̃��b�V����Ԃ�
	{
		auto itr = skeletalMeshes.find(filename);
		if (itr != skeletalMeshes.end())
		{
			return itr->second;
		}
	}

	// OBJ�t�@�C�����烁�b�V���f�[�^���쐬����
	MESH_DATA meshData = CreateMeshDataFromOBJ(filename);
	if (meshData.vertices.empty())
	{
		return nullptr; // �ǂݍ��ݎ��s
	}

	// Vertex�̋��ʕ�����SkeletaVertex�ɃR�s�[
	std::vector<VERTEX_SKELETAL> skeletalVertices(meshData.vertices.size());
	for (int i = 0; i < meshData.vertices.size(); ++i)
	{
		skeletalVertices[i].position = meshData.vertices[i].position;
		skeletalVertices[i].texcoord = meshData.vertices[i].texcoord;
		skeletalVertices[i].normal = meshData.vertices[i].normal;
		skeletalVertices[i].tangent = meshData.vertices[i].tangent;
	}

	// �X�P���g�����Q�Ƃ��Ċ֐ߔԍ���ݒ肷��
	for (auto& vertex : skeletalVertices)
	{
		std::fill_n(vertex.joints, 4, 0);
		std::fill_n(vertex.weights, 4, 0);

		// �S�Ẵ{�[���Ƃ̍ŒZ���������߂�
		struct Distance { int index; float distance; };
		std::vector<Distance> distanceList(skeleton.bones.size());
		for (int i = 0; i < skeleton.bones.size(); ++i)
		{
			// �{�[���̎n�_�ƏI�_�Ƒȉ~�̏œ_�A���_��ȉ~��̓_�Ƃ��ĒZ���̒��������߂�
			const vec3& P = vertex.position;
			const vec3& F0 = skeleton.bones[i].start;
			const vec3& F1 = skeleton.bones[i].end;
			const float a = (length(P - F0) + length(P - F1)) * 0.5f;
			const vec3 c = (F0 - F1) * 0.5f;
			const float b = sqrt(a * a - dot(c, c));
			distanceList[i].distance = b;
			distanceList[i].index = i; // �֐ߔԍ�
		}

		// �����̒Z�����ɐ��񂳂���
		std::sort
		(
			distanceList.begin(),
			distanceList.end(),
			[](const Distance& lhs, const Distance& rhs)
			{
				return lhs.distance < rhs.distance;
			}
		);

		// �Z���ق�����ő�4�c��(4�����Ȃ�[����ǉ�����4�ɂ���)
		distanceList.resize(4, Distance{ 0, 0 });

		// �������e���x�ɕϊ�����
		float total = 0; // �e���x�̍��v
		for (auto& e : distanceList)
		{
			// �e���͈͂𒴂������Ȃ����߂̌W�����v�Z����(���C�g�͈̔͐����Ɠ�����)
			const float radius = skeleton.bones[e.index].influenceRadius;
			const float f = std::clamp(1 - pow(e.distance / radius, 4.0f), 0.0f, 1.0f);

			// �t�Q��̖@���ŉe���x���v�Z����
			e.distance = f * f / (e.distance * e.distance + 0.0001f);
			total += e.distance;
		}

		// �e���x�̍��v��1�ɂȂ�悤�ɐ��K������
		if (total > 0)
		{
			for (auto& e : distanceList)
			{
				e.distance /= total;
			}
		}
		else
		{
			// ���ׂĂ�distance��0��(�ǂ̃{�[���������������)�ꍇ�A
			// ��ԋ߂��{�[���̂݉e������悤�ɐݒ肷��
			distanceList[0].distance = 1;
		}

		// ���_�Ɋ֐߃f�[�^��ݒ肷��
		for (int i = 0; i < 4; ++i)
		{
			vertex.weights[i] = static_cast<uint16_t>(distanceList[i].distance * 0xffff);
			vertex.joints[i] = static_cast<uint16_t>(distanceList[i].index);
		}
	}	// for vertex

	// �ϊ������f�[�^���v���~�e�B�u�o�b�t�@�ɒǉ�����
	AddVertexData
	(
		skeletalVertices.data(),
		skeletalVertices.size() * sizeof(VERTEX_SKELETAL),
		meshData.indices.data(),
		meshData.indices.size() * sizeof(uint16_t),
		sizeof(VERTEX_SKELETAL)
	);

	// �X�P���^�����b�V�����쐬����
	auto pMesh = std::make_shared<MESH_SKELETAL>();
	pMesh->drawParamsList.swap(meshData.drawParamsList);
	pMesh->materials.swap(meshData.materials);
	pMesh->name = filename;
	skeletalMeshes.emplace(filename, pMesh);

	// AddVertexData�ŃR�s�[�����f�[�^�̈ʒu�𔽉f����
	const intptr_t baseIndexOffset =
		reinterpret_cast<intptr_t>(drawParamsList.back().indices);
	const GLint baseVertex = drawParamsList.back().baseVertex;
	for (auto& e : pMesh->drawParamsList)
	{
		e.baseVertex = baseVertex;
		e.indices = reinterpret_cast<void*>(
			baseIndexOffset + reinterpret_cast<intptr_t>(e.indices));
	}

	// �W���C���g�ݒ�
	pMesh->joints.resize(skeleton.bones.size());
	for (int i = 0; i < pMesh->joints.size(); ++i)
	{
		// �{�[���̎n�_���W���֐߂Ƃ݂Ȃ��čs����v�Z����
		MESH_SKELETAL::JOINT& joint = pMesh->joints[i];
		const SKELETON::BONE& bone = skeleton.bones[i];
		joint.bindPose = GetTransformMatrix(vec3(1), vec3(0), bone.start);
		joint.inverseBindPose = GetTransformMatrix(vec3(1), vec3(0), -bone.start);

		// �e�֐߂̔ԍ���ݒ肷��
		joint.parent = bone.parent;
	} // for i

	LOG("%s��ǂݍ��݂܂���(���_��=%d, �C���f�b�N�X��=%d)",
		filename, meshData.vertices.size(), meshData.indices.size());

	// �쐬�������b�V����Ԃ�
	return pMesh;
}

/// <summary>
/// OBJ�t�@�C�������b�V���f�[�^�ɕϊ�����
/// </summary>
/// <param name="filename">OBJ�t�@�C����</param>
/// <returns>filename����쐬�������b�V���f�[�^</returns>
MeshBuffer::MESH_DATA MeshBuffer::CreateMeshDataFromOBJ(const char* filename)
{
	// OBJ�t�@�C�����J��
	std::ifstream file(filename);
	if (!file)
	{
		LOG_ERROR("%s���J���܂���", filename);
		return {};
	}

	// �t�H���_�����擾����
	std::string foldername(filename);
	{
		const size_t p = foldername.find_last_of("����/");
		if (p != std::string::npos)
		{
			foldername.resize(p + 1);
		}
	}

	// OBJ�t�@�C������͂��āA���_�f�[�^�ƃC���f�b�N�X�f�[�^��ǂݍ���
	std::vector<vec3> positions;
	std::vector<vec2> texcoords;
	std::vector<vec3> normals;
	struct IndexSet { int v, vt, vn; };
	std::vector<IndexSet> faceIndexSet;

	positions.reserve(20'000);
	texcoords.reserve(20'000);
	normals.reserve(20'000);
	faceIndexSet.reserve(20'000 * 3);

	// �}�e���A��
	std::vector<MaterialPtr> materials;
	materials.reserve(100);

	// �}�e���A���̎g�p�͈�
	struct USE_MATERIAL
	{
		std::string name;	// �}�e���A����
		size_t startOffset;	// ���蓖�Ĕ͈͂̐擪�ʒu
	};
	std::vector<USE_MATERIAL> usemtls;
	usemtls.reserve(100);

	// ���f�[�^��ǉ�����(�}�e���A���w�肪�Ȃ��t�@�C���΍�)
	usemtls.push_back({ std::string(),0 });

	while (!file.eof())
	{
		std::string line;
		std::getline(file, line);
		const char* p = line.c_str();

		// ���_���W�̓ǂݎ������݂�
		vec3 v;
		if (sscanf(p, "v %f %f %f", &v.x, &v.y, &v.z) == 3)
		{
			positions.push_back(v);
			continue;
		}

		// �e�N�X�`�����W�̓ǂݎ������݂�
		vec2 vt;
		if (sscanf(p, "vt %f %f", &vt.x, &vt.y) == 2)
		{
			texcoords.push_back(vt);
			continue;
		}

		// �@���̓ǂݎ������݂�
		vec3 vn;
		if (sscanf(p, "vn %f %f %f", &vn.x, &vn.y, &vn.z) == 3)
		{
			vn = normalize(vn); // ���K������Ă��Ȃ����Ƃ�����̂Ő��K������
			normals.push_back(vn);
			continue;
		}

		// �C���f�b�N�X�f�[�^�̓ǂݎ������݂�
		IndexSet f0, f1, f2;
		int readByte;
		// ���_���W + �e�N�X�`�����W + �@��
		if (sscanf(p, "f %u/%u/%u %u/%u/%u%n", &f0.v, &f0.vt, &f0.vn, &f1.v, &f1.vt, &f1.vn, &readByte) == 6)
		{
			p += readByte; // �ǂݎ��ʒu���X�V
			for (;;)
			{
				if (sscanf(p, "%u/%u/%u%n", &f2.v, &f2.vt, &f2.vn, &readByte) != 3)
				{
					break;
				}
				p += readByte; // �ǂݎ��ʒu���X�V����
				faceIndexSet.push_back(f0);
				faceIndexSet.push_back(f1);
				faceIndexSet.push_back(f2);
				f1 = f2;	   // ���̎O�p�`�̂��߂Ƀf�[�^���ړ�����
			}
			continue;
		}

		// ���_���W + �e�N�X�`�����W
		if (sscanf(p, "f %u/%u %u/%u%n", &f0.v, &f0.vt, &f1.v, &f1.vt, &readByte) == 4)
		{
			f0.vn = f1.vn = 0; // �@���Ȃ�
			p += readByte;	   // �ǂݎ��ʒu���X�V����
			for (;;)
			{
				if (sscanf(p, "%u/%u%n", &f2.v, &f2.vt, &readByte) != 2)
				{
					break;
				}
				f2.vn = 0;	   // �@���Ȃ�
				p += readByte; // �ǂݎ��ʒu���X�V����
				faceIndexSet.push_back(f0);
				faceIndexSet.push_back(f1);
				faceIndexSet.push_back(f2);
				f1 = f2;	   // ���̎O�p�`�̂��߂Ƀf�[�^���ړ�����
			}
			continue;
		}

		// MTL�t�@�C���̓ǂݎ������݂�
		char mtlFilename[1000];
		if (sscanf(line.data(), " mtllib %999s", mtlFilename) == 1)
		{
			const auto tmp = LoadMTL(foldername, mtlFilename);
			materials.insert(materials.end(), tmp.begin(), tmp.end());
			continue;
		}

		// �g�p�}�e���A�����̓ǂݎ������݂�
		char mtlName[1000];
		if (sscanf(line.data(), " usemtl %999s", mtlName) == 1)
		{
			usemtls.push_back({ mtlName,faceIndexSet.size() });
			continue;
		}
	} // while eof

	// �����ɔԕ���ǉ�����
	usemtls.push_back({ std::string(),faceIndexSet.size() });

	// OBJ�t�@�C����f�\����OpenGL�̒��_�C���f�b�N�X�z��̑Ή��\
	std::unordered_map<uint64_t, uint16_t> indexMap;
	indexMap.reserve(10'000);

	// �ǂݍ��񂾃f�[�^���AOpenGL�Ŏg����f�[�^�ɕϊ�����
	MESH_DATA meshData;
	std::vector<VERTEX>& vertices = meshData.vertices;
	vertices.reserve(faceIndexSet.size());
	std::vector<uint16_t>& indices = meshData.indices;
	indices.reserve(faceIndexSet.size());
	for (const auto& e : faceIndexSet)
	{
		// f�\���̒l��64�r�b�g�́u�L�[�v�ɕϊ�����
		const uint64_t key = static_cast<uint64_t>(e.v) +
			(static_cast<uint64_t>(e.vt) << 20) + (static_cast<uint64_t>(e.vn) << 40);

		// �Ή��\����L�[�Ɉ�v����f�[�^����������
		const auto itr = indexMap.find(key);
		if (itr != indexMap.end())
		{
			// �Ή��\�ɂ���̂Ŋ����̒��_�C���f�b�N�X���g��
			indices.push_back(itr->second);
		}
		else
		{
			// �Ή��\�ɂȂ��̂ŐV�������_�f�[�^���쐬���A���_�z��ɒǉ�����
			VERTEX v;
			v.position = positions[e.v - 1];
			v.texcoord = texcoords[e.vt - 1];
			// �@�����ݒ肳��Ă��Ȃ��ꍇ��0��ݒ肷��(��Ōv�Z)
			(e.vn == 0) ? v.normal = { 0,0,0 } : v.normal = normals[e.vn - 1];
			vertices.push_back(v);

			// �V�������_�f�[�^�̃C���f�b�N�X���A���_�C���f�b�N�X�z��ɒǉ�����
			const uint16_t index = static_cast<uint16_t>(vertices.size() - 1);
			indices.push_back(index);

			// �L�[�ƒ��_�C���f�b�N�X�̃y�A��Ή��\�ɒǉ�����
			indexMap.emplace(key, index);
		}
	}

	// �ݒ肳��Ă��Ȃ��@����₤
	FillMissingNormals(vertices.data(), vertices.size(), indices.data(), indices.size());

	// �^���W�F���g�x�N�g�����v�Z
	if (mikkTSpace)
	{
		// MikkTSpace���C�u�����Ń^���W�F���g���v�Z
		MikkTSpace::UserData userData = { indices, vertices };
		SMikkTSpaceContext context = { &mikkTSpace->interface, &userData };
		genTangSpaceDefault(&context);
	}

	// �f�[�^�̈ʒu������������
	const void* indexOffset = 0;
	const GLint baseVertex = 0;

	// �}�e���A���ɑΉ������`��p�����[�^���쐬����
	// ���f�[�^�Ɣԕ��ȊO�̃}�e���A��������ꍇ�A���f�[�^���΂�
	size_t i = 0;
	if (usemtls.size() > 2)
	{
		i = 1; // ���f�[�^�Ɣԕ��ȊO�̃}�e���A��������ꍇ�A���f�[�^���΂�
	}
	for (; i < usemtls.size() - 1; ++i)
	{
		const USE_MATERIAL& cur = usemtls[i];	   // �g�p���̃}�e���A��
		const USE_MATERIAL& next = usemtls[i + 1]; // ���̃}�e���A��
		if (next.startOffset == cur.startOffset)
		{
			continue; // �C���f�b�N�X�f�[�^���Ȃ��ꍇ�͔�΂�
		}

		// �`��p�����[�^���쐬
		DRAW_PARAMS params;
		params.mode = GL_TRIANGLES;
		params.count = static_cast<GLsizei>(next.startOffset - cur.startOffset);
		params.indices = indexOffset;
		params.baseVertex = baseVertex;
		params.materialNo = 0; // �f�t�H���g�l��ݒ�
		for (int i = 0; i < materials.size(); ++i)
		{
			if (materials[i]->name == cur.name)
			{
				// ���O�̈�v����}�e���A����ݒ肷��
				params.materialNo = i;
				break;
			}
		}
		meshData.drawParamsList.push_back(params);

		// �C���f�b�N�X�I�t�Z�b�g��ύX����
		indexOffset
			= reinterpret_cast<void*>(reinterpret_cast<size_t>(indexOffset) + sizeof(uint16_t) * params.count);
	}

	// �}�e���A���z�񂪋�̏ꍇ�A�f�t�H���g�}�e���A����ǉ�����
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
/// ���_�f�[�^��ǉ�����
/// </summary>
/// <param name="vertices">GPU�������ɃR�s�[���钸�_�f�[�^�z��</param>
/// <param name="vertexBytes">vertices�̃o�C�g��</param>
/// <param name="indices">GPU�������ɃR�s�[����C���f�b�N�X�f�[�^�z��</param>
/// <param name="indexBytes">indices�̃o�C�g��</param>
/// <param name="stride">���_�f�[�^�̊Ԋu(�o�C�g��)</param>
/// <param name="mode">�v���~�e�B�u�̎��</param>
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
	// �󂫗e�ʂ�����Ă��邱�Ƃ��m�F����
	// �K�v�ȃo�C�g�����󂫃o�C�g�����傫���ꍇ�͒ǉ��ł��Ȃ�
	const size_t totalBytes = vertexBytes + indexBytes;
	const size_t freeBytes = buffer->GetSize() - bytes_used;
	if (totalBytes > freeBytes)
	{
		LOG_ERROR
		(
			"�e�ʂ�����܂���(�v���T�C�Y=%d,�g�p��/�ő�e��=%d/%d)",
			vertexBytes + indexBytes,
			bytes_used,
			buffer->GetSize()
		);
		return;
	}

	// ���_�f�[�^��GPU�������ɃR�s�[
	GLuint tmp[2];
	glCreateBuffers(2, tmp);
	glNamedBufferStorage(tmp[0], vertexBytes, vertices, 0);
	glNamedBufferStorage(tmp[1], indexBytes, indices, 0);
	glCopyNamedBufferSubData(tmp[0], *buffer, 0, bytes_used, vertexBytes);
	glCopyNamedBufferSubData(tmp[1], *buffer, 0, bytes_used + vertexBytes, indexBytes);
	glDeleteBuffers(2, tmp);

	// �ǉ������}�`�̕`��p�����[�^���쐬����
	DRAW_PARAMS newParams;
	newParams.mode = mode;
	newParams.count = static_cast<GLsizei>(indexBytes / sizeof(uint16_t));
	// �C���f�b�N�X�f�[�^�ʒu(�o�C�g���Ŏw��)�͒��_�f�[�^�̒���
	newParams.indices = reinterpret_cast<void*>(bytes_used + vertexBytes);
	// ���_�f�[�^�̈ʒu�͒��_�f�[�^���Ŏw�肷��
	newParams.baseVertex = static_cast<GLint>(bytes_used / stride);
	drawParamsList.push_back(newParams);

	// ���̃f�[�^�i�[�J�n�ʒu���v�Z����
	constexpr size_t a = std::lcm(sizeof(VERTEX_SKELETAL),
		std::lcm(sizeof(uint16_t), sizeof(VERTEX)));
	bytes_used += ((totalBytes + a - 1) / a) * a;
}

/// <summary>
/// �����̕`��p�����[�^�ƃe�N�X�`������V�����X�^�e�B�b�N���b�V�����쐬����
/// </summary>
/// <param name="name">���b�V����</param>
/// <param name="params">���b�V���̌��ɂȂ�`��p�����[�^</param>
/// <param name="texture_color_base">���b�V���ɐݒ肷��x�[�X�J���[�e�N�X�`��</param>
/// <returns>�쐬�����X�^�e�B�b�N���b�V��</returns>
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
/// �S�Ă̒��_�f�[�^���폜����
/// </summary>
void MeshBuffer::Clear()
{
	bytes_used = 0;
	meshes.clear();
	drawParamsList.clear();
}

/// <summary>
/// �����Ă���@����₤
/// </summary>
/// <param name="vertices">���_�z��</param>
/// <param name="vertexCount">���_�z��̗v�f��</param>
/// <param name="indices">�C���f�b�N�X�z��</param>
/// <param name="indexCount">�C���f�b�N�X�z��̗v�f��</param>
void FillMissingNormals
(
	VERTEX* vertices,
	size_t vertexCount,
	const uint16_t* indices,
	size_t indexCount
)
{
	// �@�����ݒ肳��Ă��Ȃ����_��������
	std::vector<bool> missingNomals(vertexCount, false);
	for (int i = 0; i < vertexCount; ++i)
	{
		// �@���̒�����0�̏ꍇ���u�ݒ肳��Ă��Ȃ��v�Ƃ݂Ȃ�
		const vec3& n = vertices[i].normal;
		if (n.x == 0 && n.y == 0 && n.z == 0)
		{
			missingNomals[i] = true;
		}
	}

	// �@�����v�Z����
	for (int i = 0; i < indexCount; i += 3)
	{
		// �ʂ��\������2��a,b�����߂�
		const int i0 = indices[i + 0];
		const int i1 = indices[i + 1];
		const int i2 = indices[i + 2];
		const vec3& v0 = vertices[i0].position;
		const vec3& v1 = vertices[i1].position;
		const vec3& v2 = vertices[i2].position;
		const vec3 a = { v1.x - v0.x,v1.y - v0.y,v1.z - v0.z };
		const vec3 b = { v2.x - v0.x,v2.y - v0.y,v2.z - v0.z };

		// �O�ςɂ����a��b�ɐ����ȃx�N�g��(�@��)�����߂�
		const float cx = a.y * b.z - a.z * b.y;
		const float cy = a.z * b.x - a.x * b.z;
		const float cz = a.x * b.y - a.y * b.x;

		// �@���𐳋K�����ĒP�ʃx�N�g���ɂ���
		const float l = sqrt(cx * cx + cy * cy + cz * cz);
		const vec3 normal = { cx / l,cy / l,cz / l };

		// �@�����ݒ肳��Ă��Ȃ����_�ɂ����@�������Z����
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

	// �@���𐳋K������
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
/// �X�^�e�B�b�N���b�V�����擾����
/// </summary>
/// <param name="name">�X�^�e�B�b�N���b�V���̖��O</param>
/// <returns>���O��name�ƈ�v����X�^�e�B�b�N���b�V��</returns>
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
/// �X�P���^�����b�V�����擾����
/// </summary>
/// <param name="name">�X�P���^�����b�V���̖��O</param>
/// <returns>���O��name�ƈ�v����X�P���^�����b�V��</returns>
SkeletalMeshPtr MeshBuffer::GetSkeletalMesh(const char* name) const
{
	auto itr = skeletalMeshes.find(name);
	if (itr != skeletalMeshes.end())
	{
		return itr->second;
	}

	return nullptr;
}