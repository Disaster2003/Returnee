/**
* @file Collision.h
*/
#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED
#include "VecMath.h"

// ²½s«E{bNX
struct AABB
{
	vec3 min;
	vec3 max;
};

// Ì
struct SPHERE
{
	vec3 position; // SÀW
	float radius;  // ¼a
};

// Lü«E{bNX(OBB)
struct BOX 
{
  vec3 position; // SÀW
  vec3 axis[3];  // ²Ìü«xNg
  vec3 scale;    // ²ûüÌå«³
};

// õü
struct RAY
{
	vec3 start;		// õüÌn_
	vec3 direction;	// õüÌü«
};

// AABB¯mÌð·»èðs¤
bool Intersect(const AABB& a,const AABB& b,vec3& penetration);
// ÌÆÌÌð·»èðs¤
bool Intersect(const SPHERE& a, const SPHERE& b, vec3& penetration);
// AABBÆÌÌð·»èðs¤
bool Intersect(const AABB & a, const SPHERE & b, vec3 & penetration);
// OBBÆÌÌð·»èðs¤
bool Intersect(const BOX & box, const SPHERE & sphere, vec3 & penetration);

// AABBÆõüÌð·»èðs¤
bool Intersect(const AABB& a,const RAY& b,float& distance);
// ÌÆõüÌð·»èðs¤
bool Intersect(const SPHERE & a, const RAY & b, float& distance);
// OBBÆõüÌð·»èðs¤
bool Intersect(const BOX & a, const RAY & b, float& distance);

#endif // !COLLISION_H_INCLUDED