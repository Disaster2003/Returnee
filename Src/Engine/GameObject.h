/**
* @file GameObject.h
*/
#ifndef GAMEOBJECT_H_INCLUDED
#define GAMEOBJECT_H_INCLUDED
#include "AabbCollider.h"
#include "Texture.h"
#include <vector>
#include <type_traits>

//��s�錾
class Engine;
class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>;
using GameObjectList = std::vector<GameObjectPtr>;

struct MESH_STATIC;								   
using StaticMeshPtr = std::shared_ptr<MESH_STATIC>;

class SkeletalMeshRenderer;
using SkeletalMeshRendererPtr = std::shared_ptr<SkeletalMeshRenderer>;

struct MATERIAL;
using MaterialPtr = std::shared_ptr<MATERIAL>;
using MaterialList = std::vector<MaterialPtr>;

// �����Ȋ֐ߔԍ�
constexpr int number_joint_invalid = -1;

// �����_�[�E�L���[
enum RENDER_QUEUE
{
	RENDER_QUEUE_GEOMETRY = 2000,	 // ��ʓI�Ȑ}�`
	RENDER_QUEUE_TRANSPARENT = 3000, // �������Ȑ}�`
	RENDER_QUEUE_OVERLAY = 4000,	 // UI�A�S��ʃG�t�F�N�g�Ȃ�
	RENDER_QUEUE_MAX = 5000,		 // �L���[�̍ő�l
};

/// <summary>
/// �Q�[���ɓo�ꂷ�邳�܂��܂ȃI�u�W�F�N�g��\����{�N���X
/// </summary>
class GameObject
{
	friend Engine;
public:
	// �f�t�H���g�R���X�g���N�^
	GameObject() = default;
	// �f�X�g���N�^
	virtual ~GameObject();

	// �R�s�[�Ƒ�����֎~����
	GameObject(GameObject&) = delete;
	GameObject& operator=(GameObject&) = delete;

	/// <summary>
	/// �Q�[���G���W�����擾����
	/// </summary>
	Engine* GetEngine() const { return engine; }

	/// <summary>
	/// �Q�[���I�u�W�F�N�g���G���W������폜����
	/// </summary>
	void Destroy() { isDestroyed = true; }

	/// <summary>
	/// �Q�[���I�u�W�F�N�g���j�󂳂�Ă��邩���擾����
	/// </summary>
	/// <returns>�Q�[���I�u�W�F�N�g��true = �j�󂳂�Ă���Afalse = �j�󂳂�Ă��Ȃ�</returns>
	bool IsDestroyed() const { return isDestroyed; }


	/// <summary>
	/// �Q�[���I�u�W�F�N�g�ɃR���|�[�l���g��ǉ�����
	/// </summary>
	/// <returns>�ǉ������R���|�[�l���g</returns>
	template<typename T>
	std::shared_ptr<T> AddComponent()
	{
		auto p = std::make_shared<T>();
		p->owner = this;
		if constexpr (std::is_base_of_v<Collider, T>)
			colliders.push_back(p);
		components.push_back(p);
		p->Awake();
		return p;
	}

	/// <summary>
	/// �Q�[���I�u�W�F�N�g�̃R���|�[�l���g���擾����
	/// </summary>
	/// <returns>nullptr = �R���|�[�l���g�Ȃ��Anullptr != �������ꂽ�R���|�[�l���g</returns>
	template<typename T>
	std::shared_ptr<T> GetComponent()
	{
		for (auto& e : components)
		{
			// shared_ptr�̏ꍇ��dynamic_cast�ł͂Ȃ�dynamic_pointer_cast���g��
			auto p = std::dynamic_pointer_cast<T>(e);
			if (p)
			{
				return p; // ���������R���|�[�l���g��Ԃ�
			}
		}
		return nullptr;	  // ������Ȃ������̂�nullptr��Ԃ�
	}

	// �Q�[���I�u�W�F�N�g����폜�\��̃R���|�[�l���g���폜����
	void RemoveDestroyedComponent();

	// �C�x���g����
	virtual void Start();
	virtual void Update(float deltaTime);
	virtual void OnCollision(const ComponentPtr& self, const ComponentPtr& other);
	virtual void OnDestroy();

	/// <summary>
	/// �e�I�u�W�F�N�g���擾����
	/// </summary>
	/// <returns>nullptr = �e�����Ȃ��Anullptr != �e�I�u�W�F�N�g�̃A�h���X</returns>
	GameObject* GetParent() const { return parent; }

	// �e�I�u�W�F�N�g��ݒ肷��
	void SetParent(GameObject* parent);
	void SetParent(const GameObjectPtr& parent);
	
	// �e�I�u�W�F�N�g�Ɗ֐ߔԍ���ݒ肷��
	void SetParentAndJoint(const GameObjectPtr& parent, int number_joint);
	void SetParentAndJoint(GameObject* parent, int number_joint);

	/// <summary>
	/// �q�I�u�W�F�N�g�̐����擾����
	/// </summary>
	size_t GetChildCount() const { return children.size(); }

	/// <summary>
	/// �q�I�u�W�F�N�g���擾����
	/// </summary>
	/// <param name="index">
	/// �q�I�u�W�F�N�g�z��̓Y���AGetChildCount���Ԃ��ȏ�̒l��n���ƃG���[�ɂȂ�̂Œ���
	/// </param>
	/// <returns>index�Ԗڂ̎q�I�u�W�F�N�g</returns>
	GameObject* GetChild(size_t index) const { return children[index]; }

	/// <summary>
	/// ���W�ϊ��s����擾����
	/// </summary>
	const mat4& GetTransformMatrix() const { return matrix_transform; }

	/// <summary>
	/// ���[���h���W���擾����
	/// </summary>
	vec3 GetWorldPosition() const { return vec3(matrix_transform.data[3]); }

	/// <summary>
	/// �@���ϊ��s����擾����
	/// </summary>
	const mat3& GetNormalMatrix() const { return matrix_normal; }


	std::string name;
	vec3 position = { 0,0,0	};	 // ���̂̈ʒu
	vec3 rotation = { 0,0,0	};	 // ���̂̉�]�p�x
	vec3 scale = { 1,1,1 };		 // ���̂̊g�嗦
	vec4 color = { 1, 1, 1, 1 }; // ���̂̐F
	StaticMeshPtr staticMesh;	 // �\������X�^�e�B�b�N���b�V��
	SkeletalMeshRendererPtr renderer; // �\������X�P���^�����b�V��
	MaterialList materials;		 // �Q�[���I�u�W�F�N�g�ŗL�̃}�e���A���z��
	int render_queue = RENDER_QUEUE_GEOMETRY; // �`�揇
	bool isGrounded = false;	 // ����ƂȂ镨�̂̏��true = ����Ă���Afalse = ����Ă��Ȃ�

	int countOfkeepingGrounded = 0;	  // �ڒn��Ԃ��ێ�������������
	vec3 ground = vec3(0, 1, 0);

	/// <summary>
	/// �n�ʂ��畂����
	/// </summary>
	void LiftOffGround()
	{
		isGrounded = false;
		countOfkeepingGrounded = 0;
		ground = vec3(0, 1, 0);
	}

	// �ǂꂾ���`�悷�邩(�F��e)
	enum class TYPE_LIGHTING : uint8_t
	{
		COLOR = 1,
		SHADOW = 2,
		COLOR_AND_SHADOW = 3,
	};
	TYPE_LIGHTING type_lighting = TYPE_LIGHTING::COLOR_AND_SHADOW;

private:
	Engine* engine = nullptr;			  // �G���W���̃A�h���X
	bool isDestroyed = false;			  // ���S�t���O
	mat4 matrix_transform = mat4(1);	  // ���W�ϊ��s��
	mat3 matrix_normal = mat3(1);		  // �@���ϊ��s��
	GameObject* parent = nullptr;         // �e�I�u�W�F�N�g
	int number_joint_parent = number_joint_invalid; // �e�̊֐ߔԍ�
	std::vector<GameObject*> children;    // �q�I�u�W�F�N�g
	std::vector<ComponentPtr> components; // �R���|�[�l���g�z��
	std::vector<ColliderPtr> colliders;	  // �R���C�_�[�z��
};

#endif // !GAMEOBJECT_H_INCLUDED