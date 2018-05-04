#include "Result.h"

namespace cf
{
	Result::Result()
	{
		initialTaskID = 0;
		taskPartNumberStack.push_back(0);
		taskPartsTotalStack.push_back(1);

		//Default network compression status for a new result.
		compression = false;
	}

	Result::~Result()
	{
	}

	void Result::merge(std::vector<Result*> others)
	{
		//Sanity check the whole set is here.
		sf::Uint64 iID = others[0]->initialTaskID;
		size_t taskPartNumberStackSize = others[0]->taskPartNumberStack.size();
		size_t taskPartsTotalStackSize = others[0]->taskPartsTotalStack.size();
		sf::Uint32 taskPartsTotal = others[0]->taskPartsTotalStack.back();
		if (others.size() != (size_t)taskPartsTotal) CF_THROW("Cannot merge results. Incorrect number of results to merge for this set.");
		for (auto &r : others)
		{
			if (
				iID != r->initialTaskID
				||
				taskPartNumberStackSize != r->taskPartNumberStack.size()
				||
				taskPartsTotalStackSize != r->taskPartsTotalStack.size()
				||
				taskPartsTotal != r->taskPartsTotalStack.back()
				)
			{
				CF_THROW("Cannot merge results. Results are not from same set or some are missing from the set.");
			}

			if (r->taskPartsTotalStack.size() < 2) CF_THROW("Cannot merge results. One or more tasks are already fully merged.");

			if (taskPartNumberStackSize != taskPartsTotalStackSize) CF_THROW("Cannot merge results. One or more tasks have mismatched task part number and task part total counts.");
		}

		//Inherit task data from the results.
		initialTaskID = iID;
		taskPartNumberStack = others[0]->taskPartNumberStack;
		taskPartNumberStack.pop_back(); //Unwind the stack by one.
		taskPartsTotalStack = others[0]->taskPartsTotalStack;
		taskPartsTotalStack.pop_back(); //Unwind the stack by one.

		//Order of results must be preserved. Reorder parts by part number before passing them to local merge.
		std::sort(others.begin(), others.end(), 
			[](cf::Result *a, cf::Result *b) 
			{ 
				return (a->getTaskPartNumber() < b->getTaskPartNumber());
			}
		);

		mergeLocal(others);
	}

	void Result::serialize(cf::WorkPacket &p) const
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
	void Result::deserialize(cf::WorkPacket & p)
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
}
