#include <ClusterFrac.h>
#include "BenchmarkTask.h" 

int main()
{
		BenchmarkTask *bmt1 = new BenchmarkTask();
		BenchmarkTask *bmt2 = new BenchmarkTask();

		bmt1->numbers = { 1, 2, 3 };
		
		bmt2->numbers = { 4, 5, 6 };

		std::vector<cf::Task *> splitTasks = bmt1->split(2);

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