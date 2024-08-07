/**
* @file MagicMissile
*/
#ifndef MAGICMISSILE_H_INCLUDED
#define MAGICMISSILE_H_INCLUDED
#include "../../Effect/MagicMissileParticle.h"
#include "../../Engine/SphereCollider.h"

/// <summary>
/// ���@
/// </summary>
class MagicMissile
    : public Particle
{
public:
    // �R���X�g���N�^
    MagicMissile()
        :Particle(LIFE_SPAN)
    {}
    //�f�t�H���g�f�X�g���N�^
    virtual ~MagicMissile() = default;

    // ���@������������
    virtual void Awake() override;

    // ��ɖ��@�̏�Ԃ��X�V����
    virtual void Update(float deltaTime) override;

    // �Փˎ��ɏՓ˂�������ɂ���ď������s��
    virtual void OnCollision(
        const ComponentPtr& self, const ComponentPtr& other) override;

    // �_���[�W�ʂ�ݒ肷��
    void SetDamage(int _damage);

    vec3 velocity = vec3(0);

private:
    int damage = 0;              // �^����_���[�W��
    float time_particle = 0.05f; // �p�[�e�B�N�������܂ł̎���
    static constexpr float LIFE_SPAN = 1.0f; // ���@�̎���(s)
};

#endif // !MAGICMISSILE_H_INCLUDED
