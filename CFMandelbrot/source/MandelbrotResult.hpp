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

	/**
	* Default constructor.
	*/
	MandelbrotResult() {};

	/**
	* Default destructor.
	*/
	~MandelbrotResult() {};

	//Results data.
	std::vector<sf::Uint8> numbers;

	//View zoom used to create the results.
	double zoom;
	
	//View offsetX used to create the results.
	double offsetX;
	
	//View offsetY used to create the results.
	double offsetY;

	/**
	* Get the subtype of this result.
	* @returns The subtype of this result.
	*/
	inline std::string getSubtype() const override { return "MandelbrotResult"; };

private:

	/**
	* Merge other results in a std::vector into this result.
	* @param others A std::vector of pointers to the all results in a set to merge with this one.
	* @returns void.
	*/
	inline void mergeLocal(const std::vector<cf::Result *> others) override
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

	/**
	* Serialize this result and store the data in a given packet.
	* @param p The packet to store the data in.
	* @returns void.
	*/
	inline void serializeLocal(cf::WorkPacket &p) const override
	{
		p << zoom;
		p << offsetX;
		p << offsetY;
		sf::Int64 size;
		size = numbers.size();
		p << size;
		for (sf::Int64 i = 0; i < size; i++) p << numbers[i];
	};

	/**
	* Deserialize this result from data provided by a packet.
	* @param p The packet to retrieve the result data from.
	* @returns void.
	*/
	inline void deserializeLocal(cf::WorkPacket &p) override
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