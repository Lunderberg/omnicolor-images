#include "UniquePalette.hh"

#include <cassert>
#include <cmath>
#include <cfloat>
#include <algorithm>
#include <stdexcept>

#include "common.hh"

UniquePalette::UniquePalette()
  : colors(nullptr) { }

UniquePalette::~UniquePalette() { }

void UniquePalette::SetPalette(std::vector<Color> colors){
  this->colors = std::unique_ptr<KDTree<Color> >(new KDTree<Color>(std::move(colors)));
}

int UniquePalette::ColorsRemaining(){
  if(colors == nullptr){
    return 0;
  } else {
    return colors->GetNumLeaves();
  }
}

KDTree_Result<Color> UniquePalette::PopClosest(Color col, double epsilon){
  return colors->PopClosest(col, epsilon);
}

KDTree_Result<Color> UniquePalette::PopBack(){
  return colors->PopClosest({0,0,0});
}

KDTree_Result<Color> UniquePalette::PopRandom(std::mt19937& rng){
  return colors->PopClosest({
      (unsigned char)randint(rng,256),
      (unsigned char)randint(rng,256),
      (unsigned char)randint(rng,256)
    });
}

void UniquePalette::GenerateUniformPalette(int n_colors){
  assert(n_colors > 0);
  assert(n_colors <= (1<<24));

  double dim_size = std::pow(n_colors,1.0/3.0);

  std::vector<Color> colors;
  colors.reserve(n_colors);
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

    colors.push_back({
        (unsigned char)(r*255),
        (unsigned char)(g*255),
        (unsigned char)(b*255)
      });
  }

  SetPalette(std::move(colors));
}
