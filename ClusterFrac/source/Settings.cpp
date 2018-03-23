#include "Settings.h"

namespace cf
{
	Settings::Settings()
	{
		//Defaults
		logLevel = LogLevels::Info;
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