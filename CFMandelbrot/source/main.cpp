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

	//Create Mandelbrot object.
	Mandelbrot mb{ host };

	//Load font file.
	sf::Font font;
	if (!font.loadFromFile(mb.getExecutableFolder() + "\\fonts\\" + "Topaz-8.ttf"))
	{
		std::cerr << "Could not load font file." << std::endl;
		exit(1);
	}

	sf::Text clientCount;
	clientCount.setFont(font);
	clientCount.setCharacterSize(12);
	clientCount.setFillColor(sf::Color::White);
	clientCount.setPosition(5, 5);

	sf::Text cacheCount;
	cacheCount.setFont(font);
	cacheCount.setCharacterSize(12);
	cacheCount.setFillColor(sf::Color::White);
	cacheCount.setPosition(5, 20);

	//Load previous offset position and zoom, if one has been saved.
	mb.load();

	//Set user defined Task and Result types.
	host->registerTaskType("MandelbrotTask", [] { MandelbrotTask *m = new MandelbrotTask(); return static_cast<cf::Task *>(m); });
	host->registerResultType("MandelbrotResult", [] { MandelbrotResult *m = new MandelbrotResult(); return static_cast<cf::Result *>(m); });

	//Start the host with ability to process tasks as a client itself.
	host->setHostAsClient(true);
	host->start();

	//Clock used for benchmarks.
	sf::Clock clock;

	sf::RenderWindow window(sf::VideoMode(IMAGE_WIDTH, IMAGE_HEIGHT), "Mandelbrot", sf::Style::Titlebar | sf::Style::Close);
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
			//Get an extra level of zoom depth per connected client.
			cf::Result *viewResult = nullptr;
			int clCount = host->getClientsCount();
			int maxDepth = host->getClientsCount() - 1;
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
						mb.newView(mb.getNewZoom(mb.zoom, zoomFactor), mb.offsetX, mb.offsetY, IMAGE_WIDTH, IMAGE_HEIGHT);
						break;
					}
					else if (abs(zoomFactor) >= maxDepth)
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
				
				//Is the current view zoom and offset in the cache and ready to be displayed?
				
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

			//Text updates.
			clientCount.setString("Clients: " + std::to_string(host->getClientsCount() - 1));
			cacheCount.setString("Cached views: " + std::to_string(mb.cache.size()));
			
			window.draw(sprite); 
			window.draw(clientCount);
			window.draw(cacheCount);
			
			window.display();

			//Keep up to N result sets in cache. Each set is one screen worth of pixel bytes in size.
			//Or about 2.5MB per 1920x1080 screen.
			mb.purgeCache(400);
			
			clock.restart();
		}
	}
}