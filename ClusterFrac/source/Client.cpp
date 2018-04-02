#include "Client.h"

cf::Client::Client()
{
	//Default port number
	port = 5000;
}

cf::Client::~Client()
{
}

void cf::Client::start()
{
}

void cf::Client::setPort(int portNum)
{
	if (portNum > 0 && portNum <= 65535)
	{
		port = portNum;
	}
	else
	{
		std::string s = "Invalid port number.";
		CF_SAY(s, Settings::LogLevels::Error);
		throw s;
	};
}
