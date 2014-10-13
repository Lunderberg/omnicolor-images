#ifndef _GROWTHIMAGE_H_
#define _GROWTHIMAGE_H_

#include <vector>
#include <list>
#include <cmath>
#include <string>

#include <boost/gil/gil_all.hpp>

#include "SmartEnum.hh"

struct Point{
	Point(int i=-1, int j=-1) : i(i), j(j), preference(0) {}
	int i,j;
	double preference;

	bool operator==(const Point& p) const {
		return p.i==i && p.j==j;
	}
};

struct Color{
	Color(unsigned char r, unsigned char g, unsigned char b)
		: r(r), g(g), b(b), used(false) {
		magnitude = std::sqrt(r*r + g*g + b*b);
	}
	unsigned char r,g,b;
	bool used;
	double magnitude;
	boost::gil::rgb8_pixel_t gil_color(){return {r,g,b};}
	bool operator<(const Color& other) const { return magnitude<other.magnitude; }
};

SmartEnum(ColorChoice, Nearest, Ordered, Sequential);
SmartEnum(LocationChoice, Random, Snaking, Preferred);

class GrowthImage{
public:
	GrowthImage(int width, int height, int seed);

	void GenerateUniformPalette(int colors);
	void Seed(int seed);

	void SetColorChoice(ColorChoice c);
	void SetLocationChoice(LocationChoice c);
	void SetPreferredLocationIterations(int n);

	void Reset();
	bool Iterate();
	void IterateUntilDone();

	void Save(const char* filepath);
	void Save(const std::string& filepath);

	int GetWidth();
	int GetHeight();

private:
	void FirstIteration();

	Point ChooseLocation();
	Point ChooseFrontierLocation();
	Point ChooseSnakingLocation();
	Point ChoosePreferredLocation(int n_check);

	double ChoosePreference(Point p, boost::gil::rgb8_pixel_t color);

	boost::gil::rgb8_pixel_t ChooseColor(Point loc);
	boost::gil::rgb8_pixel_t ChooseNearestColor(Point loc);
	boost::gil::rgb8_pixel_t PopClosestColor(double r, double g, double b);
	boost::gil::rgb8_pixel_t ChooseOrderedColor(Point loc);
	boost::gil::rgb8_pixel_t ChooseSequentialColor(Point loc);
	void PurgeUsedColors();

private:
	ColorChoice color_choice;
	LocationChoice location_choice;

	boost::gil::rgb8_image_t image;
	boost::gil::rgb8_image_t::view_t view;

	std::vector<Color> palette;
	std::vector<std::vector<bool> > filled;
	std::vector<Point> frontier;

	Point previous_loc;
	Point goal_loc;
	int preferred_location_iterations;

	int iterations_since_purge;
	std::mt19937 rng;
};

#endif /* _GROWTHIMAGE_H_ */
