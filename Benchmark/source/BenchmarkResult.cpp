#include "BenchmarkResult.h"

BenchmarkResult::BenchmarkResult()
{
}

BenchmarkResult::~BenchmarkResult()
{
}


cf::Result * BenchmarkResult::merge(const std::vector<Result*> others) const
{
	return nullptr;
}

void BenchmarkResult::serializeLocal(cf::WorkPacket &p)
{
	sf::Int64 size = numbers.size();
	p << size;
	for (sf::Int64 i = 0; i < size; i++) p << numbers[i];
}

void BenchmarkResult::deserializeLocal(cf::WorkPacket & p)
{
	sf::Int64 size;
	p >> size;
	numbers.resize(size);
	for (sf::Int64 i = 0; i < size; i++) p >> numbers[i];
}
