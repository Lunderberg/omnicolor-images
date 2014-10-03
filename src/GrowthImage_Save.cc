#include "GrowthImage.hh"

//In a separate file because this boost header file takes forever to compile.
#include <boost/gil/extension/io/png_dynamic_io.hpp>

void GrowthImage::Save(const char* filepath){
	boost::gil::png_write_view(filepath,boost::gil::const_view(image));
}
