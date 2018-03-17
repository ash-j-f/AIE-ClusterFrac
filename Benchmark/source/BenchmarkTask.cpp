#include "BenchmarkTask.h"

BenchmarkTask::BenchmarkTask()
{
}

BenchmarkTask::~BenchmarkTask()
{
}

std::vector<cf::Task *> BenchmarkTask::split(int count) const
{
	//Limit number of tasks to at least number of target numbers.
	if ((int)numbers.size() < count) count = (int)numbers.size();

	std::vector<BenchmarkTask *> tasks = std::vector<BenchmarkTask *>();
	tasks.resize(count);

	for (int i = 0; i < count; i++) tasks[i] = new BenchmarkTask();

	//Distribute numbers among the new BenchmarkTasks.
	const int step = (int)floor((float)numbers.size() / (float)count);
	int start = 0;
	int end = step;
	for (int i = 0; i < count; i++)
	{	
		//If this is the final split, then get the remainder of items.
		if (i == count - 1) end = (int)numbers.size();

		tasks[i]->numbers.insert(tasks[i]->numbers.begin(), numbers.begin() + start, numbers.begin() + std::min(end, (int)numbers.size()));
		
		start = start + step;
		end = end + step;
	}

	//Convert BenchmarkTask pointers to Task pointers.
	std::vector<cf::Task *> tasksConv;
	for (auto &task : tasks) tasksConv.push_back((Task *) task);

	return tasksConv;
}

void BenchmarkTask::serializeLocal(cf::WorkPacket &p)
{
	sf::Int64 size = numbers.size();
	p << size;
	for (sf::Int64 i = 0; i < size; i++) p << numbers[i];
}

void BenchmarkTask::deserializeLocal(cf::WorkPacket & p)
{
	sf::Int64 size;
	p >> size;
	numbers.resize(size);
	for (sf::Int64 i = 0; i < size; i++) p >> numbers[i];
}

cf::Result *BenchmarkTask::run()
{
	BenchmarkResult *tmpResult = new BenchmarkResult();
	
	for (auto &n : numbers) tmpResult->numbers.push_back(sqrtf(n));

	return tmpResult;
}