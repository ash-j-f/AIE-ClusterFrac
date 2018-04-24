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

	Mandelbrot mb{host};

	//Defaults.
	mb.offsetX = -0.7; // and move around
	mb.offsetY = 0.0;
	mb.zoom = 0.004; // allow the user to zoom in and out...

	mb.load();

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
					mb.zoom *= 0.9;
					break;
				case sf::Keyboard::Dash:
					mb.zoom /= 0.9;
					break;
				case sf::Keyboard::W:
					mb.offsetY -= 40 * mb.zoom;
					break;
				case sf::Keyboard::S:
					mb.offsetY += 40 * mb.zoom;
					break;
				case sf::Keyboard::A:
					mb.offsetX -= 40 * mb.zoom;
					break;
				case sf::Keyboard::D:
					mb.offsetX += 40 * mb.zoom;
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
			mb.updateImage(mb.zoom, mb.offsetX, mb.offsetY, image, IMAGE_WIDTH, IMAGE_HEIGHT);
			texture.loadFromImage(image);
			sprite.setTexture(texture);
			stateChanged = false;
			mb.save();
		}
		window.draw(sprite);
		window.display();
		clock.restart();
	}
}