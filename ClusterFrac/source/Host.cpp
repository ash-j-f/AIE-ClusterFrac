#include "Host.h"

namespace cf
{
	Host::Host()
	{
		//Default port number.
		port = 5000;
	}
	
	Host::~Host()
	{
	}

	void Host::start()
	{
		//Initialise incoming connection listener.
		listener.listen(port);
		//Add the listener to the selector
		selector.add(listener);
	}

}