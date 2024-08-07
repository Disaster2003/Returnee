/**
* @file Random.cpp
*/
#include "Random.h"
#include <random>

namespace Random
{
	// ���������I�u�W�F�N�g
	std::random_device rd;
	std::mt19937 randomEngine(rd());

	/// <summary>
	/// ����������������
	/// </summary>
	/// <param name="seed">�����̎�</param>
	void Initialize(int seed)
	{
		randomEngine.seed(seed);
	}

	/// <summary>
	/// 0.0~1.0�͈̔͂̈�l�������擾����
	/// </summary>
	/// <returns>������������</returns>
	float Value()
	{
		return std::uniform_real_distribution<float>(0, 1)(randomEngine);
	}

	/// <summary>
	/// �w�肳�ꂽ�͈͂̈�l�������擾����
	/// </summary>
	/// <param name="min">�����͈͂̍ŏ��l</param>
	/// <param name="max">�����͈͂̍ő�l</param>
	/// <returns>������������</returns>
	float Range
	(
		float min,
		float max
	)
	{
		return std::uniform_real_distribution<float>(min, max)(randomEngine);
	}
}