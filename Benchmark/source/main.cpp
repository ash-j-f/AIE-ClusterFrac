#include <ClusterFrac.h>
#include "BenchmarkTask.h" 

int main()
{
		BenchmarkTask *bmt1 = new BenchmarkTask();
		BenchmarkTask *bmt2 = new BenchmarkTask();

		bmt1->numbers = { 1, 2, 3 };
		bmt2->numbers = { 4, 5, 6 };

		sf::Packet p;
		bmt1->serialize(p);
		bmt2->deserialize(p);

		bmt1->serialize(p);

		//p << i.data()[0];
		//p >> o.data()[0];
	
}