#ifndef _UNIQUEPALETTE_H_
#define _UNIQUEPALETTE_H_

#include <cassert>
#include <vector>
#include <functional>
#include <random>
#include <memory>

struct Color{
  Color(unsigned char r, unsigned char g, unsigned char b)
    : r(r), g(g), b(b) { }
  unsigned char r,g,b;

  enum {dimensions = 3};
  int get(int n){
    switch(n){
    case 0:
      return r;
    case 1:
      return g;
    case 2:
      return b;
    }
    assert(false);
  }
};

template<typename T>
class KDTree;

class UniquePalette{
public:
  UniquePalette();
  ~UniquePalette();
  Color PopClosest(Color col, double epsilon = 0);
  Color PopBack();
  Color PopRandom(std::mt19937& rng);

  void GenerateUniformPalette(int n_colors);
private:
  std::unique_ptr<KDTree<Color> > colors;
};

#endif /* _UNIQUEPALETTE_H_ */
