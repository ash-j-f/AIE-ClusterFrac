#include "Result.h"

namespace cf
{
	Result::Result()
	{
		initialTaskID = 0;
	}

	Result::~Result()
	{
	}

	void Result::merge(std::vector<Result*> others)
	{
		//Sanity check the whole set is here.
		sf::Uint32 iID = others[0]->initialTaskID;
		size_t taskPartNumberStackSize = others[0]->taskPartNumberStack.size();
		size_t taskPartsTotalStackSize = others[0]->taskPartsTotalStack.size();
		sf::Uint32 taskPartsTotal = others[0]->taskPartsTotalStack.back();
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
				throw "Cannot merge results. Tasks are not from same set or some are missing.";
			}
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
}
