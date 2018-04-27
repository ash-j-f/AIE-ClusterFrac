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

class Mandelbrot {
public:
	Mandelbrot(cf::Host *newHost);

	double offsetX;
	double offsetY;
	double zoom;

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
	double getNewZoom(double currentZoom, int factor) const;

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
	
	void newView(double zoom, double offsetX, double offsetY, unsigned int imageWidth, unsigned int imageHeight);
	
	void save() const;
	void load();
	void reset();
	void resetZoomOnly();

	/**
	* Remove excess cache results from cache and from host results list.
	* Won't purge cache entries that are still pending (no results set yet attached).
	* @param maxCacheResults The maximum number of results to store in the cache. 
	* The oldest entries in excess of this number are removed.
	* @returns void.
	*/
	void purgeCache(int maxCacheResults);

	inline const sf::Color getColor(int index) const { return colors[index];  };

	std::string getExecutableFolder() const;

private:
	cf::Host *host;
	static const sf::Uint8 MAX = 255; // maximum number of iterations for mandelbrot()
	std::array<sf::Color, MAX + 1> colors;
	sf::Color createColor(int iterations) const;
};