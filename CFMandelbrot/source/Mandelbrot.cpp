#include "Mandelbrot.h"

Mandelbrot::Mandelbrot(cf::Host *newHost) {
	host = newHost;
	for (int i = 0; i <= MAX; ++i) {
		colors[i] = getColor(i);
	}
	
	nextCacheID = 0;

	//Reset offset and zoom values to sensible defaults.
	reset();
}

sf::Color Mandelbrot::getColor(int iterations) const {

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
}

void Mandelbrot::updateImage(double zoom, double offsetX, double offsetY, sf::Image& image, unsigned int imageWidth, unsigned int imageHeight)
{
	cf::Result *viewResult = nullptr;

	//Is this view in the cache?
	for (auto &vd : cache)
	{
		if (vd.zoom == zoom && vd.offsetX == offsetX && vd.offsetY == offsetY)
		{
			viewResult = vd.result;
			break;
		}
	}

	if (viewResult == nullptr)
	{
		cf::Task *task = new MandelbrotTask();

		//Assign the task a unique ID.
		task->assignID();
		task->setNodeTargetType(cf::Task::NodeTargetTypes::Local);
		task->allowNodeTaskSplit = false;

		((MandelbrotTask *)task)->zoom = zoom;
		((MandelbrotTask *)task)->offsetX = offsetX;
		((MandelbrotTask *)task)->offsetY = offsetY;
		((MandelbrotTask *)task)->spaceWidth = imageWidth;
		((MandelbrotTask *)task)->spaceHeight = imageHeight;
		((MandelbrotTask *)task)->minY = 0;
		((MandelbrotTask *)task)->maxY = imageHeight - 1;

		unsigned __int64 taskID = task->getInitialTaskID();

		host->addTaskToQueue(task);

		//Wait until task is sent. Will wait for at least 1 client to be connected.
		while (!host->sendTasks());

		//Wait for results to be complete.
		while (!host->checkAvailableResult(taskID));

		viewResult = host->getAvailableResult(taskID);

		//Create new cache entry for this zoom level.
		MandelbrotViewData mvd;
		mvd.zoom = zoom;
		mvd.offsetX = offsetX;
		mvd.offsetY = offsetY;
		mvd.result = viewResult;
		mvd.taskID = task->getInitialTaskID();
		mvd.cacheEntryID = nextCacheID++;
		cache.push_back(mvd);
	}

	MandelbrotResult *output = static_cast<MandelbrotResult *>(viewResult);
	unsigned int count = (unsigned int)output->numbers.size();
	
	unsigned int y = 0;
	unsigned int x = 0;
	for (x = 0; x < imageWidth; x++)
	{
		for (y = 0; y < imageHeight; y++)
		{
			image.setPixel(x, y, colors[output->numbers[imageWidth * y + x]]);
		}
	}

	//Remove the result from the completed results queue.
	//host->removeResultFromQueue(finished);
	//finished = nullptr;
	//output = nullptr;

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
