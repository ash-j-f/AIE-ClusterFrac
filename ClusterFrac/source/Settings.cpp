#include "Settings.h"

namespace cf
{
	Settings::Settings()
	{
		//Defaults
		debug = false;
	}

	Settings::~Settings()
	{
	}

	Settings *Settings::getInstance()
	{
		static Settings settings;

		return &settings;
	}
}