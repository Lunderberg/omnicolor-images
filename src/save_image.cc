#include "save_image.hh"

#include <boost/gil/extension/io/png_dynamic_io.hpp>

namespace gil = boost::gil;

void save_image(boost::gil::rgb8_image_t image,
								std::vector<boost::gil::rgb8_pixel_t> palette,
								std::function<void(boost::gil::rgb8_image_t::view_t,
																	 std::vector<boost::gil::rgb8_pixel_t>)> generator,
								const char* name){
	generator(gil::view(image),palette);
	gil::png_write_view(name,gil::const_view(image));
}
