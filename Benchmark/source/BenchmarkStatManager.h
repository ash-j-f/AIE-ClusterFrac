#pragma once
#include <string>
#include <stdio.h>
#include <Windows.h>
#include <vector>
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

	/**
	* Record a test run computation and network time.
	* Time is in milliseconds.
	* @returns void.
	*/
	inline void addStat(double timeMilliseconds) { stats.push_back(timeMilliseconds); save(); };

private:

	//Time data entries, in seconds per total network and computation time for each test run.
	std::vector<double> stats;

	/**
	* Get the full path to the location of this executable file.
	* @returns The full path to the location of this executable file.
	*/
	std::string getExecutableFolder() const;

};