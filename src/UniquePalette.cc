#include "UniquePalette.hh"

#include <cassert>
#include <cmath>
#include <cfloat>
#include <algorithm>
#include <stdexcept>

#include "common.hh"

double euclidean_distance(Color c1, Color c2){
	double dx = c1.r - c2.r;
	double dy = c1.g - c2.g;
	double dz = c1.b - c2.b;
	return std::sqrt(dx*dx + dy*dy + dz*dz);
}

UniquePalette::UniquePalette() :
	iterations_since_purge(0), distance(euclidean_distance) { }

void UniquePalette::GenerateUniformPalette(int n_colors){
	assert(n_colors > 0);
	assert(n_colors <= (1<<24));

	double dim_size = std::pow(n_colors,1.0/3.0);

	colors.clear();
	for(int i=0; i < n_colors; i++){
		double val = i;
		val /= dim_size;
		double r = std::fmod(val,1);
		val = int(val);
		val /= dim_size;
		double g = std::fmod(val,1);
		val = int(val);
		val /= dim_size;
		double b = val;

		ColorEntry col(Color(r*255,g*255,b*255));
		col.magnitude = distance(col.color,{0,0,0});
		colors.push_back(col);
	}

	//std::shuffle(colors.begin(),colors.end(),rng);
	std::stable_sort(colors.begin(),colors.end());
}

// A plain linear search goes too slowly,
//  and a binary search is impossible since 3-d points are not well-ordered.
// Here, we keep the pixels ordered by magnitude.
// We search in 3-d shells, starting at the magnitude of the given point.
// The shell width places a lower bound on the distance that can be found.
Color UniquePalette::PopClosest(Color col){
	PurgeUsedColors();

	// Binary search, find the starting point.
	double mag = distance(col,{0,0,0});
	int start_index;
	{
		int low=0;
		int high = colors.size()-1;
		while(high-low > 1){
			int mid = (low+high)/2;
			if(colors[mid].magnitude < mag){
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
				 high >= colors.size()){
				index_checking = low;
				low_thickness = mag - colors[low].magnitude;
				low--;
			} else if ((high<colors.size() && high_thickness<low_thickness) ||
								 (low<0)) {
				index_checking = high;
				high_thickness = colors[high].magnitude - mag;
				high++;
			} else {
				index_checking = -1;
				break;
			}
			// Break out if the color has not been used.
			if(!colors[index_checking].used){
				break;
			}
		}

		// Break out if the index is out of range.
		if(index_checking<0 || index_checking>=colors.size()){
			break;
		}

		// Check if it is closest
		auto col_checking = colors[index_checking].color;
		double dist = distance(col,col_checking);
		if(dist<best_distance){
			best_distance = dist;
			best_index = index_checking;
		}

		// If a better point has been found than the thickness of the skins,
		//  we cannot find a better point.
		if((best_distance<low_thickness || low<0) &&
			 (best_distance<high_thickness || high>=colors.size())){
			break;
		}
	}

	auto output = colors[best_index].color;
	colors[best_index].used = true;


	return output;
}

Color UniquePalette::PopBack(){
	PurgeUsedColors();

	for(unsigned int i=colors.size()-1; i>=0; i++){
		if(!colors[i].used){
			colors[i].used = true;
			return colors[i].color;
		}
	}
	throw std::runtime_error("No unused colors remaining");
}

Color UniquePalette::PopRandom(std::mt19937& rng){
	while(colors.size()){
		PurgeUsedColors();
		long index = randint(rng,colors.size());
		if(!colors[index].used){
			colors[index].used = true;
			return colors[index].color;
		}
	}

	throw std::runtime_error("No unused colors remaining");
}

void UniquePalette::PurgeUsedColors(bool force){
	iterations_since_purge++;
	if(iterations_since_purge > 10000 || force){
		std::vector<ColorEntry> new_list;
		for(const auto& col : colors){
			if(!col.used){
				new_list.push_back(col);
			}
		}
		colors = std::move(new_list);

		iterations_since_purge = 0;
	}
}
