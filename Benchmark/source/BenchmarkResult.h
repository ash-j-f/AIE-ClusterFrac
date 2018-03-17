#pragma once
#include <string>
#include <ClusterFrac.h>

class BenchmarkResult : public cf::Result
{
public:
	BenchmarkResult();
	~BenchmarkResult();

	std::vector<float> numbers;

	inline std::string getSubtype() const { return "BenchmarkResult";  };

	virtual cf::Result *merge(const std::vector<cf::Result *> others) const;

	void serializeLocal(cf::WorkPacket &p);

	void deserializeLocal(cf::WorkPacket &p);
};