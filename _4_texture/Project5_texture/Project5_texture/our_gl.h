#pragma once
#include "tgaimage.h"
#include "geometry.h"
#include <algorithm> //minmax 
float triangle_area(const int xa, const int ya, const int xb, const int yb, const int xc, const int yc);
void init_viewport(const int x, const int y, const int w, const int h);
void init_perspective(const double f);
void init_modelview(const vec3 eye, const vec3 center, const vec3 up);
void init_zbuffer(const int width, const int height);


struct IShader {
    vec3 varying_tri[3];
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar,const vec3,const vec3) const = 0;
};

typedef vec4 Triangle[3]; // a triangle primitive is made of three ordered points

void rasterize(const vec4 (&vert_array)[], const vec3 (&normal_array)[],const IShader& shader, TGAImage& framebuffer,const vec3 eye);