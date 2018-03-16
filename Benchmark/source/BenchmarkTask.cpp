#include "BenchmarkTask.h"

BenchmarkTask::BenchmarkTask()
{
}

BenchmarkTask::~BenchmarkTask()
{
}

std::vector<cf::Task *> BenchmarkTask::split(int count) const
{
	return std::vector<cf::Task *>();
}

void BenchmarkTask::serializeLocal(sf::Packet &p)
{
	sf::Int64 size = numbers.size();
	p << size;
	for (sf::Int64 i = 0; i < size; i++) p << numbers[i];
}

void BenchmarkTask::deserializeLocal(sf::Packet & p)
{
	sf::Int64 size;
	p >> size;
	numbers.resize(size);
	for (sf::Int64 i = 0; i < size; i++) p >> numbers[i];
}
