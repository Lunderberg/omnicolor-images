#ifndef _GROWTHIMAGE_H_
#define _GROWTHIMAGE_H_

#include <vector>
#include <list>
#include <cmath>
#include <string>
#include <unordered_set>
#include <functional>

#include <boost/gil/gil_all.hpp>

#include "SmartEnum.hh"
#include "UniquePalette.hh"
#include "PerlinNoise.hh"

struct Point{
  Point(int i=-1, int j=-1) : i(i), j(j), preference(0) {}
  int i,j;
  double preference;

  bool operator==(const Point& p) const {
    return p.i==i && p.j==j;
  }
};

namespace std{
  template<>
  struct hash<Point>{
    size_t operator()(const Point& p) const{
      return 1000*p.i + p.j;
    }
  };
}

SmartEnum(ColorChoice, Nearest, Sequential, Perlin);
SmartEnum(LocationChoice, Random, Snaking, Preferred, Sequential);
SmartEnum(PreferenceChoice, Location, Perlin);

class GrowthImage{
public:
  GrowthImage(int width, int height, int seed);

  void GenerateUniformPalette(int colors);
  void Seed(int seed);

  void SetColorChoice(ColorChoice c);
  void SetLocationChoice(LocationChoice c);
  void SetPreferenceChoice(PreferenceChoice c);
  void SetPreferredLocationIterations(int n);

  void SetPerlinOctaves(int octaves);
  void SetPerlinGridSize(double grid_size);

  void Reset();
  bool Iterate();
  void IterateUntilDone();
  void ExtendFrontier(Point loc, Color color);

  void Save(const char* filepath);
  void Save(const std::string& filepath);

  int GetWidth();
  int GetHeight();

private:
  void FirstIteration();

  Point ChooseLocation();
  Point ChooseFrontierLocation();
  Point ChooseSnakingLocation();
  Point ChooseSequentialLocation();
  Point ChoosePreferredLocation(int n_check);

  double ChoosePreference(Point p, Color color);
  double ChoosePreferenceLocation(Point p, Color color);
  double ChoosePreferencePerlin(Point p, Color color);

  Color ChooseColor(Point loc);
  Color ChooseNearestColor(Point loc);
  Color ChooseSequentialColor(Point loc);
  Color ChoosePerlinColor(Point loc);

private:
  ColorChoice color_choice;
  LocationChoice location_choice;
  PreferenceChoice preference_choice;

  UniquePalette palette;

  boost::gil::rgb8_image_t image;
  boost::gil::rgb8_image_t::view_t view;

  std::vector<std::vector<bool> > filled;
  std::unordered_set<Point> frontier_set;
  std::vector<Point> frontier_vector;

  Point previous_loc;
  Point goal_loc;
  int preferred_location_iterations;

  std::mt19937 rng;
  PerlinNoise perlin;
};

#endif /* _GROWTHIMAGE_H_ */
