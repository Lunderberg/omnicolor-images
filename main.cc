#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>

#include "CompiledAlgorithms.hh"
#include "GrowthImage.hh"

void MakeVideo(GrowthImage& g, std::string output, int iterations_per_frame){
  int err;
  int picnum = 0;

  err = system("rm -rf temp && mkdir temp");
  if(err){
    return;
  }

  for(int i=0; g.Iterate(); i++){
    if(i%iterations_per_frame==0){
      std::stringstream ss;
      ss << "temp/growth_" << picnum++ << ".png";
      g.Save(ss.str());
      std::cout << "\rIteration: (" << i << "/" << g.GetWidth()*g.GetHeight() << ")" << std::flush;
    }
  }
  std::cout << std::endl;

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
  if(err){
    return;
  }
  //err = system("rm -rf temp");
}

void MakeImage(GrowthImage& g, std::string output){
  g.IterateUntilDone();
  g.Save(output);
}

SmartEnum(LocationChoice, Random, Preferred, Sequential);
SmartEnum(PreferenceChoice, Location, Perlin);

int main(int argc, char** argv){
  int height, width;
  double epsilon;
  int iterations_per_frame;
  LocationChoice location_choice;
  PreferenceChoice preference_choice;
  int seed;
  std::string output;
  int preferred_location_iterations;
  int perlin_octaves;
  double perlin_grid_size;

  std::string lua_scriptname;

  namespace po = boost::program_options;
  po::options_description desc("Options");
  desc.add_options()
    ("input,i", po::value(&lua_scriptname),
     "Filename of lua script.  Overrides all other input options if present.")
    ("output,o", po::value(&output)->required(), "Output filename")
    ("width,w", po::value(&width)->default_value(256), "Width of the output image")
    ("height,h", po::value(&height)->default_value(128), "Height of the output image")
    ("epsilon,e", po::value(&epsilon)->default_value(5), "Epsilon (allowed error).  Zero = None allowed")
    ("video,v", "Render as a video instead of a still image")
    ("iter-per-frame", po::value(&iterations_per_frame)->default_value(1000),
     "Iterations between each frame")
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
    ("loc-iter", po::value(&preferred_location_iterations)->default_value(10),
     "How often to repeat to find a close value")
    ("help","Print help message")
    ;


  po::variables_map vm;
  try{
    po::store(po::parse_command_line(argc,argv,desc),vm);

    if(vm.count("help")){
      std::cout << "Growth Image Generator" << std::endl
                << desc << std::endl;
      return 0;
    }

    po::notify(vm);
  } catch (po::error& e){
    std::cerr << "ERROR: " << e.what() << std::endl
              << desc << std::endl;
    return 1;
  }


  std::unique_ptr<GrowthImage> g;
  if(vm.count("input")){
    g = std::unique_ptr<GrowthImage>(new GrowthImage(lua_scriptname.c_str()));
  } else {
    g = std::unique_ptr<GrowthImage>(new GrowthImage(width,height,seed));

    switch(location_choice){
    case LocationChoice::Random:
      g->SetLocationGenerator(generate_frontier_location);
      break;
    case LocationChoice::Sequential:
      g->SetLocationGenerator(generate_sequential_location(width,height));
      break;
    case LocationChoice::Preferred:
      g->SetLocationGenerator(generate_preferred_location(preferred_location_iterations));
      break;
    }

    switch(preference_choice){
    case PreferenceChoice::Location:
      g->SetPreferenceGenerator(generate_location_preference());
      break;
    case PreferenceChoice::Perlin:
      g->SetPreferenceGenerator(generate_perlin_preference(perlin_grid_size,
                                                           perlin_octaves,
                                                           g->GetRNG()));
      break;
    }

    g->SetEpsilon(epsilon);
  }

  if(vm.count("video")){
    MakeVideo(*g, output, iterations_per_frame);
  } else {
    MakeImage(*g, output);
  }
}
