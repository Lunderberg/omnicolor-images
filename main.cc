#include <iostream>
using std::cout;
using std::cerr;
using std::flush;
using std::endl;
#include <cstdio>
#include <string>
#include <sstream>

#include <boost/gil/gil_all.hpp>
#include <boost/program_options.hpp>

#include "GrowthImage.hh"

void MakeVideo(GrowthImage& g, std::string output, int iterations_per_frame){
  int err;
  int picnum = 0;

  err = system("rm -rf temp && mkdir temp");

  int framenum = 0;
  for(int i=0; g.Iterate(); i++){
    if(i%iterations_per_frame==0){
      std::stringstream ss;
      ss << "temp/growth_" << picnum++ << ".png";
      g.Save(ss.str());
      cout << "\rIteration: (" << i << "/" << g.GetWidth()*g.GetHeight() << ")" << flush;
    }
  }
  cout << endl;

  for(int i=0; i<24; i++){
    std::stringstream ss;
    ss << "temp/growth_" << picnum++ << ".png";
    g.Save(ss.str());
  }

  std::stringstream ss;
  ss << "ffmpeg -f image2 -framerate 12 -i \"temp/growth_%d.png\" -s "
     << g.GetWidth() << "x" << g.GetHeight()
     << " -vcodec h264 -crf 18 -pix_fmt yuv420p"
     << " " << output;
  std::string str = ss.str();
  err = system(str.c_str());
  //err = system("rm -rf temp");
}

void MakeImage(GrowthImage& g, std::string output){
  g.IterateUntilDone();
  g.Save(output);
}

int main(int argc, char** argv){
  int height, width;
  double epsilon;
  int iterations_per_frame;
  ColorChoice color_choice;
  LocationChoice location_choice;
  PreferenceChoice preference_choice;
  int seed;
  std::string output;
  int preferred_location_iterations;
  int perlin_octaves;
  double perlin_grid_size;

  namespace po = boost::program_options;
  po::options_description desc("Options");
  desc.add_options()
    ("help","Print help message")
    ("width,w", po::value(&width)->default_value(256), "Width of the output image")
    ("height,h", po::value(&height)->default_value(128), "Height of the output image")
    ("epsilon,e", po::value(&epsilon)->default_value(5), "Epsilon (allowed error).  Zero = None allowed")
    ("video,v", "Render as a video instead of a still image")
    ("iter-per-frame", po::value(&iterations_per_frame)->default_value(1000),
     "Iterations between each frame")
    ("color,c", po::value(&color_choice)->default_value(ColorChoice::Nearest),
     "Algorithm for selecting color")
    ("location,l", po::value(&location_choice)->default_value(LocationChoice::Random),
     "Algorithm for selecting the next pixel to fill")
    ("preference,p", po::value(&preference_choice)->default_value(PreferenceChoice::Location),
     "Algorithm for setting the location preference, for LocationAlgorithm \"Preferred\"")
    ("perlin-octaves", po::value(&perlin_octaves)->default_value(7),
     "Number of octaves of perlin noise to add together")
    ("perlin-grid", po::value(&perlin_grid_size)->default_value(50),
     "Size in pixels of largest perlin noise grid")
    ("seed,s", po::value(&seed)->default_value(0),
     "Random seed (0 = seed with current time)")
    ("output,o", po::value(&output)->required(), "Output filename")
    ("loc-iter", po::value(&preferred_location_iterations)->default_value(10),
     "How often to repeat to find a close value")
    ;


  po::variables_map vm;
  try{
    po::store(po::parse_command_line(argc,argv,desc),vm);

    if(vm.count("help")){
      cout << "Growth Image Generator" << endl
           << desc << endl;
      return 0;
    }

    po::notify(vm);
  } catch (po::error& e){
    cerr << "ERROR: " << e.what() << endl
         << desc << endl;
    return 1;
  }

  GrowthImage g(width,height,seed);
  g.SetColorChoice(color_choice);
  g.SetLocationChoice(location_choice);
  g.SetPreferenceChoice(preference_choice);
  g.SetPreferredLocationIterations(preferred_location_iterations);
  g.SetEpsilon(epsilon);

  g.SetPerlinOctaves(perlin_octaves);
  g.SetPerlinGridSize(perlin_grid_size);

  if(vm.count("video")){
    MakeVideo(g, output, iterations_per_frame);
  } else {
    MakeImage(g, output);
  }
}
