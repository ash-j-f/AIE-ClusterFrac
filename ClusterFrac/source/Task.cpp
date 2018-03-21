#include "Task.h"
#include <array>
#include <istream>
#include <fstream>

namespace cf
{
	Task::Task()
	{
		initialTaskID = CF_ID->getNextTaskID();;
		taskPartNumberStack.push_back(0);
		taskPartsTotalStack.push_back(1);
	}

	//Task::Task(unsigned int newInitialTaskID, unsigned int newTaskPartNumber, unsigned int newTaskPartsTotal)
	//{
	//	taskID = CF_ID->getNextTaskID();
	//	initialTaskID = newInitialTaskID;
	//	taskPartNumber = newTaskPartNumber;
	//	taskPartsTotal = newTaskPartsTotal;
	//}

	Task::~Task()
	{
	}

	std::vector<Task*> Task::split(int count)
	{
		std::vector<Task *> tmp = splitLocal(count); 
		sf::Uint32 i = 0;
		for (auto &t : tmp) 
		{ 
			t->initialTaskID = initialTaskID;
			t->taskPartNumber = i++; 
			t->taskPartsTotal = (sf::Uint32)tmp.size();
		};  
		return tmp;
	}

	void Task::serialize(WorkPacket & p)
	{
		p << getType();
		p << getSubtype();
		p << initialTaskID;

		sf::Uint32 size = taskPartNumberStack.size();
		p << size;
		for (sf::Uint32 i = 0; i < size; i++) p << taskPartNumberStack[i];

		size = taskPartsTotalStack.size();
		p << size;
		for (sf::Uint32 i = 0; i < size; i++) p << taskPartsTotalStack[i];

		serializeLocal(p);
	}
	void Task::deserialize(WorkPacket & p)
	{
		p >> initialTaskID;

		sf::Uint32 size;
		p >> size;
		taskPartNumberStack.resize(size);
		for (sf::Uint32 i = 0; i < size; i++) p >> taskPartNumberStack[i];

		p >> size;
		taskPartsTotalStack.resize(size);
		for (sf::Uint32 i = 0; i < size; i++) p >> taskPartsTotalStack[i];

		deserializeLocal(p);
	}
}
