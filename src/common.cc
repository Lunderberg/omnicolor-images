#include "common.hh"

#include <iostream>
using std::cout;
using std::endl;

#include <cfloat>
#include <cmath>
#include <cassert>

// a is inclusive, b is exclusive.
// Range is [a,b-1]
long randint(long a, long b){
	static std::mt19937 rng(time(0));
	return std::uniform_int_distribution<long>(a,b-1)(rng);
}

// Random integer [0,a-1]
long randint(long a){
	return randint(0,a);
}
