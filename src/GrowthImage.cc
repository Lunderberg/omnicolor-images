#include "GrowthImage.hh"


#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <ctime>
#include <iostream>

#include "common.hh"
#include "CompiledAlgorithms.hh"

GrowthImage::GrowthImage(int width, int height, int seed)
  : palette_generator(generate_uniform_palette),
    initial_location_generator(generate_random_start),
    location_generator(generate_frontier_location),
    preference_generator(generate_null_preference),
    point_tracker(width, height),
    color_choice(ColorChoice::Nearest), location_choice(LocationChoice::Random),
    preference_choice(PreferenceChoice::Location), epsilon(0),
    image(width,height), view(boost::gil::view(image)),
    previous_loc(-1,-1), preferred_location_iterations(10),
    rng(seed ? seed : time(0)), perlin(rng){

  rand_int = [this](int a, int b){
    return std::uniform_int_distribution<int>(a,b-1)(rng);
  };

  Reset();
}

void GrowthImage::Seed(int seed){
  rng = std::mt19937(seed);
}

int GrowthImage::GetWidth(){
  return image.width();
}

int GrowthImage::GetHeight(){
  return image.height();
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

void GrowthImage::SetColorChoice(ColorChoice c){
  color_choice = c;
}

void GrowthImage::SetLocationChoice(LocationChoice c){
  location_choice = c;
}

void GrowthImage::SetPreferenceChoice(PreferenceChoice c){
  preference_choice = c;
}

void GrowthImage::SetPerlinOctaves(int octaves){
  perlin.SetOctaves(octaves);
}

void GrowthImage::SetPerlinGridSize(double grid_size){
  perlin.SetGridSize(grid_size);
}

void GrowthImage::SetPreferredLocationIterations(int n){
  preferred_location_iterations = n;
}

void GrowthImage::SetEpsilon(double epsilon){
  this->epsilon = epsilon;
}

double GrowthImage::GetEpsilon(){
  return epsilon;
}

void GrowthImage::Reset(){
  point_tracker.Clear();

  FirstIteration();
}

void GrowthImage::FirstIteration(){
  auto points = initial_location_generator(rand_int, image.width(), image.height());
  for(auto point : points){
    point_tracker.AddToFrontier(point);
  }
}

bool GrowthImage::Iterate(){
  if(!palette.ColorsRemaining()){
    palette.SetPalette(palette_generator(rand_int, GetWidth() * GetHeight()));
  }

  auto loc = ChooseLocation();
  auto color = ChooseColor(loc);
  view(loc.i,loc.j) = {color.r, color.g, color.b};

  point_tracker.Fill(loc,
                     std::bind(std::ref(preference_generator),
                               rand_int, std::placeholders::_1, std::cref(point_tracker)));

  previous_loc = loc;


  return point_tracker.FrontierSize();
}

void GrowthImage::ExtendFrontier(Point loc){
  point_tracker.Fill(loc,
                     std::bind(std::ref(preference_generator),
                               rand_int, std::placeholders::_1, std::cref(point_tracker)));
}

void GrowthImage::IterateUntilDone(){
  int body_size = 0;
  while(point_tracker.FrontierSize()){
    if(body_size % 100000 == 0){
      std::cout << "\r                                                   \r"
                << "Body: " << body_size << "\tFrontier: " << point_tracker.FrontierSize()
                << "\tUnexplored: " << image.height()*image.width() - body_size - point_tracker.FrontierSize()
                << std::flush;
    }
    Iterate();
    body_size++;
  }
  std::cout << std::endl;
}

Point GrowthImage::ChooseLocation(){
  return location_generator(rand_int, point_tracker);
}

Point GrowthImage::ChooseFrontierLocation(){
  return point_tracker.FrontierAtIndex(
    rand_int(0, point_tracker.FrontierSize()));
}

Point GrowthImage::ChoosePreferredLocation(int n_check){
  assert(n_check>0);
  int best_index = 0;
  double best_preference = -DBL_MAX;
  for(int i=0; i<n_check; i++){
    int index = randint(rng, point_tracker.FrontierSize() );
    Point& p = point_tracker.FrontierAtIndex(index);

    if(std::isnan(p.preference)){
      p.preference = ChoosePreference(p);
    }

    if(p.preference > best_preference){
      best_preference = p.preference;
      best_index = index;
    }
  }

  return point_tracker.FrontierAtIndex(best_index);
}

Point GrowthImage::ChooseSequentialLocation(){
  Point output;
  if(previous_loc.i == -1){
    output = {0,0};
  } else if (previous_loc.i == image.width()-1){
    output = {0,previous_loc.j+1};
  } else {
    output = {previous_loc.i+1,previous_loc.j};
  }
  return output;
}

double GrowthImage::ChoosePreference(Point p){
  switch(preference_choice){
  case PreferenceChoice::Location:
    return ChoosePreferenceLocation(p);
  case PreferenceChoice::Perlin:
    return ChoosePreferencePerlin(p);
  default:
    assert(false);
  }
}

double GrowthImage::ChoosePreferenceLocation(Point p){
  if(goal_loc.i == -1 || point_tracker.IsFilled(goal_loc.i,goal_loc.j)){
    goal_loc = {randint(rng,image.width()),
                randint(rng,image.height())};
  }

  double di = p.i - goal_loc.i;
  double dj = p.j - goal_loc.j;
  return -(di*di + dj*dj);
}

double GrowthImage::ChoosePreferencePerlin(Point p){
  return perlin(p.i, p.j);
}

Color GrowthImage::ChooseColor(Point loc){
  switch(color_choice){
  case ColorChoice::Nearest:
    return ChooseNearestColor(loc);
  case ColorChoice::Sequential:
    return ChooseSequentialColor(loc);
  case ColorChoice::Perlin:
    return ChoosePerlinColor(loc);
  default:
    assert(false);
  }
}

Color GrowthImage::ChooseSequentialColor(Point){
  return palette.PopBack();
}

Color GrowthImage::ChooseNearestColor(Point loc){
  // Find the average surrounding color.
  double ave_r = 0;
  double ave_g = 0;
  double ave_b = 0;
  int count = 0;
  for(int di=-1; di<=1; di++){
    for(int dj=-1; dj<=1; dj++){
      Point p(loc.i+di,loc.j+dj);
      if(p.i>=0 && p.i<image.width() &&
         p.j>=0 && p.j<image.height() &&
         point_tracker.IsFilled(p.i,p.j)){
        ave_r += view(p.i,p.j)[0];
        ave_g += view(p.i,p.j)[1];
        ave_b += view(p.i,p.j)[2];
        count++;
      }
    }
  }

  if(count){
    // Neighbors exist, find the closest color.
    ave_r /= count;
    ave_g /= count;
    ave_b /= count;
    return palette.PopClosest({ave_r,ave_g,ave_b}, epsilon);

  } else {
    // No neighbors, so take a random color.
    return palette.PopRandom(rng);
  }
}

Color GrowthImage::ChoosePerlinColor(Point loc){
  auto result = perlin(loc.i,loc.j);
  double value = 255*(result+1)/2;
  return {value,value,value};
}
