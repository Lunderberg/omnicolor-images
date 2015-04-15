#ifndef _COLOR_H_
#define _COLOR_H_

#include <cassert>

struct Color{
  Color(unsigned char r, unsigned char g, unsigned char b)
    : r(r), g(g), b(b) { }
  unsigned char r,g,b;

  enum {dimensions = 3};
  int get(int n){
    switch(n){
    case 0:
      return r;
    case 1:
      return g;
    case 2:
      return b;
    default:
      assert(false);
    }
  }
};

#endif /* _COLOR_H_ */
