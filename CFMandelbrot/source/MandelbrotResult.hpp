#pragma once
#include <string>
#include <Result.h>

/**
* Mandelbrot test result class.
* Derived from ClusterFrac library Result class.
* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
*/
class MandelbrotResult : public cf::Result
{
public:
	MandelbrotResult() {};
	~MandelbrotResult() {};

	std::vector<sf::Uint8> numbers;

	double zoom;
	double offsetX;
	double offsetY;

	inline std::string getSubtype() const { return "MandelbrotResult"; };

private:

	inline virtual void mergeLocal(const std::vector<cf::Result *> others)
	{

		//Insert the other results into the new merged result.
		unsigned int sizeSum = (unsigned int)numbers.size();
		for (auto &r : others)
		{
			MandelbrotResult *mbr = static_cast<MandelbrotResult *>(r);
			sizeSum += (unsigned int)mbr->numbers.size();
		}

		//Resize the numbers vector max capacity ahead of insertion.
		numbers.reserve(sizeSum);

		for (auto &r : others)
		{
			MandelbrotResult *mbr = static_cast<MandelbrotResult *>(r);
			numbers.insert(numbers.end(), mbr->numbers.begin(), mbr->numbers.end());
		}

		//All results in this set will have the same zoom and offset values.
		if (others.size() > 0)
		{
			MandelbrotResult *mbr = static_cast<MandelbrotResult *>(others.front());
			zoom = mbr->zoom;
			offsetX = mbr->offsetX;
			offsetY = mbr->offsetY;
		}
	};

	inline void serializeLocal(cf::WorkPacket &p) const
	{
		p << zoom;
		p << offsetX;
		p << offsetY;
		sf::Int64 size;
		size = numbers.size();
		p << size;
		for (sf::Int64 i = 0; i < size; i++) p << numbers[i];
	};

	inline void deserializeLocal(cf::WorkPacket &p)
	{
		p >> zoom;
		p >> offsetX;
		p >> offsetY;
		sf::Int64 size;
		p >> size;
		numbers.resize(size);
		for (sf::Int64 i = 0; i < size; i++) p >> numbers[i];
	};
};