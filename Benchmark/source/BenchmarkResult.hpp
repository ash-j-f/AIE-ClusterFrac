#pragma once
#include <string>
#include <Result.h>

/**
* Benchmark test result class.
* Derived from ClusterFrac library Result class.
* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
*/
class BenchmarkResult : public cf::Result
{
public:

	/**
	* Default constructor.
	*/
	BenchmarkResult() {};
	
	/**
	* Default destructor.
	*/
	~BenchmarkResult() {};

	//Test data set.
	std::vector<float> numbers;

	/**
	* Get the subtype of this result.
	* @returns The subtype of this result.
	*/
	inline std::string getSubtype() const { return "BenchmarkResult"; };

private:

	inline virtual void mergeLocal(const std::vector<cf::Result *> others)
	{

		//Insert the other results into the new merged result.
		for (auto &r : others)
		{
			numbers.insert(numbers.end(), ((BenchmarkResult *)r)->numbers.begin(), ((BenchmarkResult *)r)->numbers.end());
		}

	};

	inline void serializeLocal(cf::WorkPacket &p) const
	{
		sf::Int64 size = numbers.size();
		p << size;
		for (sf::Int64 i = 0; i < size; i++) p << numbers[i];
	};

	inline void deserializeLocal(cf::WorkPacket &p)
	{
		sf::Int64 size;
		p >> size;
		numbers.resize(size);
		for (sf::Int64 i = 0; i < size; i++) p >> numbers[i];
	};
};