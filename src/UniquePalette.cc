#include "UniquePalette.hh"

#include <cassert>
#include <cmath>
#include <cfloat>
#include <algorithm>
#include <stdexcept>

#include "common.hh"
#include "KDTree.hh"

UniquePalette::UniquePalette() { }

UniquePalette::~UniquePalette() { }

void UniquePalette::GenerateUniformPalette(int n_colors){
  assert(n_colors > 0);
  assert(n_colors <= (1<<24));

  double dim_size = std::pow(n_colors,1.0/3.0);

  std::vector<Color> temp_colors;
  temp_colors.reserve(n_colors);
  for(int i=0; i < n_colors; i++){
    double val = i;
    val /= dim_size;
    double r = std::fmod(val,1);
    val = int(val);
    val /= dim_size;
    double g = std::fmod(val,1);
    val = int(val);
    val /= dim_size;
    double b = val;

    temp_colors.push_back({r*255,g*255,b*255});
  }

  colors = std::unique_ptr<KDTree<Color> >(new KDTree<Color>(std::move(temp_colors)));
}

Color UniquePalette::PopClosest(Color col, double epsilon){
  return colors->PopClosest(col, epsilon);
}

Color UniquePalette::PopBack(){
  return colors->PopClosest({0,0,0});
}

Color UniquePalette::PopRandom(std::mt19937& rng){
  return colors->PopClosest({randint(rng,256),randint(rng,256),randint(rng,256)});
}
