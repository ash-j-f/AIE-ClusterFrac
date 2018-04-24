#pragma once
#include <array>
#include <vector>
#include <thread>
#include <SFML\Graphics.hpp>
#include "Host.h"
#include "MandelbrotTask.hpp"
#include "MandelbrotResult.hpp"

class Mandelbrot {
public:
	Mandelbrot(cf::Host *newHost);
	void updateImage(double zoom, double offsetX, double offsetY, sf::Image& image, unsigned int imageWidth, unsigned int imageHeight) const;
private:
	static const sf::Uint8 MAX = 255; // maximum number of iterations for mandelbrot()
	std::array<sf::Color, MAX + 1> colors;
	cf::Host *host;
	sf::Color getColor(int iterations) const;
};