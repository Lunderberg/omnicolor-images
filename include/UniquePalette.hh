#ifndef _UNIQUEPALETTE_H_
#define _UNIQUEPALETTE_H_

#include <cassert>
#include <vector>
#include <functional>
#include <random>
#include <memory>

#include "Color.hh"

template<typename T>
class KDTree;

class UniquePalette{
public:
  UniquePalette();
  ~UniquePalette();
  Color PopClosest(Color col, double epsilon = 0);
  Color PopBack();
  Color PopRandom(std::mt19937& rng);

  void SetPalette(std::vector<Color> colors);
  int ColorsRemaining();

  void GenerateUniformPalette(int n_colors);
private:
  std::unique_ptr<KDTree<Color> > colors;
};

#endif /* _UNIQUEPALETTE_H_ */
