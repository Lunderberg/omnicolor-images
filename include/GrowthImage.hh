#ifndef _GROWTHIMAGE_H_
#define _GROWTHIMAGE_H_

#include <vector>
#include <list>
#include <cmath>

#include <boost/gil/gil_all.hpp>

struct Point{
	Point(int i=0, int j=0) : i(i), j(j) {}
	int i,j;

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

enum class ColorChoice {Nearest, Ordered, Sequential};

class GrowthImage{
public:
	GrowthImage(int width, int height, int colors, ColorChoice color_choice=ColorChoice::Nearest,
							int seed=0);

	void GenerateUniformPalette(int colors);

	void Reset();
	void FirstIteration();
	bool Iterate();
	void IterateUntilDone();


	Point ChooseLocation();
	Point ChooseFrontierLocation();
	Point ChooseSnakingLocation();


	boost::gil::rgb8_pixel_t ChooseColor(Point loc);
	boost::gil::rgb8_pixel_t ChooseNearestColor(Point loc);
	boost::gil::rgb8_pixel_t PopClosestColor(double r, double g, double b);
	boost::gil::rgb8_pixel_t ChooseOrderedColor(Point loc);
	boost::gil::rgb8_pixel_t ChooseSequentialColor(Point loc);

	void PurgeUsedColors();


	void Save(const char* filepath);
private:
	boost::gil::rgb8_image_t image;
	boost::gil::rgb8_image_t::view_t view;
	std::vector<Color> palette;
	std::vector<std::vector<bool> > filled;
	Point previous_loc;
	std::vector<Point> frontier;
	ColorChoice color_choice;

	int iterations_since_purge;
	std::mt19937 rng;
};

#endif /* _GROWTHIMAGE_H_ */
