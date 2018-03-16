#pragma once
#include <string>
#include <ClusterFrac.h>

class BenchmarkTask : public cf::Task
{
public:
	BenchmarkTask();
	~BenchmarkTask();

	std::vector<float> numbers;

	inline std::string getSubtype() const { return "BenchmarkTask";  };

	std::vector<cf::Task *> split(int count) const;

	void serializeLocal(sf::Packet &p);

	void deserializeLocal(sf::Packet &p);
};