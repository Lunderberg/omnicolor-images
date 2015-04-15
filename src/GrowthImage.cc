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
