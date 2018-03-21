#include "Task.h"
#include <array>
#include <istream>
#include <fstream>

namespace cf
{
	Task::Task()
	{
		taskID = CF_ID->getNextTaskID();
		initialTaskID = taskID;
		taskPartNumber = 0;
		taskPartsTotal = 1;
	}

	Task::Task(unsigned int newInitialTaskID, unsigned int newTaskPartNumber, unsigned int newTaskPartsTotal)
	{
		taskID = CF_ID->getNextTaskID();
		initialTaskID = newInitialTaskID;
		taskPartNumber = newTaskPartNumber;
		taskPartsTotal = newTaskPartsTotal;
	}

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
}
