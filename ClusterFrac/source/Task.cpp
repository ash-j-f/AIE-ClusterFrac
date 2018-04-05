#include "Task.h"
#include <array>
#include <istream>
#include <fstream>

namespace cf
{
	Task::Task()
	{
		initialTaskID = CF_ID->getNextTaskID();
		taskPartNumberStack.push_back(0);
		taskPartsTotalStack.push_back(1);
		//Default maximum time a client may spend on this task.
		maxTaskTimeMilliseconds = 1000.f;
	}

	Task::~Task()
	{
	}

	std::vector<Task*> Task::split(int count) const
	{
		std::vector<Task *> tmp = splitLocal(count); 
		sf::Uint32 i = 0;
		for (auto &t : tmp) 
		{ 
			t->initialTaskID = initialTaskID;
			t->taskPartNumberStack = taskPartNumberStack;
			t->taskPartNumberStack.push_back(i++);
			t->taskPartsTotalStack = taskPartsTotalStack;
			t->taskPartsTotalStack.push_back((sf::Uint32)tmp.size());
		};  
		return tmp;
	}

	void Task::serialize(WorkPacket & p) const
	{
		p << getType();
		p << getSubtype();
		p << initialTaskID;

		//Uint32 for best cross platform compatibility for serialisation/deserialisation.
		sf::Uint32 size = (sf::Uint32)taskPartNumberStack.size();
		p << size;
		for (sf::Uint32 i = 0; i < size; i++) p << taskPartNumberStack[i];

		size = (sf::Uint32)taskPartsTotalStack.size();
		p << size;
		for (sf::Uint32 i = 0; i < size; i++) p << taskPartsTotalStack[i];

		serializeLocal(p);
	}
	void Task::deserialize(WorkPacket & p)
	{
		p >> initialTaskID;

		//Uint32 for best cross platform compatibility for serialisation/deserialisation.
		sf::Uint32 size;
		p >> size;
		taskPartNumberStack.resize(size);
		for (sf::Uint32 i = 0; i < size; i++) p >> taskPartNumberStack[i];

		p >> size;
		taskPartsTotalStack.resize(size);
		for (sf::Uint32 i = 0; i < size; i++) p >> taskPartsTotalStack[i];

		deserializeLocal(p);
	}

	Result *Task::run() const
	{
		Result *result = runLocal();

		//Add task data to the result.

		result->initialTaskID = initialTaskID;

		result->taskPartNumberStack = taskPartNumberStack;

		result->taskPartsTotalStack = taskPartsTotalStack;

		return result;
	}
}
