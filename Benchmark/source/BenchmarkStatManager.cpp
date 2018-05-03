#include "BenchmarkStatManager.h"

BenchmarkStatManager::BenchmarkStatManager()
{
}

BenchmarkStatManager::~BenchmarkStatManager()
{
}

void BenchmarkStatManager::save() const
{
	FILE *pFile;

	fopen_s(&pFile, (getExecutableFolder() + "\\" + "savedata.bin").c_str(), "wb");

	if (!pFile) return;

	bool closed = false;

	try
	{
		fwrite(&offsetX, sizeof(char), sizeof(offsetX), pFile);
		fwrite(&offsetY, sizeof(char), sizeof(offsetY), pFile);
		fwrite(&zoom, sizeof(char), sizeof(zoom), pFile);
		fwrite(&zoomLevel, sizeof(char), sizeof(zoomLevel), pFile);
		fwrite(&defaultZoom, sizeof(char), sizeof(defaultZoom), pFile);
	}
	catch (...)
	{
		fclose(pFile);
		closed = true;
		//Invalid data in file. Proceed with default values.
		CF_SAY("Unable to write Mandelbrot data file " + getExecutableFolder() + "\\"
			+ "savedata.bin. Zoom and offset values not saved.", cf::Settings::LogLevels::Error);
	}

	if (!closed) fclose(pFile);
}

void BenchmarkStatManager::load()
{
	FILE *pFile;

	fopen_s(&pFile, (getExecutableFolder() + "\\" + "savedata.bin").c_str(), "rb");

	if (!pFile) return;

	bool closed = false;

	try
	{
		size_t byteSize;
		byteSize = fread(&offsetX, sizeof(char), sizeof(offsetX), pFile);
		if (byteSize != sizeof(offsetX)) throw "Invalid data.";
		byteSize = fread(&offsetY, sizeof(char), sizeof(offsetY), pFile);
		if (byteSize != sizeof(offsetY)) throw "Invalid data.";
		byteSize = fread(&zoom, sizeof(char), sizeof(zoom), pFile);
		if (byteSize != sizeof(zoom)) throw "Invalid data.";
		byteSize = fread(&zoomLevel, sizeof(char), sizeof(zoomLevel), pFile);
		if (byteSize != sizeof(zoomLevel)) throw "Invalid data.";
		byteSize = fread(&defaultZoom, sizeof(char), sizeof(defaultZoom), pFile);
		if (byteSize != sizeof(defaultZoom)) throw "Invalid data.";
	}
	catch (...)
	{
		//Invalid data in file. Proceed with default values.
		CF_SAY("Invalid Mandelbrot data file " + getExecutableFolder() + "\\"
			+ "savedata.bin. Using default zoom and offset values.", cf::Settings::LogLevels::Error);
		fclose(pFile);
		closed = true;
		//Reset to defaults.
		reset();
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