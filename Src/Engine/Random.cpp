/**
* @file Random.cpp
*/
#include "Random.h"
#include <random>

namespace Random
{
	// 乱数生成オブジェクト
	std::random_device rd;
	std::mt19937 randomEngine(rd());

	/// <summary>
	/// 乱数を初期化する
	/// </summary>
	/// <param name="seed">乱数の種</param>
	void Initialize(int seed)
	{
		randomEngine.seed(seed);
	}

	/// <summary>
	/// 0.0~1.0の範囲の一様乱数を取得する
	/// </summary>
	/// <returns>生成した乱数</returns>
	float Value()
	{
		return std::uniform_real_distribution<float>(0, 1)(randomEngine);
	}

	/// <summary>
	/// 指定された範囲の一様乱数を取得する
	/// </summary>
	/// <param name="min">生成範囲の最小値</param>
	/// <param name="max">生成範囲の最大値</param>
	/// <returns>生成した乱数</returns>
	float Range
	(
		float min,
		float max
	)
	{
		return std::uniform_real_distribution<float>(min, max)(randomEngine);
	}
}