#ifndef _UNIQUEPALETTE_H_
#define _UNIQUEPALETTE_H_

#include <vector>
#include <functional>
#include <random>

struct Color{
	Color(unsigned char r, unsigned char g, unsigned char b)
		: r(r), g(g), b(b) { }
	unsigned char r,g,b;
};

struct ColorEntry{
	ColorEntry(Color c) : color(c), used(false), magnitude(-1) { }
	Color color;
	bool used;
	double magnitude;
	bool operator<(const ColorEntry& c) const{
		return magnitude < c.magnitude;
	}
};

class UniquePalette{
public:
	UniquePalette();
	Color PopClosest(Color col);
	Color PopBack();
	Color PopRandom(std::mt19937& rng);

	void GenerateUniformPalette(int n_colors);

	void SetDistanceFunction(std::function<double(Color,Color)> func){distance = func;}

private:
	void PurgeUsedColors(bool force=false);

	std::vector<ColorEntry> colors;
	int iterations_since_purge;
	std::function<double(Color,Color)> distance;
};

#endif /* _UNIQUEPALETTE_H_ */
