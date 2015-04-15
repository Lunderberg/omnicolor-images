#ifndef _GROWTHIMAGE_H_
#define _GROWTHIMAGE_H_

#include <vector>
#include <list>
#include <cmath>
#include <string>
#include <unordered_set>
#include <functional>

#include <boost/gil/gil_all.hpp>

#include "PerlinNoise.hh"
#include "Point.hh"
#include "PointTracker.hh"
#include "SmartEnum.hh"
#include "UniquePalette.hh"

SmartEnum(ColorChoice, Nearest, Sequential, Perlin);
SmartEnum(LocationChoice, Random, Preferred, Sequential);
SmartEnum(PreferenceChoice, Location, Perlin);

typedef std::function<int(int,int)> RandomInt;
typedef std::function<std::vector<Color>(RandomInt,int)> PaletteGenerator;
typedef std::function<std::vector<Point>(RandomInt,int,int)> InitialLocationGenerator;
typedef std::function<Point(RandomInt,const PointTracker&)> LocationGenerator;
typedef std::function<double(RandomInt,Point,const PointTracker&)> PreferenceGenerator;

class GrowthImage{
public:
  GrowthImage(int width, int height, int seed);

  void SetPaletteGenerator(PaletteGenerator func);
  void SetInitialLocationGenerator(InitialLocationGenerator func);
  void SetLocationGenerator(LocationGenerator func);
  void SetPreferenceGenerator(PreferenceGenerator func);

  void Seed(int seed);

  void SetPerlinOctaves(int octaves);
  void SetPerlinGridSize(double grid_size);

  void SetEpsilon(double epsilon);

  void Reset();
  bool Iterate();
  void IterateUntilDone();

  void Save(const char* filepath);
  void Save(const std::string& filepath);

  int GetWidth();
  int GetHeight();

  double GetEpsilon();

  std::mt19937& GetRNG() { return rng; }

private:
  void FirstIteration();

  Point ChooseLocation();

  double ChoosePreference(Point p);
  double ChoosePreferencePerlin(Point p);

  Color ChooseColor(Point loc);
  Color ChooseNearestColor(Point loc);
  Color ChooseSequentialColor(Point loc);
  Color ChoosePerlinColor(Point loc);

private:
  PaletteGenerator palette_generator;
  InitialLocationGenerator initial_location_generator;
  LocationGenerator location_generator;
  PreferenceGenerator preference_generator;

  PointTracker point_tracker;

  ColorChoice color_choice;
  LocationChoice location_choice;
  PreferenceChoice preference_choice;
  double epsilon;

  UniquePalette palette;

  boost::gil::rgb8_image_t image;
  boost::gil::rgb8_image_t::view_t view;

  Point previous_loc;
  Point goal_loc;
  int preferred_location_iterations;

  std::mt19937 rng;
  RandomInt rand_int;
  PerlinNoise perlin;
};

#endif /* _GROWTHIMAGE_H_ */
