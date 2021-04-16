#ifndef _SAVEPNG_H_
#define _SAVEPNG_H_

#include <string>
#include <vector>

#include "Color.hh"

void SavePNG(const std::vector<Color> pixels, int width, int height,
             const char *filepath);

void SavePNG(const std::vector<Color> pixels, int width, int height,
             const std::string &filepath);

#endif /* _SAVEPNG_H_ */
