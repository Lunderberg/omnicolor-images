#include "PerlinNoise.hh"

#include <cmath>
#include <algorithm>

#include <iostream>
using namespace std;

PerlinNoise::PerlinNoise(std::mt19937& rng) : octaves(1), grid_size(1){
  std::uniform_real_distribution<> uniform_theta(0,2*3.1415926535);

  for(auto& p : gradients){
    double theta = uniform_theta(rng);
    p = {cos(theta),sin(theta)};
  }

  for(unsigned int i=0; i<permute.size(); i++){
    permute[i] = i;
  }

  std::shuffle(permute.begin(), permute.end(), rng);
}

double PerlinNoise::operator()(double x, double y){
  return (*this)({x,y});
}

double PerlinNoise::operator()(GVector<2> p){
  p /= grid_size;

  double output = 0;
  for(int i=0; i<octaves; i++){
    output += base_perlin(p)*std::pow(0.5,i);
    p *= 2;
  }
  return output;
}

double PerlinNoise::base_perlin(GVector<2> p){
  int i = p.X();
  int j = p.Y();
  p -= {i,j};

  double v_dd = gradient_at(i  ,j  ) * p;
  double v_du = gradient_at(i  ,j+1) * (p - GVector<2>(0,1));
  double v_ud = gradient_at(i+1,j  ) * (p - GVector<2>(1,0));
  double v_uu = gradient_at(i+1,j+1) * (p - GVector<2>(1,1));

  double v_d = interpolate(v_dd,v_du,p.Y());
  double v_u = interpolate(v_ud,v_uu,p.Y());

  double v = interpolate(v_d,v_u,p.X());

  return v;
}

GVector<2> PerlinNoise::gradient_at(int i, int j){
  unsigned char output = ((i%256) + 256) % 256;
  output = permute[output];
  output = (((output+j)%256) + 256) % 256;
  output = permute[output];
  return gradients[output];
}

double PerlinNoise::interpolate(double v0, double v1, double t){
  //t = t*t*(3-2*t); //Zero derivative at endpoint
  t = t*t*t*(10 + t*(-15 + t*6)); //Zero derivative and zero second derivative
  return (1-t)*v0 + t*v1;
}
