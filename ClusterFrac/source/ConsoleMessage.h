#pragma once
#include <iostream>
#include "Settings.h"
#define CF_SAY(x) if (CF_SETTINGS->getDebug()==true) std::cout << x << std::endl;