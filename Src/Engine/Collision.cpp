/**
* @file Collision.cpp
*/
#include "Collision.h"
#include <algorithm>

/// <summary>
/// AABB���m�̌���������s��
/// </summary>
/// <param name="a">����Ώۂ�AABB����1</param>
/// <param name="b">����Ώۂ�AABB����2</param>
/// <param name="penetration">�ђʃx�N�g��</param>
/// <returns>true = �������Ă���Afalse = �������Ă��Ȃ�</returns>
bool Intersect
(
	const AABB& a,
	const AABB& b,
	vec3& penetration
)
{
	// a�̍����ʂ�b�̉E���ʂ��E�ɂ���Ȃ�A�������Ă��Ȃ�
	const float dx0 = b.max.x - a.min.x;
	if (dx0 <= 0)
	{
		return false;
	}
	// a�̉E���ʂ�b�̍����ʂ�荶�ɂ���Ȃ�A�������Ă��Ȃ�
	const float dx1 = a.max.x - b.min.x;
	if (dx1 <= 0)
	{
		return false;
	}

	// a�̉��ʂ�b�̏�ʂ���ɂ���Ȃ�A�������Ă��Ȃ�
	const float dy0 = b.max.y - a.min.y;
	if (dy0 <= 0)
	{
		return false;
	}
	// a�̏�ʂ�b�̉��ʂ�艺�ɂ���Ȃ�A�������Ă��Ȃ�
	const float dy1 = a.max.y - b.min.y;
	if (dy1 <= 0)
	{
		return false;
	}

	// a�̉����ʂ�b�̎�O���ʂ���O�ɂ���Ȃ�A�������Ă��Ȃ�
	const float dz0 = b.max.z - a.min.z;
	if (dz0 <= 0)
	{
		return false;
	}
	// a�̎�O���ʂ�b�̉����ʂ�艜�ɂ���Ȃ�A�������Ă��Ȃ�
	const float dz1 = a.max.z - b.min.z;
	if (dz1 <= 0)
	{
		return false;
	}
	// ���̎��_�Ō������m��

	// XYZ�̊e���ɂ��āA�d�Ȃ��Ă��鋗�����Z��������I������
	vec3 length = { dx1,dy1,dz1 }; // �ђʋ����̐�Βl
	vec3 signedLength = length;	   // �����t���̊ђʋ���
	if (dx0 < dx1)
	{
		length.x = dx0;
		signedLength.x = -dx0;
	}
	if (dy0 < dy1)
	{
		length.y = dy0;
		signedLength.y = -dy0;
	}
	if (dz0 < dz1)
	{
		length.z = dz0;
		signedLength.z = -dz0;
	}

	// XYZ�̂����ł��Z��������I������
	if (length.x < length.y)
	{
		if (length.x < length.z)
		{
			penetration = { signedLength.x, 0, 0 };
			return true;
		}
	}
	else if (length.y < length.z)
	{
		penetration = { 0, signedLength.y, 0 };
		return true;
	}
	penetration = { 0, 0, signedLength.z };
	return true;
}

/// <summary>
/// ���̂Ƌ��̂̌���������s��
/// </summary>
/// <param name="a">����Ώۂ̋���A</param>
/// <param name="b">����Ώۂ̋���B</param>
/// <param name="penetration">�ђʃx�N�g��</param>
/// <returns>true = �������Ă���Afalse = �������Ă��Ȃ�</returns>
bool Intersect
(
	const SPHERE& a,
	const SPHERE& b,
	vec3& penetration
)
{
	// ���S�̊Ԃ̋�����2����v�Z����
	const vec3 v = b.position - a.position;	// a�̒��~����b�̒��S�Ɍ������x�N�g��
	const float d2 = dot(v, v);				// v�̒�����2��

	// d2�����a�̍��v��蒷���ꍇ�͌������Ă��Ȃ�
	const float r = a.radius + b.radius;	// a��b�̔��a�̍��v
	// ������������邽�߁A2�擯�m�Ŕ�r����
	if (d2 > r * r)
	{
		return false;
	}

	// �������Ă���̂Ŋђʃx�N�g�������߂�
	const float d = sqrt(d2);				// �u������2��v���u�����v�ɕϊ�
	const float t = (r - d) / d;			// �u���a�̍��v - �����v�́u�����ɑ΂���䗦�v�����߂�
	penetration = v * t;					// �ђʃx�N�g�����v�Z

	// �������Ă���
	return true;
}

/// <summary>
/// AABB����_�ւ̍ŋߐړ_�����߂�
/// </summary>
/// <param name="aabb">����Ώۂ�AABB</param>
/// <param name="point">����Ώۂ̓_</param>
/// <returns>AABB�̒��ōł��_�ɋ߂����W</returns>
vec3 ClosestPoint
(
	const AABB& aabb,
	const vec3& point
)
{
	vec3 result;
	for (int i = 0; i < 3; ++i)
	{
		// �w�肳�ꂽ�͈͂ɐ������ꂽ�l��Ԃ�
		// std::clamp(���̒l, �͈͂̍ŏ��l, �͈͂̍ő�l);
		result[i] = std::clamp(point[i], aabb.min[i], aabb.max[i]);
	}

	return result;
}

/// <summary>
/// AABB�Ƌ��̂̌���������s��
/// </summary>
/// <param name="aabb">����Ώۂ�AABB</param>
/// <param name="sphere">����Ώۂ̋���</param>
/// <param name="penetration">�ђʃx�N�g��</param>
/// <returns>true = �������Ă���Afalse = �������Ă��Ȃ�</returns>
bool Intersect
(
	const AABB& aabb,
	const SPHERE& sphere,
	vec3& penetration
)
{
	// �ŋߐړ_�܂ł̋��������̂̔��a��蒷����΁A�������Ă��Ȃ�
	const vec3 p = ClosestPoint(aabb, sphere.position);
	const vec3 v = sphere.position - p;
	const float d2 = dot(v, v);
	if (d2 > sphere.radius * sphere.radius)
	{
		return false;
	}

	// �������Ă���̂ŁA�ђʃx�N�g�������߂�
	if (d2 > 0)
	{
		// ������0���傫���ꍇ�A���̂̒��S��AABB�̊O���ɂ���
		// ���̂̒��S���W����ŋߐړ_�֌�������������Փ˂����Ƃ݂Ȃ�
		const float d = sqrt(d2);
		penetration = v * ((sphere.radius - d) / d);
	}
	else
	{
		// ������0�̏ꍇ�A���̂̒��S��AABB�̓����ɂ���
		// �ђʋ������ł��Z���ʂ���Փ˂����Ƃ݂Ȃ�
		int faceIndex = 0;		  // �ђʕ����������C���f�b�N�X
		float distance = FLT_MAX; // �ђʋ���
		for (int i = 0; i < 3; ++i)
		{
			float t0 = p[i] - aabb.min[i];
			if (t0 < distance)
			{
				faceIndex = i * 2;
				distance = t0;
			}
			float t1 = aabb.max[i] - p[i];
			if (t1 < distance)
			{
				faceIndex = i * 2 + 1;
				distance = t1;
			}
		}

		// �uAABB�����̂ɑ΂��Ăǂꂾ���ђʂ��Ă��邩�v�������x�N�g�����~�����̂�
		// �ʂ̊O�����̃x�N�g�����g��
		static const vec3 faceNormals[] = {
		  {-1, 0, 0 }, { 1, 0, 0 }, // -X, +X,
		  { 0,-1, 0 }, { 0, 1, 0 }, // -Y, +Y,
		  { 0, 0,-1 }, { 0, 0, 1 }, // -Z, +Z,
		};
		penetration = faceNormals[faceIndex] * distance;
	}

	// �������Ă���
	return true;
}

/// <summary>
/// OBB����_�ւ̍ŋߐړ_�����߂�
/// </summary>
/// <param name="box">����Ώۂ�OBB</param>
/// <param name="point">����Ώۂ̓_</param>
/// <returns>�����`�̒��ōł��_�ɋ߂����W</returns>
vec3 ClosestPoint
(
	const BOX& box,
	const vec3& point
)
{
	// OBB����_�Ɍ������x�N�g��
	const vec3 v = point - box.position;

	vec3 result = box.position;
	for (int i = 0; i < 3; ++i)
	{
		// �x�N�g����OBB�̎��Ɏˉe����
		float d = dot(v, box.axis[i]);

		// �ˉe�œ���ꂽ�l��OBB�͈͓̔��ɐ�������
		d = std::clamp(d, -box.scale[i], box.scale[i]);

		// �ŋߐړ_���X�V����
		result += d * box.axis[i];
	}

	return result;
}

/// <summary>
/// OBB�Ƌ��̂̌���������s��
/// </summary>
/// <param name="box">����Ώۂ�OBB</param>
/// <param name="sphere">����Ώۂ̋���</param>
/// <param name="penetration">�ђʃx�N�g��</param>
/// <returns>true = �������Ă���Afalse = �������Ă��Ȃ�</returns>
bool Intersect
(
	const BOX& box,
	const SPHERE& sphere,
	vec3& penetration
)
{
	// �ŋߐړ_���狅�̂̒��S�܂ł̋������A���̂̔��a���傫����ΏՓ˂��Ă��Ȃ�
	const vec3 p = ClosestPoint(box, sphere.position);
	const vec3 v = sphere.position - p;
	const float d2 = dot(v, v);
	if (d2 > sphere.radius * sphere.radius)
	{
		return false;
	}

	if (d2 > 0.00001f)
	{
		// ������0���傫���ꍇ�A���̂̒��S��OBB�̊O���ɂ���
		// ���̏ꍇ�A�ŋߐڂ��狅�̂̒��S�֌�������������Փ˂����Ƃ݂Ȃ�
		const float d = sqrt(d2);
		penetration = v * ((sphere.radius - d) / d);
	}
	else
	{
		// ������0�̏ꍇ�A���̂̒��S��OBB�̓����ɂ���
		// ���̏ꍇ�A�ђʋ������ł��Z���ʂ���Փ˂����Ƃ݂Ȃ�
		const vec3 a = p - box.position; // OBB���S���狅�̒��S�ւ̃x�N�g��
		int faceIndex = 0;				 // �ђʕ����������C���f�b�N�X
		float distance = FLT_MAX;		 // �ђʋ���
		float sign = 1;					 // �ђʃx�N�g���̕���
		for (int i = 0; i < 3; ++i)
		{
			// a�����x�N�g���Ɏˉe����
			const float f = dot(box.axis[i], a);

			const float t0 = f - (-box.scale[i]);
			if (t0 < distance)
			{
				faceIndex = i;
				distance = t0;
				sign = -1;
			}
			const float t1 = box.scale[i] - f;
			if (t1 < distance)
			{
				faceIndex = i;
				distance = t1;
				sign = 1;
			}
		}
		penetration = box.axis[faceIndex] * ((distance + sphere.radius) * sign);
	}

	// �Փ˂��Ă���
	return true;
}

/// <summary>
/// �X���u(���鎲�ɐ�����2���ʂɈ͂܂ꂽ�͈�)�ƌ����̌���������s��
/// </summary>
/// <param name="min">�X���u�̊J�n����</param>
/// <param name="max">�X���u�̏I������</param>
/// <param name="start">�����̔��˓_</param>
/// <param name="direction">�����̌���</param>
/// <param name="tmin">AABB�ƌ����̌����J�n����</param>
/// <param name="tmax">AABB�ƌ����̌����I������</param>
/// <returns>true = �������Ă���Afalse = �������Ă��Ȃ�</returns>
bool IntersectSlab
(
	float min,
	float max,
	float start,
	float direction,
	float& tmin,
	float& tmax
)
{
	// �������X���u�ƕ��s�ȏꍇ
	// ���˓_���X���u���ɂ���Ό������Ă���A�O�ɂ���Ό������Ă��Ȃ�
	if (abs(direction) < 0.0001f)
	{
		return (start >= min) && (start <= max);
	}

	// �����ƃX���u����������J�n�����ƏI�����������߂�
	float t0 = (min - start) / direction;
	float t1 = (max - start) / direction;

	// �����̑��������J�n�����Ƃ���
	if (t0 > t1)
	{
		std::swap(t0, t1);
	}

	// ���ʂ̌����͈͂����߂�
	// �ȑO�̊J�n�����ƍ���̊J�n�������r���A�x���ق���I������
	if (t0 > tmin)
	{
		tmin = t0;
	}

	// ���ʂ̌����͈͂����߂�
	// �ȑO�̏I�������ƍ���̏I���������r���A�����ق���I������
	if (t1 < tmax)
	{
		tmax = t1;
	}

	//�u�J�n����<=�I�������v�̏ꍇ�͌������Ă���
	return tmin <= tmax;
}

/// <summary>
/// AABB�ƌ����̌���������s��
/// </summary>
/// <param name="aabb">����Ώۂ�AABB</param>
/// <param name="ray">����Ώۂ̌���</param>
/// <param name="distance">������AABB�ƍŏ��Ɍ������鋗��</param>
/// <returns>true = �������Ă���Afalse = �������Ă��Ȃ�</returns>
bool Intersect
(
	const AABB& aabb,
	const RAY& ray,
	float& distance
)
{
	// ���ʂ̌����͈�
	float tmin = 0;
	float tmax = FLT_MAX;

	// X�X���u�Ƃ̌���������s��
	if (!IntersectSlab(aabb.min.x, aabb.max.x, ray.start.x, ray.direction.x, tmin, tmax))
	{
		return false; // �������Ă��Ȃ�
	}

	// Y�X���u�Ƃ̌���������s��
	if (!IntersectSlab(aabb.min.y, aabb.max.y, ray.start.y, ray.direction.y, tmin, tmax))
	{
		return false; // �������Ă��Ȃ�
	}

	// Z�X���u�Ƃ̌���������s��
	if (!IntersectSlab(aabb.min.z, aabb.max.z, ray.start.z, ray.direction.z, tmin, tmax))
	{
		return false; // �������Ă��Ȃ�
	}

	//��_�܂ł̋�����ݒ肷��
	distance = tmin;
	return true;	  // �������Ă���
}

/// <summary>
/// ���̂ƌ����̌���������s��
/// </summary>
/// <param name="sphere">����Ώۂ̋���</param>
/// <param name="ray">����Ώۂ̌���</param>
/// <param name="distance">���������̂ƍŏ��Ɍ������鋗��</param>
/// <returns>true = �������Ă���Afalse = �������Ă��Ȃ�</returns>
bool Intersect
(
	const SPHERE& sphere,
	const RAY& ray,
	float& distance
)
{
	const vec3 m = ray.start - sphere.position;
	const float b = dot(m, ray.direction);
	const float c = dot(m, m) - sphere.radius * sphere.radius;

	// �����̎n�_�����̂̊O�ɂ���(c > 0)�A���������̂��痣��Ă���������
	// ���˂��ꂽ(b > 0)�ꍇ�A���̂ƌ����͌������Ȃ�
	if (c > 0 && b > 0)
	{
		return false;
	}

	// ���ʎ������̏ꍇ�͉��Ȃ�
	const float d = b * b - c; // ���ʎ�
	if (d < 0)
	{
		return false;
	}

	// �ŏ��Ɍ�������ʒu���v�Z����
	distance = -b - sqrt(d);

	// ���̈ʒu�͎n�_����O���w���A���������̓����甭�˂��ꂽ���Ƃ��Ӗ�����
	// (�n�_�����̊O�ɂ���󋵂́A���̂��痣��Ă��������̔���ŏ��O�ς�)
	// ���̏ꍇ�A�n�_���u�ŏ��Ɍ�������ʒu�v�Ƃ���
	if (distance < 0)
	{
		distance = 0;
	}

	// �������Ă���
	return true;
}

/// <summary>
/// �X���u(���鎲�ɐ�����2���ʂɈ͂܂ꂽ�͈�)�ƌ����̌���������s��
/// </summary>
/// <param name="axis">�X���u�̎��x�N�g��</param>
/// <param name="scale">�X���u�̕�</param>
/// <param name="start">�����̔��˓_(�X���u�̒��S�����_�Ƃ���)</param>
/// <param name="direction">�����̌���</param>
/// <param name="tmin">AABB�ƌ����̌����J�n����</param>
/// <param name="tmax">AABB�ƌ����̌����I������</param>
/// <returns>true = �������Ă���Afalse = �������Ă��Ȃ�</returns>
bool IntersectSlab
(
	const vec3& axis,
	float scale,
	const vec3& start,
	const vec3& direction,
	float& tmin,
	float& tmax
)
{
	// �����x�N�g���Ɣ��˓_�ɂ��āA���x�N�g�������̐��������߂�
	const float e = dot(axis, direction);
	const float f = dot(axis, start);

	// �������X���u�ƕ��s�ȏꍇ
	// ���˓_���X���u���ɂ���Ό������Ă���A�O�ɂ���Ό������Ă��Ȃ�
	if (abs(e) < 0.0001f)
	{
		return (f >= -scale) && (f <= scale);
	}

	// �����ƃX���u�������͈͂̊J�n�����ƏI�����������߂�
	float t0 = (-scale - f) / e;
	float t1 = (scale - f) / e;

	// �����̑��������J�n�����Ƃ���
	if (t0 > t1)
	{
		const float tmp = t0;
		t0 = t1;
		t1 = tmp;
	}

	// �ȑO�̊J�n�����ƍ���̊J�n�������r���A�x���ق���I������
	if (t0 > tmin)
	{
		tmin = t0;
	}

	// �ȑO�̏I�������ƍ���̏I���������r���A�����ق���I������
	if (t1 < tmax)
	{
		tmax = t1;
	}

	// �u�J�n���� <= �I�������v�̏ꍇ�͌������Ă���
	return tmin <= tmax;
}

/// <summary>
/// OBB�ƌ����̌���������s��
/// </summary>
/// <param name="box">����Ώۂ�OBB</param>
/// <param name="ray">����Ώۂ̌���</param>
/// <param name="distance">������OBB�ƍŏ��Ɍ������鋗��</param>
/// <returns>true = �������Ă���Afalse = �������Ă��Ȃ�</returns>
bool Intersect
(
	const BOX& box,
	const RAY& ray,
	float& distance
)
{
	// �X���u���S�����_�Ƃ����ꍇ�̌����̔��˓_���v�Z����
	const vec3 start = ray.start - box.position;

	// �X���u�Ƃ̌���������s��
	float tmin = 0;
	float tmax = FLT_MAX;
	for (int i = 0; i < 3; ++i)
	{
		if (!IntersectSlab(box.axis[i], box.scale[i], start, ray.direction, tmin, tmax))
		{
			return false; // �������Ă��Ȃ�
		}
	}

	// ��_�܂ł̋�����ݒ肷��
	distance = tmin;
	return true;		  // �������Ă���
}