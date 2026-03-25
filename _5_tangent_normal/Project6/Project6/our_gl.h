#pragma once
#include "tgaimage.h"
#include "geometry.h"
#include <algorithm> //minmax 
float triangle_area(const int xa, const int ya, const int xb, const int yb, const int xc, const int yc);
void init_viewport(const int x, const int y, const int w, const int h);
void init_perspective(const double f);
void init_modelview(const vec3 eye, const vec3 center, const vec3 up);
void init_modelview_light(const vec3 light, const vec3 center, const vec3 up);
void init_zbuffer(const int width, const int height);
void init_shadow_zbuffer(const int width, const int height);
void init_n_m_minus();
void shadow(TGAImage& shadowmap, const vec4 (&clip)[]);


struct IShader {
        //static TGAColor sample2D(const TGAImage& img, const vec2& uvf) {
        //    return img.get(uvf[0] * img.width(), uvf[1] * img.height());
        //}
    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0; // 声明此处为纯虚函数 const = 0;
    virtual TGAColor shadow(const vec3 shadow_bar, const TGAColor color) const = 0;
};

typedef vec4 Triangle[3]; // a triangle primitive is made of three ordered points


TGAColor sample2d(const TGAImage & map, const vec2& uv); 

void rasterize(const vec4 (&clip)[], const IShader& shader, TGAImage& framebuffer);