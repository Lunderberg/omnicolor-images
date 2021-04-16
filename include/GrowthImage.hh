#ifndef _GROWTHIMAGE_H_
#define _GROWTHIMAGE_H_

#include <vector>
#include <list>
#include <cmath>
#include <string>
#include <unordered_set>
#include <functional>

#include "PerlinNoise.hh"
#include "Point.hh"
#include "PointTracker.hh"
#include "SmartEnum.hh"
#include "UniquePalette.hh"

namespace Lua{
  class LuaState;
}

typedef std::function<int(int,int)> RandomInt;
typedef std::function<std::vector<Color>(RandomInt,int)> PaletteGenerator;
typedef std::function<std::vector<Point>(RandomInt,int,int)> InitialLocationGenerator;
typedef std::function<Point(RandomInt,const PointTracker&)> LocationGenerator;
typedef std::function<double(RandomInt,Point,const PointTracker&)> PreferenceGenerator;
typedef std::function<Color(RandomInt,std::vector<Color>,Point)> TargetColorGenerator;

class GrowthImage{
public:
  GrowthImage(int width, int height, int seed);
  GrowthImage(const char* luascript_filename);

  ~GrowthImage();

  void SetPaletteGenerator(PaletteGenerator func);
  void SetInitialLocationGenerator(InitialLocationGenerator func);
  void SetLocationGenerator(LocationGenerator func);
  void SetPreferenceGenerator(PreferenceGenerator func);
  void SetTargetColorGenerator(TargetColorGenerator func);

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
  Color ChooseColor(Point loc);

private:
  size_t get_index(int i, int j);
  size_t get_index(Point p);

  Lua::LuaState* state;

  PaletteGenerator palette_generator;
  InitialLocationGenerator initial_location_generator;
  LocationGenerator location_generator;
  PreferenceGenerator preference_generator;
  TargetColorGenerator target_color_generator;

  PointTracker point_tracker;

  double epsilon;

  UniquePalette palette;

  int width;
  int height;
  std::vector<Color> pixels;

  std::mt19937 rng;
  RandomInt rand_int;
};

#endif /* _GROWTHIMAGE_H_ */
