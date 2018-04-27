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
	~BenchmarkTask() {};

	//Test data set.
	std::vector<float> numbers;

	/**
	* Get the subtype of this task.
	* @returns The subtype of this task.
	*/
	inline std::string getSubtype() const { return "BenchmarkTask"; };

private:


	inline std::vector<cf::Task *> splitLocal(unsigned int count) const override
	{
		//Limit number of tasks to at least number of target numbers.
		if ((int)numbers.size() < count) count = (int)numbers.size();

		std::vector<BenchmarkTask *> tasks = std::vector<BenchmarkTask *>();
		tasks.resize(count);

		for (unsigned int i = 0; i < count; i++)
		{
			tasks[i] = new BenchmarkTask();
		}

		//Distribute numbers among the new BenchmarkTasks.
		const int step = (int)floor((float)numbers.size() / (float)count);
		int start = 0;
		int end = step;
		for (unsigned int i = 0; i < count; i++)
		{
			//If this is the final split, then get the remainder of items.
			if (i == count - 1) end = (int)numbers.size();

			tasks[i]->numbers.insert(tasks[i]->numbers.begin(), numbers.begin() + start, numbers.begin() + std::min(end, (int)numbers.size()));

			start = start + step;
			end = end + step;
		}

		//Convert BenchmarkTask pointers to Task pointers.
		std::vector<cf::Task *> tasksConv;
		for (auto &task : tasks) tasksConv.push_back((Task *)task);

		return tasksConv;
	};

	inline void serializeLocal(cf::WorkPacket &p) const override
	{
		sf::Int64 size = numbers.size();
		p << size;
		for (sf::Int64 i = 0; i < size; i++) p << numbers[i];
	};

	inline void deserializeLocal(cf::WorkPacket &p) override
	{
		sf::Int64 size;
		p >> size;
		numbers.resize(size);
		for (sf::Int64 i = 0; i < size; i++) p >> numbers[i];
	};

	inline cf::Result *runLocal() const override
	{
		BenchmarkResult *result = new BenchmarkResult();

		for (auto &n : numbers) result->numbers.push_back(sqrtf(n));

		return result;
	};

};