#pragma once
#include <string>
#include <ClusterFrac.h>

class BenchmarkResult : public cf::Result
{
public:
	BenchmarkResult() {};
	~BenchmarkResult() {};

	std::vector<float> numbers;

	inline std::string getSubtype() const { return "BenchmarkResult"; };

	virtual cf::Result *merge(const std::vector<cf::Result *> others) const
	{
		{
			BenchmarkResult *tmpResult = new BenchmarkResult();

			//Insert our results into the new merged result.
			tmpResult->numbers.insert(tmpResult->numbers.begin(), numbers.begin(), numbers.end());

			//Insert the other results into the new merged result.
			for (auto &r : others)
			{
				//Skip ourself, in case the list of others contains this results chunk too.
				if (r == this) continue;

				tmpResult->numbers.insert(tmpResult->numbers.end(), ((BenchmarkResult *)r)->numbers.begin(), ((BenchmarkResult *)r)->numbers.end());
			}
			return (Result *)tmpResult;
		}
	};

	void serializeLocal(cf::WorkPacket &p)
	{
		sf::Int64 size = numbers.size();
		p << size;
		for (sf::Int64 i = 0; i < size; i++) p << numbers[i];
	};

	void deserializeLocal(cf::WorkPacket &p)
	{
		sf::Int64 size;
		p >> size;
		numbers.resize(size);
		for (sf::Int64 i = 0; i < size; i++) p >> numbers[i];
	};
};