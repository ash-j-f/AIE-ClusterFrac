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
