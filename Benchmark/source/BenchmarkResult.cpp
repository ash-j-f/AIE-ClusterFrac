#include "BenchmarkResult.h"

BenchmarkResult::BenchmarkResult()
{
}

BenchmarkResult::~BenchmarkResult()
{
}

cf::Result *BenchmarkResult::merge(const std::vector<cf::Result*> others) const
{
	BenchmarkResult *tmpResult = new BenchmarkResult();

	//Insert our results into the new merged result.
	tmpResult->numbers.insert(tmpResult->numbers.begin(), numbers.begin(), numbers.end());

	//Insert the other results into the new merged result.
	for (auto &r : others)
	{
		//Skip ourself, in case the list of others contains this results chunk too.
		if (r == this) continue;

		tmpResult->numbers.insert(tmpResult->numbers.begin(), ((BenchmarkResult *)r)->numbers.begin(), ((BenchmarkResult *)r)->numbers.end());
	}
	return (Result *)tmpResult;
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
