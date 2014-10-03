#ifndef _COMMON_H_
#define _COMMON_H_

#include <vector>
#include <algorithm>
#include <random>
#include <ctime>

#include <boost/gil/gil_all.hpp>

std::vector<boost::gil::rgb8_pixel_t> allowed_colors_15bit();
std::vector<boost::gil::rgb8_pixel_t> allowed_colors(int colors);

void ordered_pixels(boost::gil::rgb8_image_t::view_t img,
										std::vector<boost::gil::rgb8_pixel_t> palette);

void random_growth(boost::gil::rgb8_image_t::view_t img,
									 std::vector<boost::gil::rgb8_pixel_t> palette);

void closest_color(boost::gil::rgb8_image_t::view_t img,
									 std::vector<boost::gil::rgb8_pixel_t> palette);

// a is inclusive, b is exclusive.
// Range is [a,b-1]
long randint(long a, long b);

// Random integer [0,a-1]
long randint(long a);

template <typename T, typename U>
bool is_in(T container, U element){
	return std::find(container.begin(), container.end(), element) != container.end();
}

template <typename T>
T popanywhere(std::vector<T>& vec, long index){
	std::swap(vec[index],vec.back());
	T output = vec.back();
	vec.pop_back();
	return output;
}

template <typename T>
T poprandom(std::vector<T>& vec){
	auto index = randint(vec.size());
	return popanywhere(vec,index);
}


#endif /* _COMMON_H_ */
