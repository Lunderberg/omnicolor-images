#ifndef _UNIQUEPALETTE_H_
#define _UNIQUEPALETTE_H_

#include <vector>
#include <functional>
#include <random>
#include <memory>

#include "KDTree.hh"

struct Color{
	Color(unsigned char r=0, unsigned char g=0, unsigned char b=0)
		: r(r), g(g), b(b) { }
	unsigned char r,g,b;

	enum {dimensions = 3};
	int get(int n) const {
		switch(n){
		case 0:
			return r;
		case 1:
			return g;
		case 2:
			return b;
		default:
			assert(false);
		}
	}
	int set(int n, unsigned char value){
		switch(n){
		case 0:
			r = value;
			break;
		case 1:
			g = value;
			break;
		case 2:
			b = value;
			break;
		default:
			assert(false);
		}
	}
};

class UniquePalette{
public:
	UniquePalette();
	Color PopClosest(Color col);
	Color PopBack();
	Color PopRandom(std::mt19937& rng);

	void GenerateUniformPalette(int n_colors);
private:
	std::unique_ptr<KDTree<Color> > colors;
};

#endif /* _UNIQUEPALETTE_H_ */
