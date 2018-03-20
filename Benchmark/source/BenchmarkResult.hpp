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

	virtual void merge(const std::vector<cf::Result *> others)
	{

		//Insert the other results into the new merged result.
		for (auto &r : others)
		{
			numbers.insert(numbers.end(), ((BenchmarkResult *)r)->numbers.begin(), ((BenchmarkResult *)r)->numbers.end());
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