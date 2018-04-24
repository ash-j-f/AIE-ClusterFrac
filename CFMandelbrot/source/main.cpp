#include <SFML\Graphics.hpp>
#include "Host.h"
#include "Mandelbrot.h"

static constexpr unsigned int IMAGE_WIDTH = 1920;
static constexpr unsigned int IMAGE_HEIGHT = 1080;

int main(int argc, //Number of strings in array argv  
	char *argv[], //Array of command-line argument strings  
	char *envp[]) // Array of environment variable strings  
{
	//CF_SETTINGS->setLogLevel(cf::Settings::LogLevels::Error);

	//Create new host object.
	cf::Host *host = new cf::Host();
	
	//Set user defined Task and Result types.
	host->registerTaskType("MandelbrotTask", []{ MandelbrotTask *m = new MandelbrotTask(); return static_cast<cf::Task *>(m); });
	host->registerResultType("MandelbrotResult", []{ MandelbrotResult *m = new MandelbrotResult(); return static_cast<cf::Result *>(m); });

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
			mb.updateImage(zoom, offsetX, offsetY, image, IMAGE_WIDTH, IMAGE_HEIGHT);
			texture.loadFromImage(image);
			sprite.setTexture(texture);
			stateChanged = false;
		}
		window.draw(sprite);
		window.display();
		clock.restart();
	}
}