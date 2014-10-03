#include "common.hh"

#include <iostream>
using std::cout;
using std::endl;

#include <cfloat>
#include <cmath>
#include <cassert>

struct point{
	point(int i, int j) : i(i), j(j) {}
	int i,j;

	bool operator==(const point& other) const{
		return other.i==i && other.j==j;
	}
};

// a is inclusive, b is exclusive.
// Range is [a,b-1]
long randint(long a, long b){
	static std::mt19937 rng(time(0));
	return std::uniform_int_distribution<long>(a,b-1)(rng);
}

// Random integer [0,a-1]
long randint(long a){
	static std::mt19937 rng(time(0));
	return std::uniform_int_distribution<long>(0,a-1)(rng);
}

std::vector<boost::gil::rgb8_pixel_t> allowed_colors_15bit(){
	std::vector<boost::gil::rgb8_pixel_t> output;
	for(int i=0; i < (1<<15); i++){
		output.push_back({i<<3 & 0xF8, i>>2 & 0xF8, i>>7 & 0xF8});
	}
	return output;
}

std::vector<boost::gil::rgb8_pixel_t> allowed_colors(int colors){
	assert(colors > 0);
	assert(colors <= (1<<24));

	double dim_size = std::pow(colors,1.0/3.0);

	std::vector<boost::gil::rgb8_pixel_t> output;
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
		output.push_back({r*255,g*255,b*255});
	}
	return output;
}

void ordered_pixels(boost::gil::rgb8_image_t::view_t img,
										std::vector<boost::gil::rgb8_pixel_t> palette){
	int pindex = 0;
	for(int i=0; i<img.width(); i++){
		for(int j=0; j<img.height(); j++){
			img(i,j) = palette[pindex];
			pindex++;
		}
	}
}

void random_growth(boost::gil::rgb8_image_t::view_t img,
									 std::vector<boost::gil::rgb8_pixel_t> palette){

	std::vector<point> frontier;
	bool filled[img.width()][img.height()];
	for(int i=0; i<img.width(); i++){
		for(int j=0; j<img.height(); j++){
			filled[i][j] = false;
		}
	}

	// Randomly choose an initial point.
	frontier.push_back({randint(img.width()),
				              randint(img.height())});

	while(frontier.size()){
		// Pick a location and color
		auto loc = poprandom(frontier);
		auto color = palette.back();
		palette.pop_back();
		img(loc.i,loc.j) = color;
		filled[loc.i][loc.j] = true;

		// Add adjacent squares to the frontier
		for(int di=-1; di<=1; di++){
			for(int dj=-1; dj<=1; dj++){
				point p(loc.i+di,loc.j+dj);
				if(p.i>=0 && p.i<img.width() &&
					 p.j>=0 && p.j<img.height() &&
					 !is_in(frontier,p) &&
					 !filled[p.i][p.j]){
					frontier.push_back(p);
				}
			}
		}
	}
}



void closest_color(boost::gil::rgb8_image_t::view_t img,
									 std::vector<boost::gil::rgb8_pixel_t> palette){

	std::vector<point> frontier;
	bool filled[img.width()][img.height()];
	for(int i=0; i<img.width(); i++){
		for(int j=0; j<img.height(); j++){
			filled[i][j] = false;
		}
	}

	// Randomly choose an initial point.
	frontier.push_back({randint(img.width()),
				              randint(img.height())});

	int body_count = 0;
	while(frontier.size()){
		if(body_count%1000==0){
			cout << "Body: " << body_count
					 << "\tFrontier: " << frontier.size()
					 << "\tUnexplored: " << img.width()*img.height() - body_count - frontier.size()
					 << endl;
		}
		body_count++;

		auto loc = poprandom(frontier);

		boost::gil::rgb8_pixel_t color;

		double ave_r = 0;
		double ave_g = 0;
		double ave_b = 0;
		int count = 0;
		for(int di=-1; di<=1; di++){
			for(int dj=-1; dj<=1; dj++){
				point p(loc.i+di,loc.j+dj);
				if(p.i>=0 && p.i<img.width() &&
					 p.j>=0 && p.j<img.height() &&
					 filled[p.i][p.j]){
					ave_r += img(p.i,p.j)[0];
					ave_g += img(p.i,p.j)[1];
					ave_b += img(p.i,p.j)[2];
					count++;
				}
			}
		}
		if(count){
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
			color = popanywhere(palette,best_i);

			// cout << "Ave: (" << ave_r << ", " << ave_g << ", " << ave_b
			// 		 << ")\tClosest: (" << color[0] << ", " << color[1] << ", " << color[2]
			// 		 << ")" << endl;

		} else {
			// No neighbors, so take a random color.
			color = poprandom(palette);
		}

		img(loc.i,loc.j) = color;
		filled[loc.i][loc.j] = true;

		// Add adjacent squares to the frontier
		for(int di=-1; di<=1; di++){
			for(int dj=-1; dj<=1; dj++){
				point p(loc.i+di,loc.j+dj);
				if(p.i>=0 && p.i<img.width() &&
					 p.j>=0 && p.j<img.height() &&
					 !is_in(frontier,p) &&
					 !filled[p.i][p.j]){
					frontier.push_back(p);
				}
			}
		}
	}
}
