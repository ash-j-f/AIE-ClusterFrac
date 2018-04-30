#pragma once
#include <array>
#include <vector>
#include <thread>
#include <string>
#include <stdio.h>
#include <SFML\Graphics.hpp>
#include "Host.h"
#include "MandelbrotTask.hpp"
#include "MandelbrotResult.hpp"
#include "MandelbrotViewData.hpp"

/**
* Mandelbrot class used to manage, calculate and display the Mandelbrot set at a chosen zoom level and offset.
* Provides user input for navigating the Mandelbrot set in real time.
* @author Ashley Flynn - Academy of Interactive Entertainment - 2018.
*/
class Mandelbrot {

public:

	/**
	* Constructor.
	* @param newHost The ClusterFrac host object.
	*/
	Mandelbrot(cf::Host *newHost);

	/**
	* Default destructor.
	*/
	~Mandelbrot();

	//Current zoom level.
	int zoomLevel;
	
	//Starting zoom at zoomLevel 0.
	double defaultZoom;

	//Current view zoom.
	double zoom;

	//Current view offsetX.
	double offsetX;

	//Current view offsetY.
	double offsetY;

	//Cache of view pixel data for various zoom and camera offset positions.
	std::vector<MandelbrotViewData> cache;

	//Next available cache id.
	unsigned int nextCacheID;

	/**
	* Get a new zoom value based on a starting zoom value and a zoom factor.
	* @param currentZoom The current zoom value.
	* @param factor A positive or negative integer repreenting the number of 
	* times to zoom in or out from the current zoom level.
	* @returns The new zoom value.
	*/
	double getNewZoom(int factor) const;

	/**
	* Get a new offsetY value based on a starting value and a move factor.
	* @param currentOffsetY The current offsetY value.
	* @param currentZoom The current zoom value.
	* @param factor A positive or negative integer representing the number of
	* times to move the camera one step, up (-1) or down (+1).
	* @returns The new offsetY value.
	*/
	double getNewOffsetY(double currentOffsetY, double currentZoom, int factor) const;

	/**
	* Get a new offsetX value based on a starting value and a move factor.
	* @param currentOffsetX The current offsetX value.
	* @param currentZoom The current zoom value.
	* @param factor A positive or negative integer representing the number of
	* times to move the camera one step, left (-1) or right (+1).
	* @returns The new offsetX value.
	*/
	double getNewOffsetX(double currentOffsetY, double currentZoom, int factor) const;
	
	/**
	* Create a new view with a given zoom and offset. This creates a ClusterFrac task to generate the view data,
	* and creates an entry in the cache for this zoom and offset linked to the new task ID.
	* @param zoom The zoom level to use.
	* @param offsetX The offset in the X dimension.
	* @param offsetY The offset in the Y dimension.
	* @param imageWidth The image width.
	* @param imageHeight The image height.
	* @returns void.
	*/
	void newView(double zoom, double offsetX, double offsetY, unsigned int imageWidth, unsigned int imageHeight);
	
	/**
	* Save current zoom and offset data to disk.
	* @returns void.
	*/
	void save() const;

	/**
	* Load zoom and offset data from disk.
	* @returns void.
	*/
	void load();

	/**
	* Reset the view zoom and offset to defaults.
	* @returns void.
	*/
	void reset();

	/**
	* Reset the view zoom to default.
	* @returns void.
	*/
	void resetZoomOnly();

	/**
	* Remove excess cache results from cache and from host results list.
	* Won't purge cache entries that are still pending (no results set yet attached).
	* @param maxCacheResults The maximum number of results to store in the cache. 
	* The oldest entries in excess of this number are removed.
	* @returns void.
	*/
	void purgeCache(int maxCacheResults);

	/**
	* Get the color value from the color table at the given index.
	* @param index The table index to use.
	* @returns The color value at the given index.
	*/
	inline const sf::Color getColor(int index) const { return colors[index];  };

	/**
	* Get the full path to the location of this executable file.
	* @returns The full path to the location of this executable file.
	*/
	std::string getExecutableFolder() const;

private:
	
	//Pointer to the ClusterFrac host object.
	cf::Host *host;

	//Maximum number of iterations for Mandelbrot calculations.
	static const sf::Uint8 MAX = 255;

	//The color table for rendering the Mandelbrot set.
	std::array<sf::Color, MAX + 1> colors;

	/**
	* Get a color value based on a Mandelbrot iteration count.
	* Colouring method from https://solarianprogrammer.com/2013/02/28/mandelbrot-set-cpp-11/
	* @param iterations The Mandelbrot iteration value for a point.
	* @returns An sf::Color representing the given number of iterations.
	*/
	sf::Color createColor(int iterations) const;
};