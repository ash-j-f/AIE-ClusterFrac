//int main(int argc, //Number of strings in array argv  
//	char *argv[], //Array of command-line argument strings  
//	char *envp[]) // Array of environment variable strings  
//{
//}

#include <mpir.h>
#include <stdio.h>
#include <SFML\Graphics.hpp>
#include <array>
#include <vector>
#include <thread>
#include <string>

static constexpr int IMAGE_WIDTH = 1680;
static constexpr int IMAGE_HEIGHT = 1050;

class Mandelbrot {
public:
	Mandelbrot();
	void updateImage(mpf_t zoomIN, mpf_t offsetXIN, mpf_t offsetYIN, sf::Image& image) const;
private:
	static const int MAX = 127; // maximum number of iterations for mandelbrot()
								// don't increase MAX or the colouring will look strange
	std::array<sf::Color, MAX + 1> colors;
	
	int mandelbrot(mpf_t startReal, mpf_t startImag) const;
	sf::Color getColor(int iterations) const;
	void updateImageSlice(mpf_t zoomIN, mpf_t offsetXIN, mpf_t offsetYIN, sf::Image& image, int minY, int maxY) const;
};

Mandelbrot::Mandelbrot() {
	for (int i = 0; i <= MAX; ++i) {
		colors[i] = getColor(i);
	}
}

int Mandelbrot::mandelbrot(mpf_t startReal, mpf_t startImag) const {
	
	mpf_t zReal;
	mpf_init_set(zReal, startReal);
	mpf_t zImag;
	mpf_init_set(zImag, startImag);
	
	mpf_t r2;
	mpf_init(r2);
	mpf_t i2;
	mpf_init(i2);
	mpf_t two;
	mpf_init_set_d(two, 2.0);
	for (int counter = 0; counter < MAX; ++counter) {
		mpf_mul(r2, zReal, zReal);
		mpf_mul(i2, zImag, zImag);
		
		mpf_t added;
		mpf_init(added);
		mpf_add(added, r2, i2);
		if (mpf_cmp_d(added, 4.0) > 0) {
			return counter;
		}

		mpf_mul(zImag, zImag, zReal);
		mpf_mul(zImag, zImag, two);
		mpf_add(zImag, zImag, startImag);
		
		//?? What is order of precedence here? zReal = r2 - i2 + startReal;
		mpf_add(zReal, i2, startReal);
		mpf_sub(zReal, r2, zReal);

		mpf_init_set_d(r2, 0.0);
		mpf_init_set_d(i2, 0.0);
	}
	return MAX;
}

sf::Color Mandelbrot::getColor(int iterations) const {
	int r, g, b;

	// colour gradient:      Red -> Blue -> Green -> Red -> Black
	// corresponding values:  0  ->  16  ->  32   -> 64  ->  127 (or -1)
	if (iterations < 16) {
		r = 16 * (16 - iterations);
		g = 0;
		b = 16 * iterations - 1;
	}
	else if (iterations < 32) {
		r = 0;
		g = 16 * (iterations - 16);
		b = 16 * (32 - iterations) - 1;
	}
	else if (iterations < 64) {
		r = 8 * (iterations - 32);
		g = 8 * (64 - iterations) - 1;
		b = 0;
	}
	else { // range is 64 - 127
		r = 255 - (iterations - 64) * 4;
		g = 0;
		b = 0;
	}
	return sf::Color(r, g, b);
}

void Mandelbrot::updateImageSlice(mpf_t zoom, mpf_t offsetX, mpf_t offsetY, sf::Image& image, int minY, int maxY) const
{
	mpf_t real;
	mpf_init(real);
	
	mpf_t imagstart;
	mpf_init(imagstart);
	//ORIGINAL:
	//double real = (x - IMAGE_WIDTH / 2.0) * zoom + offsetX;
	//double imag = (y - IMAGE_HEIGHT / 2.0) * zoom + offsetY;
	//NEW ALETERNATIVE:
	//double real = 0 * zoom - IMAGE_WIDTH / 2.0 * zoom + offsetX;
	//double imagstart = minY * zoom - IMAGE_HEIGHT / 2.0 * zoom + offsetY;
	
	//TODO - Convert the following to use ONLY MPIR.

	double NEWreal = 0 * mpf_get_d(zoom) - IMAGE_WIDTH / 2.0 * mpf_get_d(zoom) + mpf_get_d(offsetX);
	double NEWimagstart = minY * mpf_get_d(zoom) - IMAGE_HEIGHT / 2.0 * mpf_get_d(zoom) + mpf_get_d(offsetY);

	mpf_set_d(real, NEWreal);
	mpf_set_d(imagstart, NEWimagstart);

	for (int x = 0; x < IMAGE_WIDTH; x++) {
		//double imag = imagstart;
		mpf_t imag;
		mpf_init_set(imag, imagstart);
		for (int y = minY; y < maxY; y++) {
			int value = mandelbrot(real, imag);
			image.setPixel(x, y, colors[value]);
			mpf_add(imag, imag, zoom);
		}
		mpf_add(real, real, zoom);
	}
}

void Mandelbrot::updateImage(mpf_t zoom, mpf_t offsetX, mpf_t offsetY, sf::Image& image) const
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

int main() {
	
	//mpf_set_default_prec(32);

	//PI example.
	//mpf_t two;
	//mpf_t four;
	//mpf_t sub, a, b, c;
	//mpf_t pi_ish;
	//mpf_inits(two, four, sub, a, b, c, pi_ish);
	//mpf_set_d(two, 2.0);
	//mpf_set_d(four, 4.0);
	//mpf_set_d(pi_ish, 3.0);
	//mpf_set_d(a, 2.0);
	//mpf_set_d(b, 3.0);
	//mpf_set_d(c, 4.0);

	//bool signPlus = true;
	//for (int i = 0; i <= 1000000; i++)
	//{
	//	mpf_mul(sub, a, b);
	//	mpf_mul(sub, sub, c);
	//	mpf_div(sub, four, sub);
	//	if (signPlus)
	//	{
	//		mpf_add(pi_ish, pi_ish, sub);
	//	}
	//	else
	//	{
	//		mpf_sub(pi_ish, pi_ish, sub);
	//	}

	//	mpf_add(a, a, two);
	//	mpf_add(b, b, two);
	//	mpf_add(c, c, two);
	//	signPlus = !signPlus;
	//}
	//
	//mp_exp_t exp;
	//char * tmp = mpf_get_str(NULL, &exp, 10, 1000, pi_ish);
	//printf("%s", tmp);

	//double offsetX = -0.7; // and move around
	mpf_t offsetX;
	mpf_init_set_d(offsetX, -0.7);
	//double offsetY = 0.0;
	mpf_t offsetY;
	mpf_init_set_d(offsetY, 0.0);
	//double zoom = 0.004; // allow the user to zoom in and out...
	mpf_t zoom;
	mpf_init_set_d(zoom, 0.004);

	mpf_t pointNine;
	mpf_init_set_d(pointNine, 0.9);

	mpf_t forty;
	mpf_init_set_d(forty, 40.0);

	mpf_t zoomForty;
	mpf_init(zoomForty);

	Mandelbrot mb;

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
					//zoom *= 0.9;
					mpf_mul(zoom, zoom, pointNine);
					break;
				case sf::Keyboard::Dash:
					//zoom /= 0.9;
					mpf_div(zoom, zoom, pointNine);
					break;
				case sf::Keyboard::W:
					//offsetY -= 40 * zoom;
					mpf_mul(zoomForty, zoom, forty);
					mpf_sub(offsetY, offsetY, zoomForty);
					break;
				case sf::Keyboard::S:
					//offsetY += 40 * zoom;
					mpf_mul(zoomForty, zoom, forty);
					mpf_add(offsetY, offsetY, zoomForty);
					break;
				case sf::Keyboard::A:
					//offsetX -= 40 * zoom;
					mpf_mul(zoomForty, zoom, forty);
					mpf_sub(offsetX, offsetX, zoomForty);
					break;
				case sf::Keyboard::D:
					//offsetX += 40 * zoom;
					mpf_mul(zoomForty, zoom, forty);
					mpf_add(offsetX, offsetX, zoomForty);
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
	}
}