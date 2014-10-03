#ifndef _SAVE_IMAGE_H_
#define _SAVE_IMAGE_H_

#include <functional>

#include <boost/gil/gil_all.hpp>

void save_image(boost::gil::rgb8_image_t image,
								std::vector<boost::gil::rgb8_pixel_t> palette,
								std::function<void(boost::gil::rgb8_image_t::view_t,
																	 std::vector<boost::gil::rgb8_pixel_t>)> generator,
								const char* name);

#endif /* _SAVE_IMAGE_H_ */
