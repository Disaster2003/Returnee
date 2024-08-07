/**
* @file Component.h
*/
#ifndef COMPONENT_H_INCLUDED
#define COMPONENT_H_INCLUDED
#include <memory>

//��s�錾
class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>;
class Component;
using ComponentPtr = std::shared_ptr<Component>;

/// <summary>
/// �R���|�[�l���g�̊��N���X
/// </summary>
class Component
{
	friend GameObject;
public:
	// �f�t�H���g�R���X�g���N�^
	inline Component() = default;
	// �f�t�H���g�f�X�g���N�^
	inline virtual ~Component() = default;

	/// <summary>
	/// �R���|�[�l���g�̏��L�҂��擾����
	/// </summary>
	inline GameObject* GetOwner() const { return owner; }

	/// <summary>
	/// �R���|�[�l���g���Q�[���I�u�W�F�N�g����폜����
	/// </summary>
	inline void Destroy() { isDestroyed = true; }

	/// <summary>
	/// �R���|�[�l���g���j�󂳂�Ă��邩�擾����
	/// </summary>
	/// <returns>�R���|�[�l���g��true = �j�󂳂�Ă���Afalse = �j�󂳂�Ă��Ȃ�</returns>
	inline bool IsDestroyed() const { return isDestroyed; }

	/// <summary>
	/// �Q�[���I�u�W�F�N�g�ɒǉ����ꂽ���ɌĂяo�����
	/// 
	/// �h���N���X�ɂ���ď������قȂ邪�A
	/// �I�[�o���C�h����K�R���͂Ȃ����߁A
	/// ���������������Ȃ�
	/// </summary>
	inline virtual void Awake() {}

	/// <summary>
	/// �ŏ���Update�̒��O�ŌĂяo�����
	/// 
	/// �h���N���X�ɂ���ď������قȂ邪�A
	/// �I�[�o���C�h����K�R���͂Ȃ����߁A
	/// ���������������Ȃ�
	/// </summary>
	inline virtual void Start() {}

	/// <summary>
	/// ���t���[��1��Ăяo�����
	/// 
	/// �h���N���X�ɂ���ď������قȂ邪�A
	/// �I�[�o���C�h����K�R���͂Ȃ����߁A
	/// ���������������Ȃ�
	/// </summary>
	/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
	inline virtual void Update(float deltaTime) {}

	/// <summary>
	/// �Փ˂��N�����Ƃ��ɌĂяo�����
	/// 
	/// �h���N���X�ɂ���ď������قȂ邪�A
	/// �I�[�o���C�h����K�R���͂Ȃ����߁A
	/// ���������������Ȃ�
	/// </summary>
	/// <param name="self">�Փ˂����R���|�[�l���g(����)</param>
	/// <param name="other">�Փ˂����R���|�[�l���g(����)</param>
	inline virtual void OnCollision(
		const ComponentPtr& self, const ComponentPtr& other) {}

	/// <summary>
	/// �Q�[���I�u�W�F�N�g���G���W������폜�����Ƃ��ɌĂяo�����
	/// 
	/// �h���N���X�ɂ���ď������قȂ邪�A
	/// �I�[�o���C�h����K�R���͂Ȃ����߁A
	/// ���������������Ȃ�
	/// </summary>
	inline virtual void OnDestroy() {}

private:
	GameObject* owner = nullptr; // ���̃R���|�[�l���g�̏��L��
	bool isStarted = false;		 // Start���@true = ���s���Afalse =�@���s����Ă��Ȃ�
	bool isDestroyed = false;	 // Destroy��true = ���s���Afalse =�@���s����Ă��Ȃ�
};

#endif // !COMPONENT_H_INCLUDED