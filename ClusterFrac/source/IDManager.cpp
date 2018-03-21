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
		nextClientID = 0;
	}

	IDManager::~IDManager()
	{
	}
}
