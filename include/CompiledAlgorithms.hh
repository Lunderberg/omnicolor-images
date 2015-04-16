#ifndef _COMPILEDALGORITHMS_H_
#define _COMPILEDALGORITHMS_H_

#include <algorithm>
#include <functional>
#include <random>
#include <vector>

#include "Color.hh"
#include "GrowthImage.hh"
#include "Point.hh"

std::vector<Color> generate_uniform_palette(RandomInt, int n_colors);

std::vector<Point> generate_random_start(RandomInt rand, int width, int height);

Point generate_frontier_location(RandomInt rand, const PointTracker& point_tracker);

class generate_sequential_location{
public:
  generate_sequential_location(int width, int height)
    : width(width), height(height), i(-1), j(0) { }
  Point operator()(RandomInt rand, const PointTracker& point_tracker);
private:
  int width, height;
  int i,j;
};

class generate_preferred_location{
public:
  generate_preferred_location(int n)
    : n(std::max(n,1)) { }
  Point operator()(RandomInt rand, const PointTracker& p);
private:
  int n;
};

double generate_null_preference(RandomInt, Point p, const PointTracker& point_tracker);

class generate_location_preference{
public:
  double operator()(RandomInt rand, Point p, const PointTracker& point_tracker);
private:
  Point goal_loc;
};

class generate_perlin_preference{
public:
  generate_perlin_preference(double grid_size, int octaves, std::mt19937& rng)
    : perlin(rng) {
    perlin.SetGridSize(grid_size);
    perlin.SetOctaves(octaves);
  }

  double operator()(RandomInt, Point p, const PointTracker&){
    return perlin(p.i, p.j);
  }
private:
  PerlinNoise perlin;
};

Color generate_average_color(RandomInt rand, std::vector<Color> neighbors, Point p);

#endif /* _COMPILEDALGORITHMS_H_ */
