#ifndef _POINT_H_
#define _POINT_H_

#include <iostream>

struct Point{
  Point(int i=-1, int j=-1) : i(i), j(j), preference(0) { }
  int i,j;
  double preference;

  bool operator==(const Point& p) const {
    return p.i==i && p.j==j;
  }

  int GetI() const { return i; }
  int GetJ() const { return j; }
  double GetPreference() const { return preference; }
};

namespace std{
  template<>
  struct hash<Point>{
    size_t operator()(const Point& p) const{
      return 1000*p.i + p.j;
    }
  };
}

#endif /* _POINT_H_ */
