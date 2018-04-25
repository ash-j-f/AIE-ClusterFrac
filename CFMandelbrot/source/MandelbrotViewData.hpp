#pragma once
#include "Host.h"

struct MandelbrotViewData
{

public:

	/**
	* Default constructor.
	*/
	MandelbrotViewData() { taskID = cacheEntryID = offsetX = offsetY = zoom = 0; result = nullptr; };

	/**
	* Default destructor.
	*/
	~MandelbrotViewData() {};

	//Camera offset in X dimension.
	double offsetX;

	//Camera offset in Y dimension.
	double offsetY;

	//Camera zoom
	double zoom;

	//Task ID.
	sf::Uint64 taskID;

	//Results pointer.
	cf::Result *result;

	//Cache entry id. Used to determine oldest entries.
	unsigned int cacheEntryID;

};