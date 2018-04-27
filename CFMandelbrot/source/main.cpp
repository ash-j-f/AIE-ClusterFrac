#include <SFML\Graphics.hpp>
#include "Host.h"
#include "Mandelbrot.h"

static constexpr unsigned int IMAGE_WIDTH = 1024;
static constexpr unsigned int IMAGE_HEIGHT = 768;

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

	//Load previous offset position and zoom, if one has been saved.
	mb.load();

	sf::RenderWindow window(sf::VideoMode(IMAGE_WIDTH, IMAGE_HEIGHT), "Mandelbrot");
	window.setFramerateLimit(0);

	sf::Image image;
	image.create(IMAGE_WIDTH, IMAGE_HEIGHT, sf::Color(0, 0, 0));
	sf::Texture texture;
	sf::Sprite sprite;

	//Does the image need to be updated due to user input?
	bool stateChanged = true;

	//Last zoom direction. Used to determine which direction new cached zoom levels should be generated. 
	bool zoomingIn = true;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event) && !stateChanged) {
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				//Default to updating the image.
				stateChanged = true;
				switch (event.key.code) {
				case sf::Keyboard::Escape:
					window.close();
					break;
				case sf::Keyboard::R:
					mb.reset();
					zoomingIn = true;
					break;
				case sf::Keyboard::T:
					mb.resetZoomOnly();
					zoomingIn = true;
					break;
				case sf::Keyboard::Equal:
					mb.zoom = mb.getNewZoom(mb.zoom, 1);
					zoomingIn = true;
					break;
				case sf::Keyboard::Dash:
					mb.zoom = mb.getNewZoom(mb.zoom, -1);
					zoomingIn = false;
					break;
				case sf::Keyboard::W:
					mb.offsetY = mb.getNewOffsetY(mb.offsetY, mb.zoom, -1);
					break;
				case sf::Keyboard::S:
					mb.offsetY = mb.getNewOffsetY(mb.offsetY, mb.zoom, 1);
					break;
				case sf::Keyboard::A:
					mb.offsetX = mb.getNewOffsetX(mb.offsetX, mb.zoom, -1);
					break;
				case sf::Keyboard::D:
					mb.offsetX = mb.getNewOffsetX(mb.offsetX, mb.zoom, 1);
					break;
				default:
					//No change by user input, so don't update the image.
					stateChanged = false;
					break;
				}
			default:
				break;
			}
		}

		if (window.isOpen())
		{

			//For current view position, do we have the next zoomed view in cache?
			cf::Result *viewResult = nullptr;
			int clCount = host->getClientsCount();
			int maxDepth = host->getClientsCount();
			for (int i = 0; i < clCount; i++)
			{
				int zoomFactor = 0;

				while (true)
				{
					bool found = false;
					
					for (auto &mvd : mb.cache)
					{
						if (mvd.offsetX == mb.offsetX && mvd.offsetY == mb.offsetY && mvd.zoom == mb.getNewZoom(mb.zoom, zoomFactor))
						{
							if (zoomFactor == 0) viewResult = mvd.result;
							found = true;
							break;
						}
					}

					if (!found)
					{
						mb.newView(host, mb.getNewZoom(mb.zoom, zoomFactor), mb.offsetX, mb.offsetY, IMAGE_WIDTH, IMAGE_HEIGHT);
						break;
					}
					else if (abs(zoomFactor) > maxDepth)
					{
						break;
					}
					else
					{
						zoomFactor += (zoomingIn ? 1 : -1);
					}
				}
			}

			//Match incoming results to their cached tasks.
			for (auto &mvd : mb.cache)
			{
				if (mvd.result == nullptr)
				{
					if (host->checkAvailableResult(mvd.taskID))
					{
						mvd.result = host->getAvailableResult(mvd.taskID);
					}
				}
			}

			//If a new camera position or zoom was set, update the current image on screen.
			if (stateChanged)
			{
				//bool updated = mb.updateImage(mb.zoom, mb.offsetX, mb.offsetY, image, IMAGE_WIDTH, IMAGE_HEIGHT);
				
				//Is this view zoom and offset already in the cache?
				
				if (viewResult != nullptr)
				{
					MandelbrotResult *output = static_cast<MandelbrotResult *>(viewResult);
					unsigned int count = (unsigned int)output->numbers.size();

					unsigned int y = 0;
					unsigned int x = 0;
					for (x = 0; x < IMAGE_WIDTH; x++)
					{
						for (y = 0; y < IMAGE_HEIGHT; y++)
						{
							image.setPixel(x, y, mb.getColor(output->numbers[IMAGE_WIDTH * y + x]));
						}
					}

					texture.loadFromImage(image);
					sprite.setTexture(texture);
					stateChanged = false;
					mb.save();
				}
			}
			window.draw(sprite);
			window.display();

			//Keep up to N result sets in cache. Each set is one screen worth of pixel bytes in size.
			//Or about 2.5MB per 1920x1080 screen.
			mb.purgeCache(400);
			
			clock.restart();
		}
	}
}