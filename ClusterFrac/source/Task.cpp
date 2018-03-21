#include "Task.h"
#include <array>
#include <istream>
#include <fstream>

namespace cf
{
	Task::Task()
	{
	}

	Task::Task(int newParentID)
	{
		sf::Packet p;

		taskID = generateID();

		parentID = newParentID;

		taskPartNumber = 1;

		taskPartsTotal = 1;
	}

	Task::~Task()
	{
	}
	inline std::vector<Task*> Task::split(int count)
	{
		std::vector<Task *> tmp = splitLocal(count); 
		int i = 0; 
		for (auto &t : tmp) 
		{ 
			t->taskPartNumber = i++; 
			t->taskPartsTotal = (int)tmp.size(); 
		};  
		return tmp;
	}
}
