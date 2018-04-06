#pragma once
#include <string>
#include <Result.h>

class MandelbrotResult : public cf::Result
{
public:
	MandelbrotResult() {};
	~MandelbrotResult() {};

	std::vector<sf::Uint8> numbers;
	std::vector<sf::Uint16> x;
	std::vector<sf::Uint16> y;

	inline std::string getSubtype() const { return "MandelbrotResult"; };

private:

	inline virtual void mergeLocal(const std::vector<cf::Result *> others)
	{

		//Insert the other results into the new merged result.
		for (auto &r : others)
		{
			MandelbrotResult *mbr = static_cast<MandelbrotResult *>(r);
			numbers.insert(numbers.end(), mbr->numbers.begin(), mbr->numbers.end());
			x.insert(x.end(), mbr->x.begin(), mbr->x.end());
			y.insert(y.end(), mbr->y.begin(), mbr->y.end());
		}

	};

	inline void serializeLocal(cf::WorkPacket &p) const
	{
		sf::Int64 size;
		size = numbers.size();
		p << size;
		for (sf::Int64 i = 0; i < size; i++) p << numbers[i];
		size = x.size();
		p << size;
		for (sf::Int64 i = 0; i < size; i++) p << x[i];
		size = y.size();
		p << size;
		for (sf::Int64 i = 0; i < size; i++) p << y[i];
	};

	inline void deserializeLocal(cf::WorkPacket &p)
	{
		sf::Int64 size;
		p >> size;
		numbers.resize(size);
		for (sf::Int64 i = 0; i < size; i++) p >> numbers[i];
		p >> size;
		x.resize(size);
		for (sf::Int64 i = 0; i < size; i++) p >> x[i];
		p >> size;
		y.resize(size);
		for (sf::Int64 i = 0; i < size; i++) p >> y[i];
	};
};