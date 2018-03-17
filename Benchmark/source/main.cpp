#include <ClusterFrac.h>
#include "BenchmarkTask.h" 
#include "BenchmarkResult.h" 

int main()
{
		BenchmarkTask *bmt1 = new BenchmarkTask();
		BenchmarkTask *bmt2 = new BenchmarkTask();

		for (int i = 0; i < 1000000; i++) bmt1->numbers.push_back((float)rand() / 10.0f);
		
		bmt2->numbers = { 99, 100, 101 };

		std::vector<cf::Task *> splitTasks = bmt1->split(99);

		BenchmarkResult *bmr1 = new BenchmarkResult();
		for (int i = 0; i < 1000000; i++) bmr1->numbers.push_back((float)rand() / 10.0f);
		BenchmarkResult *bmr2 = new BenchmarkResult();
		for (int i = 0; i < 1000000; i++) bmr2->numbers.push_back((float)rand() / 10.0f);
		cf::Result *mergedResults = bmr1->merge(std::vector<cf::Result *> { bmr1, bmr2 });

		cf::WorkPacket p;
		splitTasks[0]->serialize(p);
		
		//Determine type here.
		std::string type;
		p >> type;
		
		bmt2->deserialize(p);

		bmt1->serialize(p);

		//p << i.data()[0];
		//p >> o.data()[0];
	
}