#include "GrowthImage.hh"

#include "common.hh"

#include <cassert>
#include <cfloat>

GrowthImage::GrowthImage(int width, int height, int colors)
	: image(width,height), view(boost::gil::view(image)), previous_loc(-1,-1){
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
	frontier.push_back({randint(image.width()),
				              randint(image.height())});
}

void GrowthImage::Iterate(){
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
}

void GrowthImage::IterateUntilDone(){
	while(frontier.size()){
		Iterate();
	}
}

Point GrowthImage::ChooseLocation(){
	return poprandom(frontier);
}

boost::gil::rgb8_pixel_t GrowthImage::ChooseColor(Point loc){
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

		//Pick the closest unused color
		long best_i = 0;
		double closest = DBL_MAX;
		for(long i=0; i<palette.size(); i++){
			double dist2 = ((palette[i][0]-ave_r)*(palette[i][0]-ave_r) +
											(palette[i][1]-ave_g)*(palette[i][1]-ave_g) +
											(palette[i][2]-ave_b)*(palette[i][2]-ave_b));
			if(dist2<closest){
				best_i = i;
				closest = dist2;
			}
		}
		return popanywhere(palette,best_i);

	} else {
		// No neighbors, so take a random color.
		return poprandom(palette);
	}
}
