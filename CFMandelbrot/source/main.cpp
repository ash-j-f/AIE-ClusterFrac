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

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
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
					break;
				case sf::Keyboard::Equal:
					mb.zoom = mb.getNewZoom(mb.zoom, 1);
					break;
				case sf::Keyboard::Dash:
					mb.zoom = mb.getNewZoom(mb.zoom, -1);
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

			//For current view position, do we have the next zoomed view in cache?
			bool found = false;
			for (auto &mvd : mb.cache)
			{
				if (mvd.offsetX == mb.offsetX && mvd.offsetY == mb.offsetY && mvd.zoom == mb.getNewZoom(mb.zoom, 1))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				{
					cf::Task *task = new MandelbrotTask();

					//Assign the task a unique ID.
					task->assignID();
					task->setNodeTargetType(cf::Task::NodeTargetTypes::Remote);
					task->allowNodeTaskSplit = false;

					((MandelbrotTask *)task)->zoom = mb.getNewZoom(mb.zoom, 1);
					((MandelbrotTask *)task)->offsetX = mb.offsetX;
					((MandelbrotTask *)task)->offsetY = mb.offsetY;
					((MandelbrotTask *)task)->spaceWidth = IMAGE_WIDTH;
					((MandelbrotTask *)task)->spaceHeight = IMAGE_HEIGHT;
					((MandelbrotTask *)task)->minY = 0;
					((MandelbrotTask *)task)->maxY = IMAGE_HEIGHT - 1;

					unsigned __int64 taskID = task->getInitialTaskID();

					host->addTaskToQueue(task);

					//Create new cache entry for this zoom level.
					MandelbrotViewData mvd;
					mvd.zoom = ((MandelbrotTask *)task)->zoom;
					mvd.offsetX = ((MandelbrotTask *)task)->offsetX;
					mvd.offsetY = ((MandelbrotTask *)task)->offsetY;
					mvd.result = nullptr;
					mvd.taskID = task->getInitialTaskID();
					mvd.cacheEntryID = mb.nextCacheID++;
					mb.cache.push_back(mvd);
				}

				{
					cf::Task *task = new MandelbrotTask();

					//Assign the task a unique ID.
					task->assignID();
					task->setNodeTargetType(cf::Task::NodeTargetTypes::Remote);
					task->allowNodeTaskSplit = false;

					((MandelbrotTask *)task)->zoom = mb.getNewZoom(mb.zoom, 2);
					((MandelbrotTask *)task)->offsetX = mb.offsetX;
					((MandelbrotTask *)task)->offsetY = mb.offsetY;
					((MandelbrotTask *)task)->spaceWidth = IMAGE_WIDTH;
					((MandelbrotTask *)task)->spaceHeight = IMAGE_HEIGHT;
					((MandelbrotTask *)task)->minY = 0;
					((MandelbrotTask *)task)->maxY = IMAGE_HEIGHT - 1;

					unsigned __int64 taskID = task->getInitialTaskID();

					host->addTaskToQueue(task);

					//Create new cache entry for this zoom level.
					MandelbrotViewData mvd;
					mvd.zoom = ((MandelbrotTask *)task)->zoom;
					mvd.offsetX = ((MandelbrotTask *)task)->offsetX;
					mvd.offsetY = ((MandelbrotTask *)task)->offsetY;
					mvd.result = nullptr;
					mvd.taskID = task->getInitialTaskID();
					mvd.cacheEntryID = mb.nextCacheID++;
					mb.cache.push_back(mvd);
				}
			}

			//Send any tasks in the queue.
			if (host->getTasksCount() > 0) host->sendTasks();

			//Marry results to their cached tasks.
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

		}

		if (window.isOpen())
		{
			if (stateChanged)
			{
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
}