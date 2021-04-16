#include "SavePNG.hh"

#include <boost/version.hpp>

//In a separate file because this boost header file takes forever to compile.
#if (BOOST_VERSION < 106800)
  #include <boost/gil/gil_all.hpp>
  #include <boost/gil/extension/io/png_dynamic_io.hpp>
#else
  #include <boost/gil.hpp>
  #include <boost/gil/extension/io/png/old.hpp>
#endif

void SavePNG(const std::vector<Color> pixels, int width, int height,
             const char *filepath) {
  boost::gil::rgb8_image_t image(width, height);
  auto view = image._view;

  // No significant performance difference when using memcpy instead.
  // memcpy(&view[0], &pixels[0], sizeof(Color)*pixels.size());

  for(int j=0; j<height; j++) {
    for(int i=0; i<width; i++) {
      auto color = pixels[i + j*width];
      view(i,j) = {color.r, color.g, color.b};
    }
  }

  boost::gil::png_write_view(filepath,boost::gil::const_view(image));
}

void SavePNG(const std::vector<Color> pixels, int width, int height,
             const std::string& filepath) {
  SavePNG(pixels, width, height, filepath.c_str());
}
