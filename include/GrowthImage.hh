#ifndef _GROWTHIMAGE_H_
#define _GROWTHIMAGE_H_

#include <vector>

#include <boost/gil/gil_all.hpp>

struct Point{
	Point(int i, int j) : i(i), j(j) {}
	int i,j;

	bool operator==(const Point& p) const {
		return p.i==i && p.j==j;
	}
};

enum class ColorChoice {Nearest, Ordered, Sequential};

class GrowthImage{
public:
	GrowthImage(int width, int height, int colors, ColorChoice color_choice=ColorChoice::Nearest);

	void GenerateUniformPalette(int colors);

	void Reset();
	void FirstIteration();
	void Iterate();
	void IterateUntilDone();
	Point ChooseLocation();
	boost::gil::rgb8_pixel_t ChooseColor(Point loc);

	boost::gil::rgb8_pixel_t ChooseNearestColor(Point loc);
	boost::gil::rgb8_pixel_t ChooseOrderedColor(Point loc);
	boost::gil::rgb8_pixel_t ChooseSequentialColor(Point loc);


	void Save(const char* filepath);
private:
	boost::gil::rgb8_image_t image;
	boost::gil::rgb8_image_t::view_t view;
	std::vector<boost::gil::rgb8_pixel_t> palette;
	std::vector<std::vector<bool> > filled;
	Point previous_loc;
	std::vector<Point> frontier;
	ColorChoice color_choice;
};

#endif /* _GROWTHIMAGE_H_ */
