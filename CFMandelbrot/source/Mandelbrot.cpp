#include "Mandelbrot.h"

Mandelbrot::Mandelbrot(cf::Host *newHost) {
	host = newHost;
	for (int i = 0; i <= MAX; ++i) {
		colors[i] = createColor(i);
	}
	
	nextCacheID = 0;

	//Reset offset and zoom values to sensible defaults.
	reset();
}

void Mandelbrot::purgeCache(int maxCacheResults)
{
	int oldestCacheID = nextCacheID - maxCacheResults;

	//If there aren't yet maxCacheResults number of cached results, abort.
	if (oldestCacheID <= 0) return;

	std::vector<MandelbrotViewData>::iterator it;
	for (it = cache.begin(); it != cache.end();)
	{
		if ((*it).cacheEntryID < (unsigned int)oldestCacheID && (*it).result != nullptr)
		{
			host->removeResultFromQueue((*it).result);
			it = cache.erase(it);
		}
		else
		{
			it++;
		}
	}
}

sf::Color Mandelbrot::createColor(int iterations) const {

	//Colouring method from https://solarianprogrammer.com/2013/02/28/mandelbrot-set-cpp-11/

	double t = (double)iterations / (double)MAX;

	// Use smooth polynomials for r, g, b
	int r = (int)(9.0 * (1.0 - t)*t*t*t * 255.0);
	int g = (int)(15.0 * (1.0 - t)*(1.0 - t)*t*t * 255.0);
	int b = (int)(8.5*(1.0 - t)*(1.0 - t)*(1.0 - t)*t * 255.0);

	return sf::Color(r, g, b);
}

std::string Mandelbrot::getExecutableFolder() const
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

double Mandelbrot::getNewZoom(double currentZoom, int factor) const
{
	if (factor == 0) return currentZoom;

	for (int i = 0; i < abs(factor); i++)
	{
		if (factor > 0)
		{
			currentZoom *= 0.9;
		}
		else
		{
			currentZoom /= 0.9;
		}
	}
	return currentZoom;
}

double Mandelbrot::getNewOffsetY(double currentOffsetY, double currentZoom, int factor) const
{
	if (factor == 0) return currentOffsetY;

	for (int i = 0; i < abs(factor); i++)
	{
		if (factor > 0)
		{
			currentOffsetY += 40.0 * currentZoom;
		}
		else
		{
			currentOffsetY -= 40.0 * currentZoom;
		}
	}
	return currentOffsetY;
}

double Mandelbrot::getNewOffsetX(double currentOffsetX, double currentZoom, int factor) const
{
	if (factor == 0) return currentOffsetX;

	for (int i = 0; i < abs(factor); i++)
	{
		if (factor > 0)
		{
			currentOffsetX += 40.0 * currentZoom;
		}
		else
		{
			currentOffsetX -= 40.0 * currentZoom;
		}
	}

	return currentOffsetX;
}

void Mandelbrot::newView(cf::Host *host, double zoom, double offsetX, double offsetY, unsigned int imageWidth, unsigned int imageHeight)
{
	cf::Task *task = new MandelbrotTask();

	//Assign the task a unique ID.
	task->assignID();
	task->setNodeTargetType(cf::Task::NodeTargetTypes::Any);
	task->allowNodeTaskSplit = false;

	((MandelbrotTask *)task)->zoom = zoom;
	((MandelbrotTask *)task)->offsetX = offsetX;
	((MandelbrotTask *)task)->offsetY = offsetY;
	((MandelbrotTask *)task)->spaceWidth = imageWidth;
	((MandelbrotTask *)task)->spaceHeight = imageHeight;
	((MandelbrotTask *)task)->minY = 0;
	((MandelbrotTask *)task)->maxY = imageHeight - 1;

	host->addTaskToQueue(task);

	//Create new cache entry for this zoom level.
	MandelbrotViewData mvd;
	mvd.zoom = zoom;
	mvd.offsetX = offsetX;
	mvd.offsetY = offsetY;
	mvd.result = nullptr;
	mvd.taskID = task->getInitialTaskID();
	mvd.cacheEntryID = nextCacheID++;
	cache.push_back(mvd);
}

void Mandelbrot::save() const
{
	FILE *pFile;
	
	fopen_s(&pFile,(getExecutableFolder() + "\\" + "savedata.bin").c_str(), "wb");
	
	if (!pFile) return;
	
	fwrite(&offsetX, sizeof(char), sizeof(offsetX), pFile);
	fwrite(&offsetY, sizeof(char), sizeof(offsetY), pFile);
	fwrite(&zoom, sizeof(char), sizeof(zoom), pFile);
	fclose(pFile);
}

void Mandelbrot::load()
{
	FILE *pFile;

	fopen_s(&pFile, (getExecutableFolder() + "\\" + "savedata.bin").c_str(), "rb");

	if (!pFile) return;

	fread(&offsetX, sizeof(char), sizeof(offsetX), pFile);
	fread(&offsetY, sizeof(char), sizeof(offsetY), pFile);
	fread(&zoom, sizeof(char), sizeof(zoom), pFile);
	fclose(pFile);
}

void Mandelbrot::reset()
{	
	//Defaults.
	offsetX = -0.7;
	offsetY = 0.0;
	zoom = 0.004;
}

void Mandelbrot::resetZoomOnly()
{
	zoom = 0.004;
}
