#include "IDManager.h"

namespace cf
{
	IDManager * IDManager::getInstance()
	{
		static IDManager id;

		return &id;
	}

	IDManager::IDManager()
	{
		//Initialise IDs at 1, as 0 indicates the value is unset.

		nextClientID = 1;

		nextTaskID = 1;

		nextResultID = 1;
	}

	IDManager::~IDManager()
	{
	}
}
