#pragma once

#ifdef DLL_CONFIG
#define DLL __declspec(dllexport)
#else
#define DLL
#endif