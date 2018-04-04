#include <array>
#include <vector>
#include <thread>
#include <SFML\Graphics.hpp>
#include "Host.h"
#include "MandelbrotTask.hpp"
#include "MandelbrotResult.hpp"

static constexpr int IMAGE_WIDTH = 1920;
static constexpr int IMAGE_HEIGHT = 1080;

class Mandelbrot {
public:
	Mandelbrot(cf::Host *newHost);
	void updateImage(double zoom, double offsetX, double offsetY, sf::Image& image) const;
private:
	static const int MAX = 256; // maximum number of iterations for mandelbrot()
	std::array<sf::Color, MAX + 1> colors;
	cf::Host *host;
	int mandelbrot(double startReal, double startImag) const;
	sf::Color getColor(int iterations) const;
	void updateImageSlice(double zoom, double offsetX, double offsetY, sf::Image& image, int minY, int maxY) const;
};

Mandelbrot::Mandelbrot(cf::Host *newHost) {
	host = newHost;
	for (int i = 0; i <= MAX; ++i) {
		colors[i] = getColor(i);
	}
}

int Mandelbrot::mandelbrot(double startReal, double startImag) const {
	double zReal = startReal;
	double zImag = startImag;

	for (int counter = 0; counter < MAX; ++counter) {
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

sf::Color Mandelbrot::getColor(int iterations) const {
	
	//Colouring method from https://solarianprogrammer.com/2013/02/28/mandelbrot-set-cpp-11/

	double t = (double)iterations / (double)MAX;
	
	// Use smooth polynomials for r, g, b
	int r = (int)(9.0 * (1.0 - t)*t*t*t * 255.0);
	int g = (int)(15.0 * (1.0 - t)*(1.0 - t)*t*t * 255.0);
	int b = (int)(8.5*(1.0 - t)*(1.0 - t)*(1.0 - t)*t * 255.0);

	return sf::Color(r, g, b);
}

void Mandelbrot::updateImageSlice(double zoom, double offsetX, double offsetY, sf::Image& image, int minY, int maxY) const
{
	double real = 0 * zoom - IMAGE_WIDTH / 2.0 * zoom + offsetX;
	double imagstart = minY * zoom - IMAGE_HEIGHT / 2.0 * zoom + offsetY;
	for (int x = 0; x < IMAGE_WIDTH; x++, real += zoom) {
		double imag = imagstart;
		for (int y = minY; y < maxY; y++, imag += zoom) {
			int value = mandelbrot(real, imag);
			image.setPixel(x, y, colors[value]);
		}
	}
}

void Mandelbrot::updateImage(double zoom, double offsetX, double offsetY, sf::Image& image) const
{
	const int STEP = IMAGE_HEIGHT / std::thread::hardware_concurrency();
	std::vector<std::thread> threads;
	for (int i = 0; i < IMAGE_HEIGHT; i += STEP) {
		threads.push_back(std::thread(&Mandelbrot::updateImageSlice, *this, zoom, offsetX, offsetY, std::ref(image), i, std::min(i + STEP, IMAGE_HEIGHT)));
	}
	for (auto &t : threads) {
		t.join();
	}
}

int main(int argc, //Number of strings in array argv  
	char *argv[], //Array of command-line argument strings  
	char *envp[]) // Array of environment variable strings  
{

	//Create new host object.
	cf::Host *host = new cf::Host();
	
	//Set user defined Task and Result types.
	host->registerTaskType("MendelbrotTask", []() { MandelbrotTask *m = new MandelbrotTask(); return static_cast<cf::Task *>(m); });
	host->registerResultType("MendelbrotResult", []() { MandelbrotResult *m = new MandelbrotResult(); return static_cast<cf::Result *>(m); });

	host->setHostAsClient(true);
	host->start();

	sf::Clock clock;

	double offsetX = -0.7; // and move around
	double offsetY = 0.0;
	double zoom = 0.004; // allow the user to zoom in and out...
	Mandelbrot mb{host};

	sf::RenderWindow window(sf::VideoMode(IMAGE_WIDTH, IMAGE_HEIGHT), "Mandelbrot");
	window.setFramerateLimit(0);

	sf::Image image;
	image.create(IMAGE_WIDTH, IMAGE_HEIGHT, sf::Color(0, 0, 0));
	sf::Texture texture;
	sf::Sprite sprite;

	bool stateChanged = true; // track whether the image needs to be regenerated

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				stateChanged = true; // image needs to be recreated when the user changes zoom or offset
				switch (event.key.code) {
				case sf::Keyboard::Escape:
					window.close();
					break;
				case sf::Keyboard::Equal:
					zoom *= 0.9;
					break;
				case sf::Keyboard::Dash:
					zoom /= 0.9;
					break;
				case sf::Keyboard::W:
					offsetY -= 40 * zoom;
					break;
				case sf::Keyboard::S:
					offsetY += 40 * zoom;
					break;
				case sf::Keyboard::A:
					offsetX -= 40 * zoom;
					break;
				case sf::Keyboard::D:
					offsetX += 40 * zoom;
					break;
				default:
					stateChanged = false;
					break;
				}
			default:
				break;
			}
		}

		if (stateChanged) {
			mb.updateImage(zoom, offsetX, offsetY, image);
			texture.loadFromImage(image);
			sprite.setTexture(texture);
			stateChanged = false;
		}
		window.draw(sprite);
		window.display();
		clock.restart();
	}
}