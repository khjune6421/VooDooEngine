#pragma once
#include <random>
#include "Singleton.h"

class RandomNumberGenerator : public Singleton<RandomNumberGenerator>
{
	friend class Singleton<RandomNumberGenerator>;
	RandomNumberGenerator() { std::random_device rd; m_generator.seed(rd()); }
	std::mt19937 m_generator;

public:
	template<std::integral T>
	T GetRandom(T min, T max)
	{
		const std::uniform_int_distribution<T> dist(min, max);
		return dist(m_generator);
	}

	template<std::floating_point T>
	T GetRandom(T min, T max)
	{
		const std::uniform_real_distribution<T> dist(min, max);
		return dist(m_generator);
	}
};