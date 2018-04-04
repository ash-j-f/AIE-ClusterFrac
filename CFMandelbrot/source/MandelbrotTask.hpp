#pragma once
#include <string>
#include <Task.h>
#include "MandelbrotResult.hpp"

class MandelbrotTask : public cf::Task
{
public:
	MandelbrotTask() {};
	
	~MandelbrotTask() {};

	double zoom;
	double offsetX;
	double offsetY;
	sf::Uint32 minY;
	sf::Uint32 maxY;
	sf::Uint32 spaceWidth;
	sf::Uint32 spaceHeight;
	
	inline std::string getSubtype() const { return "MandelbrotTask"; };

private:

	static const sf::Uint8 MAX = 255; // maximum number of iterations for mandelbrot()

	inline std::vector<cf::Task *> splitLocal(int count) const
	{
		//Get number of pixels being computed.
		int pixelCount = (maxY - minY) + 1;

		//Limit number of tasks to at least number of target numbers.
		if (pixelCount < count) count = pixelCount;

		std::vector<MandelbrotTask *> tasks = std::vector<MandelbrotTask *>();
		tasks.resize(count);

		for (int i = 0; i < count; i++)
		{
			tasks[i] = new MandelbrotTask();
		}

		//Distribute numbers among the new tasks.
		const int step = (int)floor(pixelCount / (float)count);
		int start = 0;
		int end = step;
		for (int i = 0; i < count; i++)
		{
			//If this is the final split, then get the remainder of items.
			if (i == count - 1) end = pixelCount;

			tasks[i]->minY = start;
			tasks[i]->maxY = std::min(end, pixelCount);
			tasks[i]->zoom = zoom;
			tasks[i]->offsetX = offsetX;
			tasks[i]->offsetY = offsetY;

			start = start + step;
			end = end + step;
		}

		//Convert BenchmarkTask pointers to Task pointers.
		std::vector<cf::Task *> tasksConv;
		for (auto &task : tasks) tasksConv.push_back((Task *)task);

		return tasksConv;
	};

	inline void serializeLocal(cf::WorkPacket &p) const
	{
		p << zoom;
		p << offsetX;
		p << offsetY;
		p << minY;
		p << maxY;
	};

	inline void deserializeLocal(cf::WorkPacket &p)
	{
		p >> zoom;
		p >> offsetX;
		p >> offsetY;
		p >> minY;
		p >> maxY;
	};

	inline sf::Uint8 mandelbrot(double startReal, double startImag) const {
		double zReal = startReal;
		double zImag = startImag;

		for (sf::Uint8 counter = 0; counter < MAX; ++counter) {
			double r2 = zReal * zReal;
			double i2 = zImag * zImag;
			if (r2 + i2 > 4.0) {
				return counter;
			}
			zImag = 2.0 * zReal * zImag + startImag;
			zReal = r2 - i2 + startReal;
		}
		return MAX;
	}

	inline cf::Result *runLocal() const
	{
		MandelbrotResult *result = new MandelbrotResult();

		double real = 0 * zoom - spaceWidth / 2.0 * zoom + offsetX;
		double imagstart = minY * zoom - spaceHeight / 2.0 * zoom + offsetY;
		for (unsigned int x = 0; x < spaceWidth; x++, real += zoom) {
			double imag = imagstart;
			for (unsigned int y = minY; y < maxY; y++, imag += zoom) {
				result->numbers.push_back(mandelbrot(real, imag));
			}
		}
		return result;
	};

};