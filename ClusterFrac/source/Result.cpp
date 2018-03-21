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

	void Result::merge(const std::vector<Result*> others)
	{
		//Sanity check the whole set is here.
		others[0]->

		mergeLocal(others);
	}
}
