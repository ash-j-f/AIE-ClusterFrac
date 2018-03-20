#include "Settings.h"

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