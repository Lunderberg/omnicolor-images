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
    target_color_generator(generate_average_color),
    point_tracker(width, height),
    epsilon(0),
    image(width,height), view(boost::gil::view(image)),
    rng(seed ? seed : time(0)) {

  rand_int = [this](int a, int b){
    return std::uniform_int_distribution<int>(a,b-1)(rng);
  };
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
  auto points = initial_location_generator(rand_int, image.width(), image.height());
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
  view(loc.i,loc.j) = {color.r, color.g, color.b};

  point_tracker.Fill(loc,
                     std::bind(std::ref(preference_generator),
                               rand_int, std::placeholders::_1, std::cref(point_tracker)));

  return point_tracker.FrontierSize();
}

void GrowthImage::IterateUntilDone(){
  int body_size = 0;
  while(Iterate()){
    if(body_size % 100000 == 0){
      std::cout << "\r                                                   \r"
                << "Body: " << body_size << "\tFrontier: " << point_tracker.FrontierSize()
                << "\tUnexplored: " << image.height()*image.width() - body_size - point_tracker.FrontierSize()
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
      if(p.i>=0 && p.i<image.width() &&
         p.j>=0 && p.j<image.height() &&
         point_tracker.IsFilled(p.i,p.j)){
        neighbors.push_back({
            view(p.i,p.j)[0],
              view(p.i,p.j)[1],
              view(p.i,p.j)[2]});
      }
    }
  }

  Color target = target_color_generator(rand_int, std::move(neighbors), loc);
  return palette.PopClosest(target, epsilon);
}
