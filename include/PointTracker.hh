#ifndef _POINTTRACKER_H_
#define _POINTTRACKER_H_

#include <unordered_map>
#include <vector>

#include "Point.hh"

class PointTracker{
public:
  PointTracker(int width, int height)
    : width(width), height(height) {
    filled.assign(width*height, false);
  }

  void Clear(){
    filled.assign(width*height, false);
    frontier_map.clear();
    frontier_vector.clear();
  }

  int FrontierSize(){
    return frontier_vector.size();
  }

  void Fill(Point p){
    filled[p.j*width + p.i] = true;
    RemoveFromFrontier(p);

    for(int di=-1; di<=1; di++){
      for(int dj=-1; dj<=1; dj++){
        Point loc(p.i+di, p.j+dj);
        loc.preference = std::sqrt(-1);
        AddToFrontier(loc);
      }
    }
  }

  bool IsFilled(Point p){
    return filled[p.j*width + p.i];
  }

  bool IsFilled(int i, int j){
    return filled[j*width + i];
  }

  void AddToFrontier(Point p){
    if(p.i>=0 && p.i<width &&
       p.j>=0 && p.j<height &&
       !IsInFrontier(p) &&
       !IsFilled(p)){
      frontier_map[p] = frontier_vector.size();
      frontier_vector.push_back(p);
    }
  }

  bool IsInFrontier(Point p){
    return frontier_map.count(p);
  }

  Point& FrontierAtIndex(int i){
    return frontier_vector[i];
  }

private:
  void RemoveFromFrontier(Point p){
    int index = frontier_map.at(p);
    frontier_map[frontier_vector.back()] = index;
    std::swap(frontier_vector[index], frontier_vector.back());
    frontier_vector.pop_back();
    frontier_map.erase(p);
  }

  int width;
  int height;
  std::vector<bool> filled;

  std::unordered_map<Point,int> frontier_map;
  std::vector<Point> frontier_vector;
};

#endif /* _POINTTRACKER_H_ */
