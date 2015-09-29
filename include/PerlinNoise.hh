#ifndef _PERLINNOISE_H_
#define _PERLINNOISE_H_

#include <vector>
#include <random>

#include "GVector.hh"

// Creates 2-d Perlin noise
class PerlinNoise{
public:
  PerlinNoise(std::mt19937& rng);
  double operator()(double x, double y);
  double operator()(GVector<2> p);

  void SetOctaves(int octaves){
    this->octaves = octaves;
  }
  int GetOctaves(){
    return octaves;
  }

  void SetGridSize(double grid_size){
    this->grid_size = grid_size;
  }
  double GetGridSize(){
    return grid_size;
  }

private:
  double base_perlin(GVector<2> p);

  double interpolate(double v0, double v1, double t);
  GVector<2> gradient_at(int i, int j);

  std::array<GVector<2>,256> gradients;
  std::array<unsigned char,256> permute;

  int octaves;
  double grid_size;
};

#endif /* _PERLINNOISE_H_ */
