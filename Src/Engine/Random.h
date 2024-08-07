/**
* @file Random.h
*/
#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED

// 乱数生成機能を格納する名前空間
namespace Random
{
	// 乱数を初期化する
	void Initialize(int seed);

	// 0.0~1.0の範囲の一様乱数を取得する
	float Value();

	// 指定された範囲の一様乱数を取得する
	float Range(float min, float max);
}
#endif // !RANDOM_H_INCLUDED