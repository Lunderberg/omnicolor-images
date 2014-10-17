#ifndef _GROWTHIMAGE_H_
#define _GROWTHIMAGE_H_

#include <vector>
#include <list>
#include <cmath>
#include <string>

#include <boost/gil/gil_all.hpp>

#include "SmartEnum.hh"
#include "UniquePalette.hh"

struct Point{
	Point(int i=-1, int j=-1) : i(i), j(j), preference(0) {}
	int i,j;
	double preference;

	bool operator==(const Point& p) const {
		return p.i==i && p.j==j;
	}
};

SmartEnum(ColorChoice, Nearest, Sequential);
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
	void ExtendFrontier(Point loc, Color color);

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

	double ChoosePreference(Point p, Color color);

	Color ChooseColor(Point loc);
	Color ChooseNearestColor(Point loc);
	Color ChooseSequentialColor(Point loc);
	void PurgeUsedColors();

private:
	ColorChoice color_choice;
	LocationChoice location_choice;

	UniquePalette palette;

	boost::gil::rgb8_image_t image;
	boost::gil::rgb8_image_t::view_t view;

	std::vector<std::vector<bool> > filled;
	std::vector<Point> frontier;

	Point previous_loc;
	Point goal_loc;
	int preferred_location_iterations;

	std::mt19937 rng;
};

#endif /* _GROWTHIMAGE_H_ */
