#include <ClusterFrac.h>
#include "BenchmarkTask.h" 
#include "BenchmarkResult.h" 

int main()
{
		BenchmarkTask *bmt1 = new BenchmarkTask();

		for (int i = 0; i < 1000000; i++) bmt1->numbers.push_back((float)rand() / 10.0f);

		std::vector<cf::Task *> splitTasks = bmt1->split(8);

		std::vector<cf::Result *> resultList;
		for (auto task : splitTasks) resultList.push_back(task->run());

		cf::Result *mergedResults = resultList[0]->merge(resultList);

		//cf::Result *mergedResults = bmr1->merge(std::vector<cf::Result *> { bmr1, bmr2 });

		cf::WorkPacket p;
		//splitTasks[0]->serialize(p);
		//
		////Determine type here.
		//std::string type;
		//p >> type;
		//
		//bmt2->deserialize(p);

		bmt1->serialize(p);

	
}