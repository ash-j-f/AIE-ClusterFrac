#pragma once
#include <string>
#include <Task.h>
#include "MandelbrotResult.hpp"

/**
* Mandelbrot test task class.
* Derived from ClusterFrac library Task class.
* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
*/
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
	
	inline std::string getSubtype() const override { return "MandelbrotTask"; };

private:

	static const sf::Uint8 MAX = 255; // maximum number of iterations for mandelbrot()

	inline std::vector<cf::Task *> splitLocal(unsigned int count) const override
	{
		//Get number of pixels being computed per Y-axis chunk.
		unsigned int pixelCount = (maxY - minY) + 1;

		//Limit number of tasks to at least number of target numbers.
		if (pixelCount < count) count = pixelCount;

		std::vector<MandelbrotTask *> tasks = std::vector<MandelbrotTask *>();
		tasks.resize(count);

		for (unsigned int i = 0; i < count; i++)
		{
			tasks[i] = new MandelbrotTask();
		}

		//Distribute numbers among the new tasks.
		const unsigned int step = (int)floor(pixelCount / (float)count);
		unsigned int start = minY; // 0;
		unsigned int end = minY + (step - 1);
		for (unsigned int i = 0; i < count; i++)
		{
			//If this is the final split, then get the remainder of items.
			if (i == count - 1) end = minY + pixelCount;

			tasks[i]->minY = start;
			tasks[i]->maxY = std::min(end, minY + (pixelCount - 1));
			tasks[i]->zoom = zoom;
			tasks[i]->offsetX = offsetX;
			tasks[i]->offsetY = offsetY;
			tasks[i]->spaceHeight = spaceHeight;
			tasks[i]->spaceWidth = spaceWidth;

			start = start + step;
			end = end + step;
		}

		//Convert BenchmarkTask pointers to Task pointers.
		std::vector<cf::Task *> tasksConv;
		for (auto &task : tasks) tasksConv.push_back((Task *)task);

		return tasksConv;
	};

	inline void serializeLocal(cf::WorkPacket &p) const override
	{
		p << zoom;
		p << offsetX;
		p << offsetY;
		p << minY;
		p << maxY;
		p << spaceWidth;
		p << spaceHeight;
	};

	inline void deserializeLocal(cf::WorkPacket &p) override
	{
		p >> zoom;
		p >> offsetX;
		p >> offsetY;
		p >> minY;
		p >> maxY;
		p >> spaceWidth;
		p >> spaceHeight;
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

	inline cf::Result *runLocal() const override
	{
		MandelbrotResult *result = new MandelbrotResult();

		result->zoom = zoom;
		result->offsetX = offsetX;
		result->offsetY = offsetY;

		result->numbers.resize(spaceWidth * ((maxY - minY) + 1));

		double real = 0 * zoom - spaceWidth / 2.0 * zoom + offsetX;
		double imagstart = minY * zoom - spaceHeight / 2.0 * zoom + offsetY;
		for (unsigned int x = 0; x < spaceWidth; x++, real += zoom) 
		{
			double imag = imagstart;
			for (unsigned int y = minY; y <= maxY; y++, imag += zoom) 
			{
				result->numbers[spaceWidth * (y - minY) + x] = mandelbrot(real, imag);
			}
		}

		return result;
	};

};