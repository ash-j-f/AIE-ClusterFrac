#pragma once
#include <string>
#include <ClusterFrac.h>

class BenchmarkTask : public cf::Task
{
public:
	BenchmarkTask();
	~BenchmarkTask();

	inline std::string get_subtype() const { return "BenchmarkTask";  };

	std::vector<cf::Task *> split(int count) const;
};