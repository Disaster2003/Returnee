/**
* @file GoalEvent.h
*/
#ifndef GOALEVENT_H_INCLUDED
#define GOALEVENT_H_INCLUDED
#include "../Engine/Component.h"

/// <summary>
/// �S�[�����o���s���R���|�[�l���g
/// </summary>
class GoalEvent
	:public Component
{
public:
	// �f�t�H���g�R���X�g���N�^
	GoalEvent() = default;
	// �f�t�H���g�f�X�g���N�^
	virtual ~GoalEvent() = default;

	// �R���|�[�l���g������������
	virtual void Awake() override;
	// ��ɃS�[����ʂ��X�V����
	virtual void Update(float deltaTime) override;

private:
	// �S�[�����o�ɂ�����Փ˃C�x���g
	virtual void OnCollision(const ComponentPtr& self, const ComponentPtr& other)override;
	
	GameObjectPtr obj_fade;	// �t�F�[�h�A�E�g�I�u�W�F�N�g
	float time_fade = 0,	// �t�F�[�h�A�E�g�܂ł̎���
		time_particle = 0;	// �p�[�e�B�N�������܂ł̎���

	bool isTriggered = false; // �S�[�����肪true = �N���Afalse = ��~
};
#endif // !GOALEVENT_H_INCLUDED