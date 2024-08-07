/**
* @file Engine.h
*/
#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED
#include "glad/glad.h"
#include "GameObject.h"
#include "Scene.h"
#include "Mesh.h"
#include "FramebufferObject.h"
#include <GLFW/glfw3.h>

#include <unordered_map>
#include <utility>
#include <fstream>
#include <filesystem>

// ��s�錾
using ProgramObjectPtr = std::shared_ptr<class ProgramObject>;

// SSBO�̃o�C���f�B���O�|�C���g�ԍ�
constexpr GLuint bpJointMatrices = 0; // �֐ߍs��

// �V�F�[�_�Ŏg���郉�C�g�̐�
constexpr size_t maxShaderLightCount = 16;

// �V�F�[�_�̃��P�[�V�����ԍ�
constexpr GLint locTransformMatrix = 0;
constexpr GLint locNormalMatrix = 1;
constexpr GLint locViewProjectionMatrix = 2;
constexpr GLint locShadowTextureMatrix = 10;
constexpr GLint locShadowNormalOffset = 11;
constexpr GLint locColor = 100;
constexpr GLint locAlphaCutoff = 102;
constexpr GLint locAmbientLight = 107;
constexpr GLint locDirectionalLightColor = 108;
constexpr GLint locDirectionalLightDirection = 109;
constexpr GLint locLightCount = 110;
constexpr GLint locLightColorAndFalloffAngle = 111;
constexpr GLint locLightPositionAndRadius = locLightColorAndFalloffAngle + maxShaderLightCount;
constexpr GLint locLightDirectionAndConeAngle = locLightColorAndFalloffAngle + maxShaderLightCount * 2;

// �摜�����V�F�[�_�p�̃��P�[�V�����ԍ�
constexpr GLint locColorMatrix = 200;
constexpr GLint locRadialBlur = 201;
constexpr GLint locHighPassFilter = 202;

// �_����
struct LIGHT_DATA
{
	vec3 color;			// �F
	float intensity;	// ���邳
	vec3 position;		// �ʒu
	float radius;		// ���C�g���͂��ő唼�a

	// �X�|�b�g���C�g�p�̕ϐ�
	vec3 direction;		// ���C�g�̌���
	float coneAngle;	// �X�|�b�g���C�g���Ƃ炷�p�x
	float falloffAngle;	// �X�|�b�g���C�g�̌����J�n�p�x

	bool isUsed = false;// �g�p�����ǂ���
};

// ���s����
struct DIRECTIONAL_LIGHT
{
  vec3 color = { 1.00f, 0.98f, 0.95f };		 // �F
  float intensity = 5.0f;					 // ���邳
  vec3 direction = { 0.58f, -0.58f, 0.58f }; // ����
};

/// <summary>
/// �Q�[���G���W��
/// </summary>
class Engine
{
public:
	// �f�t�H���g�R���X�g���N�^
	Engine() = default;
	// �f�t�H���g�f�X�g���N�^
	virtual ~Engine() = default;

	// �Q�[���G���W�������s����
	int Run();

	/// <summary>
	/// �Q�[���I�u�W�F�N�g���쐬����
	/// </summary>
	/// <typeparam name="T">�^�̈Ⴂ�̊��e��</typeparam>
	/// <param name="name">�I�u�W�F�N�g�̖��O</param>
	/// <param name="position">�I�u�W�F�N�g��z�u������W</param>
	/// <param name="rotation">�I�u�W�F�N�g�̉�]�p�x</param>
	/// <returns>�Q�[���I�u�W�F�N�g�̃f�[�^</returns>
	template<typename T>
	std::shared_ptr<T> Create
	(
		const std::string& name,
		const vec3& position = { 0,0,0 },
		const vec3& rotation = { 0,0,0 }
	)
	{
		std::shared_ptr<T> p = std::make_shared<T>();
		p->engine = this;
		p->name = name;
		p->position = position;
		p->rotation = rotation;
		gameObjects.push_back(p); // �G���W���ɓo�^����
		return p;
	}

	/// <summary>
	/// UI�I�u�W�F�N�g���쐬����
	/// </summary>
	/// <typeparam name="T">�Q�[���I�u�W�F�N�g�Ɋ��蓖�Ă�UILayout�܂��͂��̔h���N���X</typeparam>
	/// <param name="fileName">UI�I�u�W�F�N�g�ɕ\������摜</param>
	/// <param name="position">UI�I�u�W�F�N�g�̍��W</param>
	/// <param name="scale">UI�I�u�W�F�N�g�̑傫��</param>
	/// <returns>�쐬����UI�I�u�W�F�N�g</returns>
	template<typename T>
	std::pair<GameObjectPtr, std::shared_ptr<T>> CreateUIObject
	(
		const char* fileName,
		const vec2& position,
		float scale
	)
	{
		auto object = Create<GameObject>(fileName, { position.x,position.y,0 });
		object->render_queue = RENDER_QUEUE_OVERLAY;
		object->staticMesh = GetStaticMesh("plane_xy");

		// �ŗL�}�e���A�����쐬���A�e�N�X�`���������ւ���
		auto texBaseColor = GetTexture(fileName);
		object->materials = CloneMaterialList(object->staticMesh);
		object->materials[0]->texBaseColor = texBaseColor;

		// �摜�T�C�Y�ɉ����Ċg�嗦�𒲐�����
		const float aspectRatio = texBaseColor->GetAspectRatio();
		object->scale = { scale * aspectRatio,scale,1 };

		// �R���|�[�l���g��ǉ�����
		auto component = object->AddComponent<T>();

		return{ object,component };
	}

	/// <summary>
	/// �Q�[���I�u�W�F�N�g�z�u�t�@�C���Ɋ܂܂�郁�b�V�����Ǝ��ۂ̃t�@�C�����̑Ή��\
	/// ��first = ���b�V�����Asecond = ���ۂ̃t�@�C����
	/// </summary>
	using FilepathMap = std::unordered_map<std::string, std::string>;

	// �Q�[���I�u�W�F�N�g�z�u�t�@�C����ǂݍ���
	GameObjectList LoadGameObjectMap(const char* filename, const FilepathMap&);

	// ���ׂẴQ�[���I�u�W�F�N�g���폜����
	void ClearGameObjectAll();

	// ����p�̊Ǘ�
	void SetFovY(float fovY);
	float GetFovY() const { return degFovY; }
	float GetFovScale()const { return fovScale; }

	// ���̃V�[����ݒ肷��
	template<typename T>
	void SetNextScene() { nextScene = std::make_shared<T>(); }

	// �J�������擾����
	GameObject& GetMainCamera() { return camera; }
	const GameObject& GetMainCamera() const { return camera; }

	/// <summary>
	/// ���͂��ꂽ�L�[���擾����
	/// </summary>
	/// <param name="key">���͂��ꂽ�L�[</param>
	/// <returns>true = ������Ă���Afalse = ������Ă��Ȃ�</returns>
	bool GetKey(int key) const { return glfwGetKey(window, key) == GLFW_PRESS; }

	// �}�E�X�J�[�\���̍��W���擾����
	vec2 GetMousePosition() const;

	// �}�E�X�J�[�\���̓���ʂ��擾����
	vec2 GetMouseMovement() const { return mouseMovement; }

	// �}�E�X�J�[�\�����B��
	void HideMouseCursor() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
	// �}�E�X�J�[�\����\������
	void ShowMouseCursor() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }

	// �}�E�X�̃{�^���̏�Ԃ��擾����
	bool GetMouseButton(int button) const;

	// �}�E�X�{�^���̃N���b�N��Ԃ��擾����
	bool GetMouseClick(int button) const;

	// �t���[���o�b�t�@�̑傫�����擾����
	vec2 GetFramebufferSize() const;

	// �t���[���o�b�t�@�̃A�X�y�N�g����擾����
	float GetAspectRatio() const;

	/// <summary>
	/// �X�^�e�B�b�N���b�V���̎擾
	/// </summary>
	/// <param name="name">���b�V����</param>
	/// <returns>���O��name�ƈ�v����X�^�e�B�b�N���b�V��</returns>
	StaticMeshPtr GetStaticMesh(const char* name) { return meshBuffer->GetStaticMesh(name); }

	/// <summary>
	/// �X�P���^�����b�V���̎擾
	/// </summary>
	/// <param name="name">���b�V����</param>
	/// <returns>���O��name�ƈ�v����X�P���^�����b�V��</returns>
	SkeletalMeshPtr GetSkeletalMesh(const char* name) { return meshBuffer->GetSkeletalMesh(name); }

	// �e�N�X�`�����擾����
	TexturePtr GetTexture(const char* name);

	// �L���[�u�}�b�v�e�N�X�`����ǂݍ���
	TexturePtr LoadCubemapTexture(
		const char* name, const char* const cubemapFilenames[6]);

	// ���}�b�s���O�p�̃L���[�u�}�b�v��ݒ肷��
	void SetEnvironmentCubemap(const TexturePtr & cubemap) { texEnvironment = cubemap; }

	// ���C�g�z�������������
	void InitializeLight();

	// �V�������C�g���擾����
	int AllocateLight();

	// ���C�g���������
	void DeallocateLight(int index);

	// �C���f�b�N�X�ɑΉ����郉�C�g�f�[�^���擾����
	LIGHT_DATA* GetLight(int index);
	const LIGHT_DATA* GetLight(int index) const;

	// �}�E�X���W���甭�˂����������擾����
	RAY GetRayFromMousePosition() const;

	/// <summary>
	/// �����̌������茋��
	/// </summary>
	struct RAYCAST_HIT
	{
		ColliderPtr collider; // �ŏ��Ɍ����ƌ��������R���C�_�[
		vec3 point;			  // �ŏ��̌�_�̍��W
		float distance;		  // �ŏ��̌�_�܂ł̋���
	};

	/// <summary>
	/// ��������̑ΏۂɂȂ邩�ǂ����𒲂ׂ�q��^
	/// ��collider = ���肷��R���C�_�[�Adistance = �����ƃR���C�_�[�̌�_�܂ł̋���
	/// �@true = ��������̑Ώۓ��Afalse = ��������̑ΏۊO
	/// </summary>
	using RaycastPredicate =
		std::function <bool(const ColliderPtr& collider, float distance)>;

	// �����ƃR���C�_�[�̌���������擾����
	bool Raycast(const RAY& ray, RAYCAST_HIT& hitInfo, const RaycastPredicate& pred) const;

private:
	// �R���X�g���N�^�A�f�X�g���N�^���Ăׂ�悤�ɂ��邽�߂̕⏕�N���X
	struct TEX_HELPER
		: public Texture
	{
		TEX_HELPER(const char* p)
			: Texture(p)
		{}
		TEX_HELPER(const char* p, int w, int h, GLenum f, int levels = 1)
			: Texture(p, w, h, f, GL_CLAMP_TO_EDGE, levels)
		{}
		TEX_HELPER(const char* p, const char* const c[6])
			: Texture(p, c)
		{}
	};

	// ���[���h���W�n�̃R���C�_�[��\���\����
	struct WORLDCOLLIDER
	{
		/// <summary>
		/// ���W��ύX����
		/// </summary>
		/// <param name="v">�ǉ����̍��W</param>
		void AddPosition(const vec3& v)
		{
			origin->GetOwner()->position += v;
			world->AddPosition(v);
		}

		ColliderPtr origin;
		ColliderPtr world;
	};
	using WorldColliderList = std::vector<WORLDCOLLIDER>;
	void ApplyPenetration(WorldColliderList*, GameObject*, const vec3&);

	int Initialize();
	void CreateMainFBO(int width, int height);
	void Update();
	void Render();
	void DrawGameObject
	(
		ProgramObject& prog,
		GameObjectList::iterator begin,
		GameObjectList::iterator end
	);
	void UpdateGameObject(float deltaTime);
	void UpdateJointMatrix();
	void HandleGameObjectCollision();
	void HandleWorldColliderCollision(WorldColliderList* a, WorldColliderList* b);
	void RemoveDestroyedGameObject();
	void UpdateShaderLight();
	void DrawSkySphere();
	void CreateShadowMap(
		GameObjectList::iterator begin,
		GameObjectList::iterator end);
	void DrawAmbientOcclusion();
	void DrawBloomEffect();
	void DrawMainImageToDefaultFramebuffer();

	GLFWwindow* window = nullptr;		   // �E�C���h�E�I�u�W�F�N�g
	const std::string title = "Returnee";  // �E�C���h�E�^�C�g��
	float degFovY = 60;					   // ��������p(�x���@)
	float radFovY = degFovY * 3.1415926535f / 180;
	float fovScale = 1 / tan(radFovY / 2); // ����p�ɂ��g�嗦�̋t��

	// �V�F�[�_�Ǘ��p�̃����o�ϐ�
	ProgramObjectPtr prog3D;			 // ���C�g�t���V�F�[�_
	ProgramObjectPtr progUnlit;			 // ���C�g�Ȃ��V�F�[�_
	ProgramObjectPtr progShadow;		 // �e�e�N�X�`���쐬�V�F�[�_
	ProgramObjectPtr progSkeletal;		 // ���C�g�t���X�P���^�����b�V���V�F�[�_
	ProgramObjectPtr progShadowSkeletal; // �X�P���^�����b�V���p�̉e�e�N�X�`���쐬�V�F�[�_

	// �u���[���G�t�F�N�g
	ProgramObjectPtr progHighPassFilter; // ���邢�̈�𒊏o����V�F�[�_
	ProgramObjectPtr progDownSampling;   // �k���ڂ����V�F�[�_
	ProgramObjectPtr progUpSampling;     // �g��ڂ����V�F�[�_
	float bloomThreshold = 1;			 // ���邢�Ƃ݂Ȃ�臒l
	float bloomIntensity = 2;			 // �u���[���̋���

	// SAO�p�̃����o�ϐ�
	ProgramObjectPtr progSAORestoreDepth; // ���`�����ϊ��V�F�[�_
	ProgramObjectPtr progSAODownSampling; // �k���V�F�[�_
	ProgramObjectPtr progSAO;             // SAO�v�Z�V�F�[�_
	ProgramObjectPtr progSAOBlur;         // SAO�p�ڂ����V�F�[�_
	FramebufferObjectPtr fboSAODepth[4];  // �[�x�l�̏k���o�b�t�@
	FramebufferObjectPtr fboSAO;          // SAO�v�Z���ʃo�b�t�@
	FramebufferObjectPtr fboSAOBlur;      // �ڂ������ʃo�b�t�@

	ProgramObjectPtr progSimple; // �e�N�X�`����`�悷�邾���̃V�F�[�_

	ProgramObjectPtr progImageProcessing;	  // �摜�����V�F�[�_
	mat3 colorMatrix = mat3(1);				  // �摜�����Ŏg���F�ϊ��s��
	vec2 radialBlurCenter = vec2(0.5f, 0.5f); // ���ˏ�u���[�̒��S
	float radialBlurLength = 0.02f;           // ���ˏ�u���[�̒���
	float radialBlurStart = 0.2f;             // ���ˏ�u���[�̊J�n����

	FramebufferObjectPtr fboMain;				// 3D�`��pFBO
	std::vector<FramebufferObjectPtr> fboBloom; // �u���[���G�t�F�N�g�pFBO
	FramebufferObjectPtr fboShadow;				// �f�v�X�V���h�E�pFBO

	// �֐ߍs��̔z��̍ő�o�C�g��
	static constexpr size_t jointMatricesBytes = 1024 * 1024;
	BufferObjectPtr ssboJointMatrices[3];	 // �֐ߍs��̔z��p�̃o�b�t�@
	GLsync syncJointMatrix[3] = { 0, 0, 0 }; // �֐ߍs��̓����I�u�W�F�N�g
	size_t jointMatrixIndex = 0;			 // �֐ߍs��̏������ݐ�C���f�b�N�X

	MeshBufferPtr meshBuffer; // �}�`�f�[�^�Ǘ��I�u�W�F�N�g
	std::unordered_map<std::string, TexturePtr> textureCache;

	GameObjectList gameObjects;	// �Q�[���I�u�W�F�N�g�z��
	double previousTime = 0;	// �O��X�V���̎���
	float deltaTime = 0;		// �O��X�V����̌o�ߎ���
	ScenePtr scene;				// ���s���̃V�[��
	ScenePtr nextScene;			// ���̃V�[��

	// �N���b�N�Ɣ��肷�鑬�x
	static constexpr float mouseClickSpeed = 0.3f;

	// �}�E�X�{�^���̏��
	struct MOUSEBUTTON
	{
		bool current = false;  // ���݃t���[���̃{�^���̏��
		bool previous = false; // �O�t���[���̃{�^���̏��
		bool click = false;	   // �N���b�N�̏��
		float timer = 0;	   // �{�^����������Ă��鎞��
	};
	MOUSEBUTTON mouseButtons[3];
	vec2 oldMousePosition = { 0, 0 }; // ���݂̃}�E�X�ʒu
	vec2 mousePosition = { 0, 0 };	  // �ύX��̃}�E�X�ʒu
	vec2 mouseMovement = { 0, 0 };	  // �ǂꂾ����������

	// �_����
	std::vector<LIGHT_DATA> lights;	// ���C�g�f�[�^�̔z��
	std::vector<int> usedLights;	// �g�p���̃��C�g�̃C���f�b�N�X�z��
	std::vector<int> freeLights;	// ���g�p�̃��C�g�̃C���f�b�N�X�z��

	// ��x�ɑ��₷���C�g��
	static constexpr size_t lightResizeCount = 100;

	// ���s����
	DIRECTIONAL_LIGHT directionalLight;

	// ����
	vec3 ambientLight = vec3(0);

	// ���}�b�s���O�p�̃L���[�u�}�b�v
	TexturePtr texEnvironment;

	// �X�J�C�X�t�B�A�p���f��
	StaticMeshPtr skySphere;

	// �J�����I�u�W�F�N�g
	GameObject camera;
	GLsizei indexCount = 0;
};

#endif // !ENGINE_H_INCLUDED