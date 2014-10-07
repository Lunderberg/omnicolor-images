#include "GrowthImage.hh"

#include "common.hh"

#include <iostream>
using std::cout;
using std::endl;

#include <cassert>
#include <cfloat>
#include <algorithm>
#include <ctime>

GrowthImage::GrowthImage(int width, int height, int colors, ColorChoice color_choice, int seed)
	: image(width,height), view(boost::gil::view(image)), previous_loc(-1,-1),
		color_choice(color_choice), iterations_since_purge(0){
	rng = std::mt19937((seed>0) ? seed : std::time(0));
	Reset();
	GenerateUniformPalette(colors);
}

void GrowthImage::GenerateUniformPalette(int colors){
	assert(colors > 0);
	assert(colors <= (1<<24));

	double dim_size = std::pow(colors,1.0/3.0);

	palette.clear();
	for(int i=0; i < colors; i++){
		double val = i;
		val /= dim_size;
		double r = std::fmod(val,1);
		val = int(val);
		val /= dim_size;
		double g = std::fmod(val,1);
		val = int(val);
		val /= dim_size;
		double b = val;
		palette.push_back({r*255,g*255,b*255});
	}

	std::shuffle(palette.begin(),palette.end(),rng);
	std::stable_sort(palette.begin(),palette.end());
}

void GrowthImage::Reset(){
	filled.clear();
	filled.resize(image.width());
	for(int i=0; i<image.width(); i++){
		filled[i].resize(image.height());
		for(int j=0; j<image.height(); j++){
			filled[i][j] = false;
		}
	}

	frontier.clear();

	FirstIteration();
}

void GrowthImage::FirstIteration(){
	frontier.push_back({randint(rng,image.width()),
				              randint(rng,image.height())});
}

bool GrowthImage::Iterate(){
	auto loc = ChooseLocation();
	auto color = ChooseColor(loc);
	view(loc.i,loc.j) = color;

	// Extend the frontier
	filled[loc.i][loc.j] = true;
	for(int di=-1; di<=1; di++){
		for(int dj=-1; dj<=1; dj++){
			Point p(loc.i+di, loc.j+dj);
			if(p.i>=0 && p.i<image.width() &&
				 p.j>=0 && p.j<image.height() &&
				 !is_in(frontier,p) &&
				 !filled[p.i][p.j]){
				frontier.push_back(p);
			}
		}
	}

	previous_loc = loc;

	return frontier.size();
}

void GrowthImage::IterateUntilDone(){
	int body_size = 0;
	while(frontier.size()){
		if(body_size%1000==0){
			cout << "\r                                                   \r"
					 << "Body: " << body_size << "\tFrontier: " << frontier.size()
					 << "\tUnexplored: " << image.height()*image.width() - body_size - frontier.size()
					 << std::flush;
		}
		Iterate();
		body_size++;
	}
	cout << endl;
}

Point GrowthImage::ChooseLocation(){
	return ChooseSnakingLocation();
}

Point GrowthImage::ChooseFrontierLocation(){
	return poprandom(rng,frontier);
}

Point GrowthImage::ChooseSnakingLocation(){
	Point new_loc;
	switch(randint(rng,4)){
	case 0:
		new_loc = Point(previous_loc.i-1, previous_loc.j);
		break;
	case 1:
		new_loc = Point(previous_loc.i+1, previous_loc.j);
		break;
	case 2:
		new_loc = Point(previous_loc.i, previous_loc.j-1);
		break;
	case 3:
		new_loc = Point(previous_loc.i, previous_loc.j+1);
		break;
	}
	if(new_loc.i>=0 && new_loc.i<image.width() &&
		 new_loc.j>=0 && new_loc.j<image.height() &&
		 !filled[new_loc.i][new_loc.j]){
		frontier.erase(std::remove_if(frontier.begin(),frontier.end(),
																	[&new_loc](const Point& o){return o==new_loc;}),
									 frontier.end());
		return new_loc;
	} else {
		return poprandom(rng,frontier);
	}
}

boost::gil::rgb8_pixel_t GrowthImage::ChooseColor(Point loc){
	switch(color_choice){
	case ColorChoice::Nearest:
		return ChooseNearestColor(loc);
	case ColorChoice::Ordered:
		return ChooseOrderedColor(loc);
	case ColorChoice::Sequential:
		return ChooseSequentialColor(loc);
	}
}

boost::gil::rgb8_pixel_t GrowthImage::ChooseOrderedColor(Point loc){
	return palette[loc.j*image.width() + loc.i].gil_color();
}

boost::gil::rgb8_pixel_t GrowthImage::ChooseSequentialColor(Point loc){
	auto output = palette.back();
	palette.pop_back();
	return output.gil_color();
}

boost::gil::rgb8_pixel_t GrowthImage::ChooseNearestColor(Point loc){
	// Find the average surrounding color.
	double ave_r = 0;
	double ave_g = 0;
	double ave_b = 0;
	int count = 0;
	for(int di=-1; di<=1; di++){
		for(int dj=-1; dj<=1; dj++){
			Point p(loc.i+di,loc.j+dj);
			if(p.i>=0 && p.i<image.width() &&
				 p.j>=0 && p.j<image.height() &&
				 filled[p.i][p.j]){
				ave_r += view(p.i,p.j)[0];
				ave_g += view(p.i,p.j)[1];
				ave_b += view(p.i,p.j)[2];
				count++;
			}
		}
	}

	if(count){
		// Neighbors exist, find the closest color.
		ave_r /= count;
		ave_g /= count;
		ave_b /= count;
		return PopClosestColor(ave_r,ave_g,ave_b);

	} else {
		// No neighbors, so take a random color.
		return poprandom(rng,palette).gil_color();
	}
}

// A plain linear search goes too slowly,
//  and a binary search is impossible since 3-d points are not well-ordered.
// Here, we keep the pixels ordered by magnitude.
// We search in 3-d shells, starting at the magnitude of the given point.
// The shell width places a lower bound on the distance that can be found.
boost::gil::rgb8_pixel_t GrowthImage::PopClosestColor(double r, double g, double b){
	// Binary search, find the starting point.
	double mag = std::sqrt(r*r + g*g + b*b);
	int start_index;
	{
		int low=0;
		int high = palette.size()-1;
		while(high-low > 1){
			int mid = (low+high)/2;
			if(palette[mid].magnitude < mag){
				low = mid;
			} else {
				high = mid;
			}
		}
		start_index = low;
	}

	// Iterate out from the starting point, in both directions
	double low_thickness = 0;
	double high_thickness = 0;
	int low = start_index;
	int high = start_index;
	int best_index = -1;
	double best_distance = DBL_MAX;
	while(true){
		int index_checking;
		// Whichever skin is thinner, move in that direction.
		while(true){
			if((low_thickness<=high_thickness && low>=0) ||
				 high >= palette.size()){
				index_checking = low;
				low_thickness = mag - palette[low].magnitude;
				low--;
			} else if ((high<palette.size() && high_thickness<low_thickness) ||
								 (low<0)) {
				index_checking = high;
				high_thickness = palette[high].magnitude - mag;
				high++;
			} else {
				index_checking = -1;
				break;
			}
			// Break out if the color has not been used.
			if(!palette[index_checking].used){
				break;
			}
		}

		// Break out if the index is out of range.
		if(index_checking<0 || index_checking>=palette.size()){
			break;
		}

		// Check if it is closest
		auto col = palette[index_checking];
		double dist = std::sqrt((col.r-r)*(col.r-r) + (col.g-g)*(col.g-g) + (col.b-b)*(col.b-b));
		if(dist<best_distance){
			best_distance = dist;
			best_index = index_checking;
		}

		// If a better point has been found than the thickness of the skins,
		//  we cannot find a better point.
		if((best_distance<low_thickness || low<0) &&
			 (best_distance<high_thickness || high>=palette.size())){
			break;
		}
	}

	auto output = palette[best_index].gil_color();
	palette[best_index].used = true;

	iterations_since_purge++;
	if(iterations_since_purge>10000){
		PurgeUsedColors();
		iterations_since_purge = 0;
	}

	return output;
}

void GrowthImage::PurgeUsedColors(){
	std::vector<Color> new_list;
	for(const auto& col : palette){
		if(!col.used){
			new_list.push_back(col);
		}
	}
	palette = std::move(new_list);
}
