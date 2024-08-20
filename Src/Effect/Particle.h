/**
* @file Particle.h
*/
#ifndef PARTICLE_H_INCLUDED
#define PARTICLE_H_INCLUDED
#include "../Engine/Billboard.h"

/// <summary>
/// �p�[�e�B�N���̊��N���X
/// </summary>
class Particle
    : public Component
{
public:
    // �R���X�g���N�^
    Particle(float _lifespan)
        :life_span(_lifespan)
    {}

    // �f�t�H���g�f�X�g���N�^
    virtual ~Particle() = default;

    /// <summary>
    /// �p�[�e�B�N���S�̂�����������
    /// </summary>
    virtual void Awake() override
    {
        auto owner = GetOwner();
        auto engine = owner->GetEngine();
        owner->AddComponent<Billboard>(); // �r���{�[�h��
        owner->staticMesh = engine->GetStaticMesh("plane_xy");
        owner->materials = CloneMaterialList(owner->staticMesh);
        owner->render_queue = RENDER_QUEUE_TRANSPARENT; // �������L���[�ŕ`��
    }

    /// <summary>
    /// ��Ƀp�[�e�B�N���̎������X�V����
    /// </summary>
    /// <param name="deltaTime">�O��̍X�V����̌o�ߎ���(�b)</param>
    virtual void Update(float deltaTime) override
    {
        auto owner = GetOwner();
        // ��莞�Ԍo�߂�����
        if (life_span <= 0)
        {
            // ���g��j������
            owner->Destroy();
            return;
        }

        life_span -= deltaTime; // �������Ԃ����炷
    }

    /// <summary>
    /// �������擾����
    /// </summary>
    float GetLifespan() const { return life_span; }

private:
    float life_span = 0; // ����
};

#endif // PARTICLE_H_INCLUDED