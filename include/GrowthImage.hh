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

typedef std::function<int(int,int)> RandomInt;
typedef std::function<std::vector<Color>(RandomInt,int)> PaletteGenerator;
typedef std::function<std::vector<Point>(RandomInt,int,int)> InitialLocationGenerator;

std::vector<Color> generate_uniform_palette(RandomInt, int n_colors);
std::vector<Point> generate_random_start(RandomInt rand, int width, int height);

class GrowthImage{
public:
  GrowthImage(int width, int height, int seed);

  void SetPaletteGenerator(PaletteGenerator func);
  void SetInitialLocationGenerator(InitialLocationGenerator func);

  void GenerateUniformPalette(int colors);
  void Seed(int seed);

  void SetColorChoice(ColorChoice c);
  void SetLocationChoice(LocationChoice c);
  void SetPreferenceChoice(PreferenceChoice c);
  void SetPreferredLocationIterations(int n);

  void SetPerlinOctaves(int octaves);
  void SetPerlinGridSize(double grid_size);

  void SetEpsilon(double epsilon);

  void Reset();
  bool Iterate();
  void IterateUntilDone();
  void ExtendFrontier(Point loc, Color color);

  void Save(const char* filepath);
  void Save(const std::string& filepath);

  int GetWidth();
  int GetHeight();

  double GetEpsilon();

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
  PaletteGenerator palette_generator;
  InitialLocationGenerator initial_location_generator;

  ColorChoice color_choice;
  LocationChoice location_choice;
  PreferenceChoice preference_choice;
  double epsilon;

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
  RandomInt rand_int;
  PerlinNoise perlin;
};

#endif /* _GROWTHIMAGE_H_ */
