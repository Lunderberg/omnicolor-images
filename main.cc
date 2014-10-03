#include <iostream>
using std::cout;
using std::endl;

#include <boost/gil/gil_all.hpp>

// #include "common.hh"

#include "GrowthImage.hh"

int main(){
	GrowthImage g(512,512,512*512,ColorChoice::Sequential);
	g.IterateUntilDone();
	g.Save("test.png");
}
