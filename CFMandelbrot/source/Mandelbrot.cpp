#include "Mandelbrot.h"

Mandelbrot::Mandelbrot(cf::Host *newHost) {
	host = newHost;
	for (int i = 0; i <= MAX; ++i) {
		colors[i] = getColor(i);
	}
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

//void Mandelbrot::updateImageSlice(double zoom, double offsetX, double offsetY, sf::Image& image, int minY, int maxY) const
//{
//	double real = 0 * zoom - IMAGE_WIDTH / 2.0 * zoom + offsetX;
//	double imagstart = minY * zoom - IMAGE_HEIGHT / 2.0 * zoom + offsetY;
//	for (int x = 0; x < IMAGE_WIDTH; x++, real += zoom) {
//		double imag = imagstart;
//		for (int y = minY; y < maxY; y++, imag += zoom) {
//			int value = mandelbrot(real, imag);
//			image.setPixel(x, y, colors[value]);
//		}
//	}
//}

void Mandelbrot::updateImage(double zoom, double offsetX, double offsetY, sf::Image& image, unsigned int imageWidth, unsigned int imageHeight) const
{
	cf::Task *task = new MandelbrotTask();

	//Assign the task a unique ID.
	task->assignID();

	((MandelbrotTask *)task)->zoom = zoom;
	((MandelbrotTask *)task)->offsetX = offsetX;
	((MandelbrotTask *)task)->offsetY = offsetY;
	((MandelbrotTask *)task)->spaceWidth = imageWidth;
	((MandelbrotTask *)task)->spaceHeight = imageHeight;
	((MandelbrotTask *)task)->minY = 0;
	((MandelbrotTask *)task)->maxY = imageHeight - 1;

	unsigned __int64 taskID = task->getInitialTaskID();

	host->addTaskToQueue(task);

	//Wait until task is sent. Will wait for at least 1 client to be connected.
	while (!host->sendTasks());

	//Wait for results to be complete.
	while (!host->checkAvailableResult(taskID));

	cf::Result *finished = host->getAvailableResult(taskID);
	MandelbrotResult *output = static_cast<MandelbrotResult *>(finished);

	int count = (int)output->numbers.size();
	for (int p = 0; p < count; p++)
	{
		image.setPixel(output->x[p], output->y[p], colors[output->numbers[p]]);
	}

	/*const int STEP = IMAGE_HEIGHT / std::thread::hardware_concurrency();
	std::vector<std::thread> threads;
	for (int i = 0; i < IMAGE_HEIGHT; i += STEP) {
	threads.push_back(std::thread(&Mandelbrot::updateImageSlice, *this, zoom, offsetX, offsetY, std::ref(image), i, std::min(i + STEP, IMAGE_HEIGHT)));
	}
	for (auto &t : threads) {
	t.join();
	}*/
}