#ifndef _COMMON_H_
#define _COMMON_H_

#include <vector>
#include <algorithm>
#include <random>
#include <ctime>

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
