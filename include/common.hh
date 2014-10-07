#ifndef _COMMON_H_
#define _COMMON_H_

#include <vector>
#include <algorithm>
#include <random>
#include <ctime>

// a is inclusive, b is exclusive.
// Range is [a,b-1]
template<typename Generator>
long randint(Generator& rng, long a, long b){
	return std::uniform_int_distribution<long>(a,b-1)(rng);
}

// Random integer [0,a-1]
template<typename Generator>
long randint(Generator& rng, long a){
	return randint(rng,0,a);
}

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

template <typename Generator,typename T>
T poprandom(Generator& rng,std::vector<T>& vec){
	auto index = randint(rng,vec.size());
	return popanywhere(vec,index);
}


#endif /* _COMMON_H_ */
