#include "GrowthImage.hh"

#include <boost/version.hpp>

//In a separate file because this boost header file takes forever to compile.
#if (BOOST_VERSION < 106800)
  #include <boost/gil/extension/io/png_dynamic_io.hpp>
#else
  #include <boost/gil/extension/io/png/old.hpp>
#endif

void GrowthImage::Save(const char* filepath){
  boost::gil::png_write_view(filepath,boost::gil::const_view(image));
}

void GrowthImage::Save(const std::string& filepath){
  Save(filepath.c_str());
}
