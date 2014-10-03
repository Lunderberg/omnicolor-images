#include <iostream>
using std::cout;
using std::endl;

#include <boost/gil/gil_all.hpp>

#include "save_image.hh"
#include "common.hh"

int main(){
	// save_image({256,128},allowed_colors_15bit(),ordered_pixels,"ordered_pixels.png");
	// save_image({256,128},allowed_colors_15bit(),random_growth,"random_growth.png");
	//save_image({256,128},allowed_colors_15bit(),closest_color,"closest_color.png");


	save_image({256,128},allowed_colors(1<<15),ordered_pixels,"ordered_pixels.png");
	save_image({256,128},allowed_colors(1<<15),random_growth,"random_growth.png");
	save_image({256,128},allowed_colors(1<<15),closest_color,"closest_color.png");


	save_image({256,256},allowed_colors(1<<16),ordered_pixels,"ordered_pixels_16.png");
	save_image({256,256},allowed_colors(1<<16),random_growth,"random_growth_16.png");
	save_image({256,256},allowed_colors(1<<16),closest_color,"closest_color_16.png");

	save_image({1920,1080},allowed_colors(1920*1080),closest_color,"closest_color_bg.png");
}
