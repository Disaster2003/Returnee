/**
* @file Light.h
*/
#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED
#include "Component.h"
#include "VecMath.h"

/// <summary>
/// ���C�g�E�R���|�[�l���g
/// </summary>
class Light
	: public Component
{
public:
	// �f�t�H���g�R���X�g���N�^
	Light() = default;
	// �f�X�g���N�^
	virtual ~Light();

	// ���C�g������������
	virtual void Awake() override;
	// ��Ƀ��C�g�̏�Ԃ��X�V����
	virtual void Update(float deltaTime) override;

	// ���C�g�̎��
	enum class TYPE_LIGHT
	{
		POINT, // �_����
		SPOT,  // �X�|�b�g����
	};
	TYPE_LIGHT type = TYPE_LIGHT::POINT; // ���C�g�̎��

	vec3 color = { 1,1,1 };	// �F
	float intensity = 1;	// ���邳
	float radius = 1;		// ���C�g���͂��ő唼�a

	// �X�|�b�g���C�g�p�̃p�����[�^
	// ������GameObject::rotation����v�Z
	float angle_cone = radians(30);	  // �X�|�b�g���C�g���Ƃ炷�p�x
	float angle_falloff = radians(20); // �X�|�b�g���C�g�̌����J�n�p�x

private:
	int index_light = -1; // ���C�g�C���f�b�N�X
};

#endif // !LIGHT_H_INCLUDED