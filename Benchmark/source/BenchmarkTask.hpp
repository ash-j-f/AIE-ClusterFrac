#pragma once
#include <string>
#include <Task.h>
#include "BenchmarkResult.hpp"

/**
* Benchmark test task class.
* Derived from ClusterFrac library Task class.
* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
*/
class BenchmarkTask : public cf::Task
{
public:

	/**
	* Default constructor.
	*/
	BenchmarkTask() {};

	/**
	* Default destructor.
	*/
	~BenchmarkTask() { dataRangeStart = dataRangeEnd = cycles = 0; };

	//Test data set.
	sf::Uint32 dataRangeStart;
	sf::Uint32 dataRangeEnd;
	sf::Uint32 cycles;

	/**
	* Get the subtype of this task.
	* @returns The subtype of this task.
	*/
	inline std::string getSubtype() const { return "BenchmarkTask"; };

private:

	/**
	* Split this task up as equally as possible in to N chunks, and return
	* a std::vector of pointers to those split tasks.
	* @param count Split the task into this many subtasks.
	* @returns A std::vector of pointers to the new split tasks.
	*/
	inline std::vector<cf::Task *> splitLocal(unsigned int count) const override
	{
		//Limit number of tasks to at least number of target numbers.
		unsigned int valueCount = (dataRangeEnd - dataRangeStart) + 1;
		if (valueCount < count) count = valueCount;

		std::vector<BenchmarkTask *> tasks = std::vector<BenchmarkTask *>();
		tasks.resize(count);

		for (unsigned int i = 0; i < count; i++)
		{
			tasks[i] = new BenchmarkTask();
		}

		//Distribute numbers among the new tasks.
		const unsigned int step = (int)floor(valueCount / (float)count);
		unsigned int start = dataRangeStart;
		unsigned int end = dataRangeStart + (step - 1);
		for (unsigned int i = 0; i < count; i++)
		{
			//If this is the final split, then get the remainder of items.
			if (i == count - 1) end = dataRangeStart + valueCount;

			tasks[i]->dataRangeStart = start;
			tasks[i]->dataRangeEnd = std::min(end, dataRangeStart + (valueCount - 1));
			tasks[i]->cycles = cycles;

			start = start + step;
			end = end + step;
		}

		//Convert BenchmarkTask pointers to Task pointers.
		std::vector<cf::Task *> tasksConv;
		for (auto &task : tasks) tasksConv.push_back((Task *)task);

		return tasksConv;
	};

	/**
	* Serialize this task and store the data in a given packet.
	* @param p The packet to store the data in.
	* @returns void.
	*/
	inline void serializeLocal(cf::WorkPacket &p) const override
	{
		p << dataRangeStart;
		p << dataRangeEnd;
		p << cycles;
	};

	/**
	* Deserialize this task from data provided by a packet.
	* @param p The packet to retrieve the task data from.
	* @returns void.
	*/
	inline void deserializeLocal(cf::WorkPacket &p) override
	{
		p >> dataRangeStart;
		p >> dataRangeEnd;
		p >> cycles;
	};

	/**
	* Run the task and produce a results object.
	* @returns A pointer to the new results object.
	*/
	inline cf::Result *runLocal() const override
	{
		BenchmarkResult *result = new BenchmarkResult();

		for (unsigned int i = dataRangeStart; i <= dataRangeEnd; i++)
		{
			double r = i;
			for (unsigned int c = 0; c < cycles; c++)
			{
				r = (double)sqrtl((double)r);
			}
			result->numbers.push_back(r);
		}

		return result;
	};

};