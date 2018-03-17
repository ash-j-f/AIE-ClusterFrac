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

	virtual Result *merge(const std::vector<Result *> others) const;

	void serializeLocal(cf::WorkPacket &p);

	void deserializeLocal(cf::WorkPacket &p);
};