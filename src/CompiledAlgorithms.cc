#include "CompiledAlgorithms.hh"

#include <cassert>
#include <cfloat>
#include <cmath>

#include <iostream>

std::vector<Color> generate_uniform_palette(RandomInt, int n_colors){
  assert(n_colors > 0);
  assert(n_colors < (1<<24));

  double dim_size = std::pow(n_colors,1.0/3.0);

  std::vector<Color> colors;
  colors.reserve(n_colors);
  for(int i=0; i < n_colors; i++){
    double val = i;
    val /= dim_size;
    double r = std::fmod(val,1);
    val = int(val);
    val /= dim_size;
    double g = std::fmod(val,1);
    val = int(val);
    val /= dim_size;
    double b = val;

    colors.push_back({r*255,g*255,b*255});
  }

  return colors;
}

std::vector<Point> generate_random_start(RandomInt rand, int width, int height){
  std::vector<Point> output;
  output.push_back({rand(0,width), rand(0,height)});
  return output;
}

Point generate_frontier_location(RandomInt rand, const PointTracker& point_tracker){
  return point_tracker.FrontierAtIndex(
    rand(0, point_tracker.FrontierSize()));
}

Point generate_sequential_location::operator()(RandomInt, const PointTracker&){
  i++;
  if(i==width){
    i = 0;
    j++;
  }
  return {i,j};
}

Point generate_preferred_location::operator()(RandomInt rand, const PointTracker& point_tracker){
  int best_index = 0;
  double best_preference = -DBL_MAX;
  for(int i=0; i<n; i++){
    int index = rand(0, point_tracker.FrontierSize() );
    auto p = point_tracker.FrontierAtIndex(index);

    if(p.preference > best_preference){
      best_preference = p.preference;
      best_index = index;
    }
  }

  return point_tracker.FrontierAtIndex(best_index);
}

double generate_null_preference(RandomInt, Point, const PointTracker&){
  return 0;
}

double generate_location_preference::operator()(RandomInt rand, Point p, const PointTracker& point_tracker){
  if(goal_loc == -1 || point_tracker.IsFilled(goal_loc.i, goal_loc.j)){
    goal_loc = {rand(0, point_tracker.GetWidth()),
                rand(0, point_tracker.GetHeight())};
  }

  double di = p.i - goal_loc.i;
  double dj = p.j - goal_loc.j;
  return -(di*di + dj*dj);
}

Color generate_average_color(RandomInt rand, std::vector<Color> neighbors, Point){
  if(neighbors.size()){
    Color output(0,0,0);
    for(auto col : neighbors){
      output.r += col.r/neighbors.size();
      output.g += col.g/neighbors.size();
      output.b += col.b/neighbors.size();
    }
    return output;
  } else {
    return {rand(0,255), rand(0,255), rand(0,255)};
  }
}
