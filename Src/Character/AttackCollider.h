/**
* @file AttackCollider.h
*/
#ifndef ATTACKCOLLIDER_H_INCLUDED
#define ATTACKCOLLIDER_H_INCLUDED
#include "../Engine/SphereCollider.h"

/// <summary>
/// �U���p�̃R���C�_�[����
/// </summary>
class AttackCollider
    : public Component
{
public:
    // �U������̏��
    enum class STATE_ATTACK_COLLIDER
    {
        SLEEP,
        COLLISION_ENABLE,
        COLLISION_DISABLE,
    };

    // �R���X�g���N�^
    AttackCollider(STATE_ATTACK_COLLIDER _state)
        :state(_state)
    {}
    // �f�t�H���g�f�X�g���N�^
    virtual ~AttackCollider() = default;

    // �R���C�_�[������������
    virtual void Awake() override;

    // �U���p�̃R���C�_�[��Ԃ��X�V����
    virtual void Update(float deltaTime) override;

    /// <summary>
    /// �Փˎ��ɏՓ˂�������ɂ���ď������s��
    /// 
    /// �h���N���X�ɂ���ď������قȂ�A
    /// �K���I�[�o�[���C�h����K�v�����邽�߁A
    /// �������z�֐��ɂ���
    /// </summary>
    /// <param name="self">���g</param>
    /// <param name="other">�Փ˂�������</param>
    virtual void OnCollision(
        const ComponentPtr& self, const ComponentPtr& other) override = 0;

    // �U���������N������
    void Activate(float collider_sphere_radius);

    /// <summary>
    /// �U���������~����
    /// </summary>
    void Deactivate() { state = STATE_ATTACK_COLLIDER::SLEEP; }

    /// <summary>
    /// �U������̏�Ԃ��擾����
    /// </summary>
    STATE_ATTACK_COLLIDER GetState() const { return state; }

private:
    STATE_ATTACK_COLLIDER state = STATE_ATTACK_COLLIDER::SLEEP;

    std::shared_ptr<SphereCollider> collider; // �U������p�̃R���C�_�[
    float timer = 0.0f;                       // ���Ԍv���p�ϐ�
};

#endif // !ATTACKCOLLIDER_H_INCLUDED
