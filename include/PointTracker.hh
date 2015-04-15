#ifndef _POINTTRACKER_H_
#define _POINTTRACKER_H_

#include <cmath>
#include <unordered_map>
#include <vector>

#include "Point.hh"

class PointTracker{
public:
  PointTracker(int width, int height);

  void Clear();

  int FrontierSize() const;
  bool IsFilled(Point p) const;
  bool IsFilled(int i, int j) const;
  bool IsInFrontier(Point p) const;
  Point FrontierAtIndex(int i) const;

  int GetWidth() const { return width; }
  int GetHeight() const { return height; }

  void AddToFrontier(Point p);
  Point& FrontierAtIndex(int i);

  template<typename Callable>
  void Fill(Point p, Callable func){
    filled[p.j*width + p.i] = true;
    RemoveFromFrontier(p);

    for(int di=-1; di<=1; di++){
      for(int dj=-1; dj<=1; dj++){
        Point loc(p.i+di, p.j+dj);
        loc.preference = func(loc);
        AddToFrontier(loc);
      }
    }
  }

private:
  void RemoveFromFrontier(Point p);

  int width;
  int height;
  std::vector<bool> filled;

  std::unordered_map<Point,int> frontier_map;
  std::vector<Point> frontier_vector;
};

#endif /* _POINTTRACKER_H_ */
