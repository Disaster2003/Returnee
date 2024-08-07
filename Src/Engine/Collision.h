/**
* @file Collision.h
*/
#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED
#include "VecMath.h"

// 軸平行境界ボックス
struct AABB
{
	vec3 min;
	vec3 max;
};

// 球体
struct SPHERE
{
	vec3 position; // 中心座標
	float radius;  // 半径
};

// 有向境界ボックス(OBB)
struct BOX 
{
  vec3 position; // 中心座標
  vec3 axis[3];  // 軸の向きベクトル
  vec3 scale;    // 軸方向の大きさ
};

// 光線
struct RAY
{
	vec3 start;		// 光線の始点
	vec3 direction;	// 光線の向き
};

// AABB同士の交差判定を行う
bool Intersect(const AABB& a,const AABB& b,vec3& penetration);
// 球体と球体の交差判定を行う
bool Intersect(const SPHERE& a, const SPHERE& b, vec3& penetration);
// AABBと球体の交差判定を行う
bool Intersect(const AABB & a, const SPHERE & b, vec3 & penetration);
// OBBと球体の交差判定を行う
bool Intersect(const BOX & box, const SPHERE & sphere, vec3 & penetration);

// AABBと光線の交差判定を行う
bool Intersect(const AABB& a,const RAY& b,float& distance);
// 球体と光線の交差判定を行う
bool Intersect(const SPHERE & a, const RAY & b, float& distance);
// OBBと光線の交差判定を行う
bool Intersect(const BOX & a, const RAY & b, float& distance);

#endif // !COLLISION_H_INCLUDED