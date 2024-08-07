/**
* @file Collision.h
*/
#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED
#include "VecMath.h"

// �����s���E�{�b�N�X
struct AABB
{
	vec3 min;
	vec3 max;
};

// ����
struct SPHERE
{
	vec3 position; // ���S���W
	float radius;  // ���a
};

// �L�����E�{�b�N�X(OBB)
struct BOX 
{
  vec3 position; // ���S���W
  vec3 axis[3];  // ���̌����x�N�g��
  vec3 scale;    // �������̑傫��
};

// ����
struct RAY
{
	vec3 start;		// �����̎n�_
	vec3 direction;	// �����̌���
};

// AABB���m�̌���������s��
bool Intersect(const AABB& a,const AABB& b,vec3& penetration);
// ���̂Ƌ��̂̌���������s��
bool Intersect(const SPHERE& a, const SPHERE& b, vec3& penetration);
// AABB�Ƌ��̂̌���������s��
bool Intersect(const AABB & a, const SPHERE & b, vec3 & penetration);
// OBB�Ƌ��̂̌���������s��
bool Intersect(const BOX & box, const SPHERE & sphere, vec3 & penetration);

// AABB�ƌ����̌���������s��
bool Intersect(const AABB& a,const RAY& b,float& distance);
// ���̂ƌ����̌���������s��
bool Intersect(const SPHERE & a, const RAY & b, float& distance);
// OBB�ƌ����̌���������s��
bool Intersect(const BOX & a, const RAY & b, float& distance);

#endif // !COLLISION_H_INCLUDED