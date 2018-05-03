#include "BenchmarkStatManager.h"

BenchmarkStatManager::BenchmarkStatManager()
{
	load();
}

BenchmarkStatManager::~BenchmarkStatManager()
{
	save();
}

void BenchmarkStatManager::save() const
{
	FILE *pFile;

	fopen_s(&pFile, (getExecutableFolder() + "\\" + "benchmarkstats.bin").c_str(), "wb");

	if (!pFile) return;

	bool closed = false;

	try
	{
		//Write total stats count.
		size_t size = stats.size();
		fwrite(&size, sizeof(char), sizeof(size), pFile);
		
		//Write each stat entry.
		for (auto &s : stats)
		{
			fwrite(&s, sizeof(char), sizeof(s), pFile);
		}

		CF_SAY("Saved benchmark statistics to file benchmarkstats.bin.", cf::Settings::LogLevels::Info);
	}
	catch (...)
	{
		fclose(pFile);
		closed = true;
		//Invalid data in file. Proceed with default values.
		CF_SAY("Unable to write benchmark stats file " + getExecutableFolder() + "\\"
			+ "benchmarkstats.bin.", cf::Settings::LogLevels::Error);
	}

	if (!closed) fclose(pFile);
}

void BenchmarkStatManager::load()
{
	//Clear any curent stat entries.
	stats.clear();

	FILE *pFile;

	fopen_s(&pFile, (getExecutableFolder() + "\\" + "benchmarkstats.bin").c_str(), "rb");

	if (!pFile) return;

	bool closed = false;

	try
	{
		size_t byteSize;
		size_t size;
		double newStatEntry;

		//Read number of entries in the file.
		byteSize = fread(&size, sizeof(char), sizeof(size), pFile);
		if (byteSize != sizeof(size)) throw "Invalid data.";

		//Read entries into the object vector.
		for (size_t i = 0; i < size; i++)
		{
			byteSize = fread(&newStatEntry, sizeof(char), sizeof(newStatEntry), pFile);
			if (byteSize != sizeof(newStatEntry)) throw "Invalid data.";
			stats.push_back(newStatEntry);
		}

		CF_SAY("Loaded benchmark statistics from file benchmarkstats.bin.", cf::Settings::LogLevels::Info);
	}
	catch (...)
	{
		//Invalid data in file. Proceed with default values.
		CF_SAY("Invalid benchmark stats file " + getExecutableFolder() + "\\"
			+ "benchmarkstats.bin.", cf::Settings::LogLevels::Error);
		fclose(pFile);
		closed = true;
		//Reset to default state.
		stats.clear();
		//Save the defaults to repair the file.
		save();
	}

	if (!closed) fclose(pFile);

}

std::string BenchmarkStatManager::getExecutableFolder() const
{

	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	char *pos;
	if (pos = strrchr(buffer, '\\'))
	{
		*pos = 0;
	}

	return buffer;

}