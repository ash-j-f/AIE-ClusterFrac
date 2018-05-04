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
	std::vector<double> numbers;

	/**
	* Get the subtype of this result.
	* @returns The subtype of this result.
	*/
	inline std::string getSubtype() const override { return "BenchmarkResult"; };

private:

	/**
	* Merge other results in a std::vector into this result.
	* Overrides virtual function in base class.
	* @param others A std::vector of pointers to the all results in a set to merge with this one.
	* @returns void.
	*/
	inline void mergeLocal(const std::vector<cf::Result *> others) override
	{

		//Insert the other results into the new merged result.
		for (auto &r : others)
		{
			numbers.insert(numbers.end(), ((BenchmarkResult *)r)->numbers.begin(), ((BenchmarkResult *)r)->numbers.end());
		}

	};

	/**
	* Serialize this result and store the data in a given packet.
	* Overrides virtual function in base class.
	* @param p The packet to store the data in.
	* @returns void.
	*/
	inline void serializeLocal(cf::WorkPacket &p) const override
	{
		sf::Int64 size = numbers.size();
		p << size;
		for (sf::Int64 i = 0; i < size; i++) p << numbers[i];
	};

	/**
	* Deserialize this result from data provided by a packet.
	* Overrides virtual function in base class.
	* @param p The packet to retrieve the result data from.
	* @returns void.
	*/
	inline void deserializeLocal(cf::WorkPacket &p) override
	{
		sf::Int64 size;
		p >> size;
		numbers.resize(size);
		for (sf::Int64 i = 0; i < size; i++) p >> numbers[i];
	};
};