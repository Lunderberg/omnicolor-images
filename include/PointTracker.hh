#ifndef _POINTTRACKER_H_
#define _POINTTRACKER_H_

#include "Point.hh"

class PointTracker{
public:
  PointTracker(int width, int height)
    : width(width), height(height) {
    filled.assign(width*height, false);
  }

  void Clear(){
    filled.assign(width*height, false);
    frontier_set.clear();
    frontier_vector.clear();
  }

  int FrontierSize(){
    return frontier_vector.size();
  }

  void Fill(Point p){
    filled[p.j*width + p.i] = true;
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
      frontier_set.insert(p);
      frontier_vector.push_back(p);
    }
  }

  bool IsInFrontier(Point p){
    return frontier_set.count(p);
  }

  Point& FrontierAtIndex(int i){
    return frontier_vector[i];
  }

  Point PopFrontierAtIndex(int i){
    std::swap(frontier_vector[i], frontier_vector.back());
    Point output = frontier_vector.back();
    frontier_vector.pop_back();
    frontier_set.erase(output);
    return output;
  }


private:
  int width;
  int height;
  std::vector<bool> filled;

  std::unordered_set<Point> frontier_set;
  std::vector<Point> frontier_vector;
};

#endif /* _POINTTRACKER_H_ */
