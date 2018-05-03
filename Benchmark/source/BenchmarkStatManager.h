#pragma once
#include <string>
#include <stdio.h>
#include <Windows.h>
#include <ConsoleMessager.hpp>

/**
* Benchmark statistics manager for use with the Benchmark reference application and the 
* ClusterFrac library host class.
* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
*/

class BenchmarkStatManager
{

public:

	BenchmarkStatManager();

	~BenchmarkStatManager();

	void save() const;

	void load();

private:

	/**
	* Get the full path to the location of this executable file.
	* @returns The full path to the location of this executable file.
	*/
	std::string getExecutableFolder() const;

};