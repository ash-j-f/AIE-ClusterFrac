#include <SFML\Graphics.hpp>
#include "Host.h"
#include "Mandelbrot.h"

/**
* Mandelbrot reference application that provides an example of using the ClusterFrac library host class.
* User input is provided to allow pan and zoom control. Last pan/zoom position is saved and loaded from 
* a file on disk.
* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
*/

//Image and window width.
static constexpr unsigned int IMAGE_WIDTH = 1024;

//Image and window height.
static constexpr unsigned int IMAGE_HEIGHT = 768;

/**
* Main function, performing the core Mandelbrot test and display task.
* @param argc The number of strings in array argv.
* @param argv The array of command-line argument strings.
* @param envp The array of environment variable strings.
*/
int main(int argc, char *argv[], char *envp[])
{
	try
	{

		//CF_SETTINGS->setLogLevel(cf::Settings::LogLevels::Error);

		//Create new host object.
		cf::Host *host = new cf::Host();

		//Check if a non default port was specified.
		if (argc > 1)
		{
			host->setPort(atoi(argv[1]));
		}

		//Check if a non default concurrency was specified.
		if (argc > 2)
		{
			host->setConcurrency(atoi(argv[2]));
		}

		//Create Mandelbrot object.
		Mandelbrot mb{ host };

		//Load font file.
		sf::Font font;
		if (!font.loadFromFile(mb.getExecutableFolder() + "\\fonts\\" + "Topaz-8.ttf"))
		{
			std::cerr << "Could not load font file." << std::endl;
			exit(1);
		}

		//Define text elements.

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

		sf::Text benchTime;
		benchTime.setFont(font);
		benchTime.setCharacterSize(12);
		benchTime.setFillColor(sf::Color::White);
		benchTime.setPosition(5, 35);

		sf::Text zoomAmt;
		zoomAmt.setFont(font);
		zoomAmt.setCharacterSize(12);
		zoomAmt.setFillColor(sf::Color::White);
		zoomAmt.setPosition(5, 50);

		sf::Text onscreenHelp1;
		onscreenHelp1.setFont(font);
		onscreenHelp1.setCharacterSize(12);
		onscreenHelp1.setFillColor(sf::Color::White);
		onscreenHelp1.setPosition(5, 65);
		onscreenHelp1.setString("WASD to pan. -/+ to zoom.");

		sf::Text onscreenHelp2;
		onscreenHelp2.setFont(font);
		onscreenHelp2.setCharacterSize(12);
		onscreenHelp2.setFillColor(sf::Color::White);
		onscreenHelp2.setPosition(5, 80);
		onscreenHelp2.setString("R rst. view. T rst. zoom.");

		//Box behind text elements.
		sf::RectangleShape rectangle(sf::Vector2f(285, 100));
		rectangle.setPosition(0, 0);
		rectangle.setFillColor(sf::Color(0, 0, 0, 127));

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

		//Create window.
		sf::RenderWindow window(sf::VideoMode(IMAGE_WIDTH, IMAGE_HEIGHT), "Mandelbrot", sf::Style::Titlebar | sf::Style::Close);
		window.setFramerateLimit(0);

		//Set up image.
		sf::Image image;
		image.create(IMAGE_WIDTH, IMAGE_HEIGHT, sf::Color(0, 0, 0));
		sf::Texture texture;
		sf::Sprite sprite;

		//Does the image need to be updated due to user input?
		bool stateChanged = true;

		//Last zoom direction. Used to determine which direction new cached zoom levels should be generated. 
		bool zoomingIn = true;

		//User input.
		while (window.isOpen() && !cf::ConsoleMessager::getInstance()->exceptionThrown) {
			sf::Event event;
			while (window.pollEvent(event) && !stateChanged && !cf::ConsoleMessager::getInstance()->exceptionThrown) {
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
						mb.zoomLevel++;
						mb.zoom = mb.getNewZoom(1);
						zoomingIn = true;
						break;
					case sf::Keyboard::Dash:
						mb.zoomLevel--;
						mb.zoom = mb.getNewZoom(-1);
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
							if (mvd.offsetX == mb.offsetX && mvd.offsetY == mb.offsetY && mvd.zoom == mb.getNewZoom(zoomFactor))
							{
								if (zoomFactor == 0) viewResult = mvd.result;
								found = true;
								break;
							}
						}

						if (!found)
						{
							mb.newView(mb.getNewZoom(zoomFactor), mb.offsetX, mb.offsetY, IMAGE_WIDTH, IMAGE_HEIGHT);
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
				clientCount.setString("Connected Clients: " + std::to_string(host->getClientsCount() - 1));
				cacheCount.setString("Cached views: " + std::to_string(mb.cache.size()));
				benchTime.setString("Avg. task time (ms): " + std::to_string(host->getAverageBenchmarkTime().asMilliseconds()));

				//Generate scientific notation for zoom level.
				char buffer[1024] = { '\0' };
				sprintf_s(buffer, "%+.5e", mb.zoom);

				zoomAmt.setString((std::string) "Zoom: " + buffer);

				//Draw objects on screen.
				window.draw(sprite);

				window.draw(rectangle);

				window.draw(clientCount);
				window.draw(cacheCount);
				window.draw(benchTime);
				window.draw(zoomAmt);
				window.draw(onscreenHelp1);
				window.draw(onscreenHelp2);

				//Send buffer to GPU.
				window.display();

				//Keep up to N result sets in cache. Each set is one screen worth of pixel bytes in size.
				//Or about 2.5MB per 1920x1080 screen.
				mb.purgeCache(400);

				//Reset frame clock.
				clock.restart();
			}
		}

		if (cf::ConsoleMessager::getInstance()->exceptionThrown)
		{
			CF_SAY("\nExeception thrown. Aborting.", cf::Settings::LogLevels::Error);
			CF_SAY("Exeception was: " + cf::ConsoleMessager::getInstance()->exceptionMessage + "\n", cf::Settings::LogLevels::Error);
		}
	}
	catch (std::string e)
	{
		std::cerr << e << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown exception." << std::endl;
	}
	system("pause");
}