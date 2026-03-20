```c++
#pragma once

#include <cmath>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

// restore zbuffer
std::vector<double> zbuffer;

// restore matrix
mat<4,4> ModelView, Viewport, Perspective;

// restore vertex
typedef Triangle vec3[3];

void init_model_view(const vec3 center, const vec3 eye, const vec3 up);

void init_viewport(const int width, const int height);

void init_perspective(const double focus_c);

void rasterize(const Triangle clip, Ishader shader, TGAImage framebuffer);

struct Ishader
{
    virtual 
};
#pragma once
```

```c++
//header file
#include "our_gl.h"

//matrix
//ModelView
void init_model_view(vec3 center, vec3 eye, vec3 up)
{
    vec3 L = normalized(cross(center,up));
    vec3 m = normalized(cross(l,eye-center));
    vec3 n = normalized(cross(l,m));
    mat<4,4> temp = {l,m,n,0};
    ModelView = temp.transpose();
}

void init_viewport(int width, int height)
{
    Viewport = {{
        {width / 2., 0, 0, width / 2.},
        {0, height / 2., 0, 0, height / 2.},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }};
}

void init_zbuffer(int width, int height)
{
    zbuffer = std::vector(width * height, -1000.);
}

void init_perspective(double focus_c)
{
    Perspective = {{ 
        {1,0,0,0},
        {0,1,0,0},
        {0,0,1,0},
        {0,0,-1. * focus_c , 0}
    }};
}
//clip - after pespective, framebuffer - image, shader - color light, 
void rasterize(const Triangle & clip[], IShader shader,TGAImage & framebuffer)
{
    
}
```


```c++
#include "iostream"
extern std::vector<double> zbuffer

struct BlingPhongShader: IShader
{
    vec4 l;
    Model model;
    vec3 gl_position[3];// vertex after MVP
    vec3 tri[3];// vertex real 3d position

    //init
    BlingPhongShader (const vec3 light, Model& m, const vec3 )model(m)
    {
        l = ModelView * {light.x,light.y,light.z,0};
    }
    
    //vertex
    virtual vec3 vertex(const int face, const int vert)
    {
        tri[vert] = ModelView * model.vertex(face, vert);
        gl_position[vert] = Perspective * tri[vert];

        

    }
};

int main(argv,*argc)
{
    if (argc < 2)
    {
        std::cout<<"File read problem"<<std::endl;
    }
    exprconst int width = 800;
    exprconst int height = 800;
    vec3 eye = {-1, 1, 1};
    vec3 up = {0,0,1};
    vec3 center = {0,0,0};
    vec3 light = {1,1,1};
    double focus_c = 3.;
    
    init_zbuffer(width, height);
    init_model_view(center, eye, up);
    init_viewport(width, height);
    init_perspective(focus_c);

    

}
```