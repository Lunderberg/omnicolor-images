#include "PointTracker.hh"

PointTracker::PointTracker(int width, int height)
  : width(width), height(height) {
  filled.assign(width*height, false);
}

void PointTracker::Clear(){
  filled.assign(width*height, false);
  frontier_map.clear();
  frontier_vector.clear();
}

int PointTracker::FrontierSize() const {
  return frontier_vector.size();
}

bool PointTracker::IsFilled(Point p) const {
  return filled[p.j*width + p.i];
}

bool PointTracker::IsFilled(int i, int j) const {
  return filled[j*width + i];
}

void PointTracker::AddToFrontier(Point p){
  if(p.i>=0 && p.i<width &&
     p.j>=0 && p.j<height &&
     !IsInFrontier(p) &&
     !IsFilled(p)){
    frontier_map[p] = frontier_vector.size();
    frontier_vector.push_back(p);
  }
}

bool PointTracker::IsInFrontier(Point p) const {
  return frontier_map.count(p);
}

Point& PointTracker::FrontierAtIndex(int i){
  return frontier_vector[i];
}

Point PointTracker::FrontierAtIndex(int i) const {
  return frontier_vector[i];
}

void PointTracker::RemoveFromFrontier(Point p){
  if(IsInFrontier(p)){
    int index = frontier_map.at(p);
    frontier_map[frontier_vector.back()] = index;
    std::swap(frontier_vector[index], frontier_vector.back());
    frontier_vector.pop_back();
    frontier_map.erase(p);
  }
}
