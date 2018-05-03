#pragma once
#include <string>
#include <stdio.h>
#include <Windows.h>
#include <vector>
#include <ConsoleMessager.hpp>

/**
* Benchmark statistics manager for use with the Benchmark reference application 
* and the ClusterFrac library host class.
* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
*/

class BenchmarkStatManager
{

public:

	/**
	* Default constructor.
	*/
	BenchmarkStatManager();
	
	/**
	* Default destructor.
	*/
	~BenchmarkStatManager();

	/**
	* Save benchmark statistical data to disk.
	* @returns void.
	*/
	void save() const;

	/**
	* Load benchmark statistical data from disk.
	* @returns void.
	*/
	void load();

	/**
	* Record a test run computation and network time statistic.
	* Time is in milliseconds.
	* @returns void.
	*/
	inline void addStat(double timeMilliseconds) { stats.push_back(timeMilliseconds); save(); };

	/**
	* Get the average time of all tests.
	* @returns The average time of all tests.
	*/
	inline double getAverage() const { double size = (double)stats.size(); if (size == 0) return 0; double avg = 0; for (auto &s : stats) avg += s; avg /= size; return avg; };

	/**
	* Get the number of test statistics entries.
	* @returns The number of test statistics entries.
	*/
	inline unsigned int getCount() const { return (unsigned int)stats.size(); };

private:

	//Time data entries, in seconds per total network and computation time for each test run.
	std::vector<double> stats;

	/**
	* Get the full path to the location of this executable file.
	* @returns The full path to the location of this executable file.
	*/
	std::string getExecutableFolder() const;

};