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

	/**
	* Default constructor.
	*/
	MandelbrotTask() {};
	
	/**
	* Default destructor.
	*/
	~MandelbrotTask() {};

	//View zoom for this task.
	double zoom;

	//View offsetX for this task.
	double offsetX;

	//View offsetY for this task.
	double offsetY;

	//Starting Y coordinate for this task.
	sf::Uint32 minY;

	//Ending Y coordinate for this task.
	sf::Uint32 maxY;

	//Number space width.
	sf::Uint32 spaceWidth;

	//Number space height.
	sf::Uint32 spaceHeight;
	
	/**
	* Get the subtype of this task.
	* @returns The subtype of this task.
	*/
	inline std::string getSubtype() const override { return "MandelbrotTask"; };

private:

	//Maximum number of iterations for Mandelbrot calculations.
	const sf::Uint8 MAX = 255;

	/**
	* Split this task up as equally as possible in to N chunks, and return
	* a std::vector of pointers to those split tasks.
	* @param count Split the task into this many subtasks.
	* @returns A std::vector of pointers to the new split tasks.
	*/
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
		unsigned int start = minY;
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

	/**
	* Serialize this task and store the data in a given packet.
	* @param p The packet to store the data in.
	* @returns void.
	*/
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

	/**
	* Deserialize this task from data provided by a packet.
	* @param p The packet to retrieve the task data from.
	* @returns void.
	*/
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

	/**
	* Perform the Mandelbrot calculation on a number.
	* @param startReal The starting value of the real component of the number.
	* @param startImag The starting value of the imaginary component of the number.
	* @returns The number of iterations before the value was determined to be escaping towards infinity.
	*/
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

	/**
	* Run the task and produce a results object.
	* @returns A pointer to the new results object.
	*/
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