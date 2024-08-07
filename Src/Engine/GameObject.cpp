/**
* @file GameObject.cpp
*/
#include "GameObject.h"
#include <algorithm>

/// <summary>
/// �f�X�g���N�^
/// </summary>
GameObject::~GameObject()
{
	// ���ʑO�ɐe�q�֌W����������
	SetParent(nullptr);
	for (auto child : children)
		child->parent = nullptr;
}

/// <summary>
/// �Q�[���I�u�W�F�N�g����폜�\��̃R���|�[�l���g���폜����
/// </summary>
void GameObject::RemoveDestroyedComponent()
{
	if (components.empty())
		return;	// �R���|�[�l���g�������Ă��Ȃ���Ή������Ȃ�

	// �j���\��̗L���ŃR���|�[�l���g�𕪂���
	const auto iter =
		std::stable_partition
		(
			components.begin(),
			components.end(),
			[](const auto& p) { return !p->IsDestroyed(); }
	);

	// �j���\��̃R���|�[�l���g��ʂ̔z��Ɉړ�����
	std::vector<ComponentPtr> destroyList
	(
		std::move_iterator(iter),
		std::move_iterator(components.end())
	);

	// �z�񂩂�ړ��ς݃R���|�[�l���g���폜����
	components.erase(iter, components.end());

	// �R���C�_�[��colliders�ɂ��o�^����Ă���̂ŁA��������폜����K�v������
	const auto iter2 =
		std::remove_if
		(
			colliders.begin(),
			colliders.end(),
			[](const auto& p) { return p->IsDestroyed(); }
		);
	colliders.erase(iter2, colliders.end());

	// �j���\��̃R���|�[�l���g��OnDestroy�����s����
	for (auto& e : destroyList)
		e->OnDestroy();

	// �����Ŏ��ۂɃR���|�[�l���g���폜�����(destroyList�̎������I��邽��)
}

/// <summary>
/// �X�^�[�g�C�x���g
/// </summary>
void GameObject::Start()
{
	for(auto& e : components)
	{
		if(!e->isStarted)
			e->Start();

		e->isStarted = true;
	}
}

/// <summary>
/// �X�V�C�x���g
/// </summary>
/// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
void GameObject::Update(float deltaTime)
{
	for(auto& e : components)
		e->Update(deltaTime);
	
	RemoveDestroyedComponent();
}

/// <summary>
/// �Փ˃C�x���g
/// </summary>
/// <param name="self">�Փ˂����R���|�[�l���g(����)</param>
/// <param name="other">�Փ˂����R���|�[�l���g(����)</param>
void GameObject::OnCollision
(
	const ComponentPtr& self,
	const ComponentPtr& other
)
{
	for(auto& e : components)
		e->OnCollision(self,other);
}

/// <summary>
/// �폜�C�x���g
/// </summary>
void GameObject::OnDestroy()
{
	for(auto& e : components)
		e->OnDestroy();
}

/// <summary>
/// �e�I�u�W�F�N�g��ݒ肷��
/// </summary>
/// <param name="parent">
/// �e�ɂ���Q�[���I�u�W�F�N�g�Anullptr���w�肷��Ɛe�q�֌W����������
/// </param>
void GameObject::SetParent(GameObject* parent)
{
	// �����e���w�肳�ꂽ�ꍇ�͉������Ȃ�
	if (parent == this->parent)
		return;

	// �ʂ̐e������ꍇ�A���̐e�Ƃ̊֌W����������
	if (this->parent)
	{
		// �����̈ʒu����������
		auto& c = this->parent->children;
		auto itr = std::find(c.begin(), c.end(), this);
		if (itr != c.end())
			c.erase(itr); // �z�񂩂玩�����폜����
	}

	// �V���Ȑe�q�֌W��ݒ肷��
	if (parent)
		parent->children.push_back(this);

	this->parent = parent;
	number_joint_parent = number_joint_invalid; // �����Ȋ֐ߔԍ���ݒ肷��
}

/// <summary>
/// �e�I�u�W�F�N�g��ݒ肷��
/// </summary>
/// <param name="parent">
/// �e�ɂ���Q�[���I�u�W�F�N�g�Anullptr���w�肷��Ɛe�q�֌W����������
/// </param>
void GameObject::SetParent(const GameObjectPtr& parent)
{
	SetParent(parent.get());
}

/// <summary>
/// �e�I�u�W�F�N�g�Ɗ֐ߔԍ���ݒ肷��
/// </summary>
/// <param name="parent">
/// �e�ɂ���Q�[���I�u�W�F�N�g�Anullptr���w�肷��Ɛe�q�֌W����������
/// </param>
/// <param name="number_joint">�֐ߔԍ�</param>
void GameObject::SetParentAndJoint
(
	const GameObjectPtr& parent,
	int number_joint
)
{
	SetParent(parent.get());
	number_joint_parent = number_joint;
}

/// <summary>
/// �e�I�u�W�F�N�g�Ɗ֐ߔԍ���ݒ肷��
/// </summary>
/// <param name="parent">
/// �e�ɂ���Q�[���I�u�W�F�N�g�Anullptr���w�肷��Ɛe�q�֌W����������
/// </param>
/// <param name="number_joint">�֐ߔԍ�</param>
void GameObject::SetParentAndJoint
(
	GameObject* parent,
	int number_joint
)
{
	SetParent(parent);
	number_joint_parent = number_joint;
}