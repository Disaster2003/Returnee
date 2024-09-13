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

//��s�錾
class MeshBuffer;
using MeshBufferPtr = std::shared_ptr<MeshBuffer>;


// ���_�f�[�^�`��
struct VERTEX
{
	vec3 position; // ���_���W
	vec2 texcoord; // �e�N�X�`�����W
	vec3 normal;   // �@���x�N�g��
	vec4 tangent;  // �ڐ��x�N�g��
};

// �`��p�����[�^
struct DRAW_PARAMS
{
	GLenum mode = GL_TRIANGLES;	// �v���~�e�B�u�̎��
	GLsizei count = 0;			// �`�悷��C���f�b�N�X��
	const void* indices = 0;	// �`��J�n�C���f�b�N�X�̃o�C�g�I�t�Z�b�g
	GLint baseVertex = 0;		// �C���f�b�N�X0�ƂȂ钸�_�z����̈ʒu
	int materialNo = -1;		// �}�e���A���C���f�b�N�X
};

// �}�e���A��
struct MATERIAL
{
	std::string name = "<Default>";	// �}�e���A����
	vec4 baseColor = { 1,1,1,1 };	// ��{�F+�A���t�@
	vec3 emission = { 0,0,0 };		// �����F
	float roughness = 0.7f;         // �\�ʂ̑e��
	float metallic = 0.0f;          // 0 = ������A1 = ����
	TexturePtr texBaseColor;		// ��{�F�e�N�X�`��
	TexturePtr texEmission;			// �����F�e�N�X�`��
	TexturePtr texNormal;           // �@���e�N�X�`��
};
using MaterialPtr = std::shared_ptr<MATERIAL>;
using MaterialList = std::vector<MaterialPtr>; // �}�e���A���z��^

// 3D���f��
struct MESH_STATIC
{
	std::string name;		//���b�V����
	std::vector<DRAW_PARAMS> drawParamsList; //�`��p�����[�^
	MaterialList materials; // ���L�}�e���A���z��
};
using StaticMeshPtr = std::shared_ptr<MESH_STATIC>;

// ���b�V����`�悷��
void Draw(const MESH_STATIC & mesh, ProgramObject& program, const MaterialList & materials);
// �`��p�����[�^�z���`�悷��
void Draw(const std::vector<DRAW_PARAMS>& drawParamsList, ProgramObject& program,
	const MaterialList & materials, const vec4* objectColor);

// �֐ߕt�����_�f�[�^�`��
struct VERTEX_SKELETAL
{
	vec3 position;		 // ���_���W
	vec2 texcoord;		 // �e�N�X�`�����W
	vec3 normal;		 // �@���x�N�g��
	vec4 tangent;		 // �ڐ��x�N�g��
	uint16_t joints[4];	 // �e�����󂯂�֐߂̔ԍ�
	uint16_t weights[4]; // �e�֐߂̉e���x
};

// �֐ߕt�����b�V��
struct MESH_SKELETAL
{
	std::string name;		// ���b�V����
	std::vector<DRAW_PARAMS> drawParamsList; // �`��p�����[�^
	MaterialList materials; // ���L�}�e���A��

	// �֐߃f�[�^
	struct JOINT
	{
		mat4 bindPose;        // �o�C���h�|�[�Y�s��
		mat4 inverseBindPose; // �t�o�C���h�|�[�Y�s��
		int32_t parent;       // �e�֐߂̔ԍ�
	};
	std::vector<JOINT> joints;
};
using SkeletalMeshPtr = std::shared_ptr<MESH_SKELETAL>;

// 

/// <summary>
/// ���L�}�e���A���z��𕡐�����
/// </summary>
/// <param name="original">�}�e���A���z��</param>
/// <returns>������������</returns>
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
/// ���_�f�[�^���Ǘ�����N���X
/// </summary>
class MeshBuffer
{
public:
	/// <summary>
	/// ���b�V���o�b�t�@���쐬����
	/// </summary>
	/// <param name="buffer_size">�i�[�ł��钸�_�f�[�^�̃T�C�Y(�o�C�g��)</param>
	static MeshBufferPtr Create(size_t buffer_size)
	{
		return std::make_shared<MeshBuffer>(buffer_size);
	}

	// �R���X�g���N�^
	MeshBuffer(size_t buffer_size);
	// �f�t�H���g�f�X�g���N�^
	virtual ~MeshBuffer() = default;

	// �R�s�[�Ƒ�����֎~����
	MeshBuffer(const MeshBuffer&) = delete;
	MeshBuffer& operator=(const MeshBuffer&) = delete;

	/// <summary>
	/// �e�N�X�`���쐬�R�[���o�b�N�^
	/// �������Ɏw�肳�ꂽ�t�@�C��������e�N�X�`�����쐬����^
	/// </summary>
	using TextureCallback = std::function<TexturePtr(const char*)>;

	/// <summary>
	/// �e�N�X�`���쐬�R�[���o�b�N��ݒ肷��
	/// </summary>
	/// <param name="callback">�ݒ肷��R�[���o�b�N�I�u�W�F�N�g</param>
	void SetTextureCallback(const TextureCallback& callback)
	{
		textureCallback = callback;
	}

	// OBJ�t�@�C����ǂݍ���
	StaticMeshPtr LoadOBJ(const char* filename);

	// �֐߃f�[�^
	struct SKELETON
	{
		// �֐߂��Ȃ���
		struct BONE
		{
			vec3 start;      // �֐߂̎n�_
			vec3 end;        // �֐߂̏I�_
			int parent = -1; // �e�֐߂̔ԍ�
			float influenceRadius = 1; // �֐߂̉e���͈�
		};
		std::vector<BONE> bones; // �֐߃��X�g
	};

	// OBJ�t�@�C����ǂݍ��݊֐߂�ݒ肷��
	SkeletalMeshPtr LoadOBJ(const char* filename, const SKELETON& skeleton);

	// ���_�f�[�^��ǉ�����
	void AddVertexData(const void* vertices, size_t vertexBytes,
		const uint16_t* indices, size_t indexBytes,
		size_t stride = sizeof(VERTEX), GLenum mode = GL_TRIANGLES);

	// �����̕`��p�����[�^�ƃe�N�X�`������V�����X�^�e�B�b�N���b�V�����쐬����
	StaticMeshPtr CreateStaticMesh(const char* name,
		const DRAW_PARAMS& params, const TexturePtr& texture_color_base);

	// �S�Ă̒��_�f�[�^���폜����
	void Clear();

	// �X�^�e�B�b�N���b�V�����擾����
	StaticMeshPtr GetStaticMesh(const char* name) const;

	// �X�P���^�����b�V�����擾����
	SkeletalMeshPtr GetSkeletalMesh(const char* name) const;

	/// <summary>
	/// �`��p�����[�^���擾����
	/// </summary>
	/// <param name="index">�z��ԍ�</param>
	const DRAW_PARAMS& GetDrawParams(size_t index) const
	{
		return drawParamsList[index];
	}

	/// <summary>
	/// �`��p�����[�^�̐����擾����
	/// </summary>
	size_t GetDrawParamsCount() const { return drawParamsList.size(); }

	/// <summary>
	/// VAO���擾����
	/// </summary>
	VertexArrayObjectPtr GetVAO() const { return vao; }
	/// <summary>
	/// �X�P���^�����b�V���p��VAO���擾����
	/// </summary>
	VertexArrayObjectPtr GetVAOSkeletal() const { return vao_skeletal; }

private:
	// MTL�t�@�C����ǂݍ���
	std::vector<MaterialPtr> LoadMTL(
		const std::string& foldername, const char* filename);

	// ���b�V���\�z�p�̒��ԃf�[�^
	struct MESH_DATA
	{
		std::vector<VERTEX> vertices;  // ���_�f�[�^
		std::vector<uint16_t> indices; // �C���f�b�N�X�f�[�^
		std::vector<DRAW_PARAMS> drawParamsList; // �`��p�����[�^�z��
		MaterialList materials;		   // �}�e���A���z��
	};
	// OBJ�t�@�C�������b�V���f�[�^�ɕϊ�����
	MESH_DATA CreateMeshDataFromOBJ(const char* filename);

	std::unordered_map<std::string, StaticMeshPtr> meshes;
	std::unordered_map<std::string, SkeletalMeshPtr> skeletalMeshes;
	std::vector<DRAW_PARAMS> drawParamsList; // �`��p�����[�^�z��
	VertexArrayObjectPtr vao;				 // ���_�A�g���r���[�g�z��
	VertexArrayObjectPtr vao_skeletal;		 // �X�P���^�����b�V���p�̒��_�z��
	BufferObjectPtr buffer;					 // ���_�f�[�^����уC���f�b�N�X�f�[�^
	size_t bytes_used = 0;					 // �o�b�t�@�̎g�p�ςݗe��(�o�C�g)
	TextureCallback textureCallback;		 // �e�N�X�`���쐬�R�[���o�b�N
	std::shared_ptr<class MikkTSpace> mikkTSpace;
};

// �����Ă���@����₤
void FillMissingNormals(VERTEX* vertices, size_t vertexCount,
	const uint16_t* indices, size_t indexCount);

#endif // !MESH_H_INCLUDED