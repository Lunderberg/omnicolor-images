#include <iostream>
using std::cout;
using std::flush;
using std::endl;
#include <cstdio>
#include <cstdlib>

#include <boost/gil/gil_all.hpp>

#include "GrowthImage.hh"

void MakeVideo(){
	int width = 1920;
	int height = 1080;
	int iterations_per_pic = 1000;

	system("rm -rf temp && mkdir temp");

	GrowthImage g(width,height,width*height,ColorChoice::Nearest,5);
	int picnum = 0;
	for(int i=0; g.Iterate(); i++){
		if(i%iterations_per_pic==0){
			char buf[100];
			snprintf(buf,sizeof(buf),"temp/growth_%d.png",picnum++);
			g.Save(buf);
			cout << "\rIteration: (" << i << "/" << width*height << ")" << flush;
		}
	}
	cout << endl;

	for(int i=0; i<24; i++){
		char buf[100];
		snprintf(buf,sizeof(buf),"temp/growth_%d.png",picnum++);
		g.Save(buf);
	}

	char buf[250];
	snprintf(buf,sizeof(buf),
					 "avconv -f image2 -framerate 12 -i \"temp/growth_%%d.png\" -b 1500k -s %dx%d video.avi",
					 width,height);
	system(buf);
	system("rm -rf temp");
}

void MakeImage(){
	int width = 1920;
	int height = 1080;
	GrowthImage g(width,height,width*height,ColorChoice::Nearest);
	g.IterateUntilDone();
	g.Save("image.png");
}

int main(){
	MakeImage();
}
