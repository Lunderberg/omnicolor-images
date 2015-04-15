#ifndef _POINT_H_
#define _POINT_H_

struct Point{
  Point(int i=-1, int j=-1) : i(i), j(j), preference(0) {}
  int i,j;
  double preference;

  bool operator==(const Point& p) const {
    return p.i==i && p.j==j;
  }
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
