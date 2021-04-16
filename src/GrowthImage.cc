#include "GrowthImage.hh"


#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <ctime>
#include <iostream>
#include <stdexcept>

#include "lua-bindings/LuaState.hh"

#include "common.hh"
#include "CompiledAlgorithms.hh"
#include "SavePNG.hh"

GrowthImage::GrowthImage(int width, int height, int seed)
  : state(NULL),
    palette_generator(generate_uniform_palette),
    initial_location_generator(generate_random_start),
    location_generator(generate_frontier_location),
    preference_generator(generate_null_preference),
    target_color_generator(generate_average_color),
    point_tracker(width, height),
    epsilon(0),
    width(width),
    height(height),
    pixels(width*height, Color(0,0,0)),
    rng(seed ? seed : time(0)) {

  rand_int = [this](int a, int b){
    if(a >= b){
      throw std::runtime_error("Improper range for random numbers");
    }
    return std::uniform_int_distribution<int>(a,b-1)(rng);
  };
}

GrowthImage::GrowthImage(const char* luascript_filename)
  : point_tracker(0,0) {

  state = new Lua::LuaState;
  state->LoadSafeLibs();
  state->SetGarbageCollectPause(100);

  state->MakeClass<Color>("Color")
    .AddMethod("GetR", &Color::GetR)
    .AddMethod("GetG", &Color::GetG)
    .AddMethod("GetB", &Color::GetB);
  state->MakeClass<Point>("Point")
    .AddConstructor<int, int>("Point")
    .AddMethod("GetI", &Point::GetI)
    .AddMethod("GetJ", &Point::GetJ)
    .AddMethod("GetPreference", &Point::GetPreference);
  state->MakeClass<PointTracker>("PointTracker")
    .AddMethod("GetWidth", &PointTracker::GetWidth)
    .AddMethod("GetHeight", &PointTracker::GetHeight)
    .AddMethod<bool, int, int>("IsFilled", &PointTracker::IsFilled)
    .AddMethod("IsInFrontier", &PointTracker::IsInFrontier);

  state->SetGlobal("uniform_color_palette", generate_uniform_palette);
  state->SetGlobal("generate_random_start", generate_random_start);
  state->SetGlobal("choose_frontier_location", generate_frontier_location);
  state->SetGlobal("null_preference", generate_null_preference);
  state->SetGlobal("target_average_color", generate_average_color);

  state->LoadFile(luascript_filename);

  palette_generator = state->CastGlobal<PaletteGenerator>("color_palette");
  initial_location_generator = state->CastGlobal<InitialLocationGenerator>("initial_location");
  location_generator = state->CastGlobal<LocationGenerator>("next_location");
  preference_generator = state->CastGlobal<PreferenceGenerator>("location_preference");
  target_color_generator = state->CastGlobal<TargetColorGenerator>("target_color");

  width = state->CastGlobal<int>("width");
  height = state->CastGlobal<int>("height");
  pixels = std::vector<Color>(width*height, Color(0,0,0));

  epsilon = state->CastGlobal<double>("epsilon");
  int seed = state->CastGlobal<int>("seed");

  point_tracker = PointTracker(width, height);
  rng = std::mt19937(seed ? seed : time(0));

  rand_int = [this](int a, int b){
    if(a >= b){
      throw std::runtime_error("Improper range for random numbers");
    }
    auto output = std::uniform_int_distribution<int>(a,b-1)(rng);
    return output;
  };
}

GrowthImage::~GrowthImage(){
  if(state){
    delete state;
  }
}

void GrowthImage::Seed(int seed){
  rng = std::mt19937(seed);
}

int GrowthImage::GetWidth(){
  return width;
}

int GrowthImage::GetHeight(){
  return height;
}

void GrowthImage::SetPaletteGenerator(PaletteGenerator func){
  palette_generator = func;
}

void GrowthImage::SetInitialLocationGenerator(InitialLocationGenerator func){
  initial_location_generator = func;
}

void GrowthImage::SetLocationGenerator(LocationGenerator func){
  location_generator = func;
}

void GrowthImage::SetPreferenceGenerator(PreferenceGenerator func){
  preference_generator = func;
}

void GrowthImage::SetTargetColorGenerator(TargetColorGenerator func){
  target_color_generator = func;
}

void GrowthImage::SetEpsilon(double epsilon){
  this->epsilon = epsilon;
}

double GrowthImage::GetEpsilon(){
  return epsilon;
}

void GrowthImage::Reset(){
  point_tracker.Clear();
}

void GrowthImage::FirstIteration(){
  auto points = initial_location_generator(rand_int, GetWidth(), GetHeight());
  for(auto point : points){
    point_tracker.AddToFrontier(point);
  }
}

bool GrowthImage::Iterate(){
  if(!palette.ColorsRemaining()){
    palette.SetPalette(palette_generator(rand_int, GetWidth() * GetHeight()));
  }
  if(!point_tracker.FrontierSize()){
    FirstIteration();
  }

  auto loc = ChooseLocation();
  auto color = ChooseColor(loc);
  pixels[get_index(loc)] = color;

  point_tracker.Fill(
    loc,
    [&](Point pos) {
      return preference_generator(rand_int, pos, point_tracker);
    });

  return point_tracker.FrontierSize();
}

void GrowthImage::IterateUntilDone(){
  int body_size = 0;
  while(Iterate()){
    if(body_size % 100000 == 0){
      std::cout << "\r                                                   \r"
                << "Body: " << body_size << "\tFrontier: " << point_tracker.FrontierSize()
                << "\tUnexplored: " << pixels.size() - body_size - point_tracker.FrontierSize()
                << std::flush;
    }
    body_size++;
  }
  std::cout << std::endl;
}

Point GrowthImage::ChooseLocation(){
  return location_generator(rand_int, point_tracker);
}

Color GrowthImage::ChooseColor(Point loc){
  // Find the average surrounding color.
  std::vector<Color> neighbors;
  for(int di=-1; di<=1; di++){
    for(int dj=-1; dj<=1; dj++){
      Point p(loc.i+di,loc.j+dj);
      auto index = get_index(p);
      if((index!=size_t(-1)) &&
         point_tracker.IsFilled(p.i,p.j)){
        neighbors.push_back(pixels[index]);
      }
    }
  }

  Color target = target_color_generator(rand_int, std::move(neighbors), loc);
  return palette.PopClosest(target, epsilon);
}

size_t GrowthImage::get_index(int i, int j) {
  if ( i>=0 && i<GetWidth() &&
       j>=0 && j<GetHeight() ) {
    return j*width + i;
  } else {
    return -1;
  }
}

size_t GrowthImage::get_index(Point p) {
  return get_index(p.i, p.j);
}

void GrowthImage::Save(const char *filepath) {
  SavePNG(pixels, width, height, filepath);
}

void GrowthImage::Save(const std::string &filepath) {
  Save(filepath.c_str());
}
