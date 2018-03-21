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
		taskPartNumber = 1;
		taskPartsTotal = 1;
	}

	Task::~Task()
	{
	}

	std::vector<Task*> Task::split(int count)
	{
		std::vector<Task *> tmp = splitLocal(count); 
		int i = 0; 
		for (auto &t : tmp) 
		{ 
			t->initialTaskID = initialTaskID;
			t->taskPartNumber = i++; 
			t->taskPartsTotal = (int)tmp.size(); 
		};  
		return tmp;
	}
}
