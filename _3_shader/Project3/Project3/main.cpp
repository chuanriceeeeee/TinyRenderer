#include <cstdlib>
#include "our_gl.h"
#include "model.h"
#include <cmath>

extern mat<4, 4> ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer

struct RandomShader : IShader // Inheritate
{
    const Model& model;
    TGAColor color = { };
    vec3 l;
    vec3 tri[3];

    RandomShader(const Model& m) : model(m)//输入模型时使用model初始化提取m模型信息
    {}

    virtual vec4 vertex(const int face, const int vert)
    {
        vec3 v = model.vert(face, vert);  //face的vert点向量vec4
        vec4 gl_Position = ModelView * vec4{ v.x,v.y,v.z,1. };
        tri[vert] = gl_Position.xyz();
        return Perspective * gl_Position;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const
    {
        TGAColor gl_FragColor{ 255,255,255,255 };
        vec3 n = normalized(cross(tri[1] - tri[0], tri[2] - tri[0]));
        vec3 r = normalized(n * (n * l) * 2 - l);

        return { false, color };
    }
};

struct PhongShader : IShader // Inheritate
{
    const Model& model;
    TGAColor color = { };
    vec3 l;
    vec3 tri[3];

    PhongShader(const vec3 light, const Model& m) : model(m)//输入模型时使用model初始化提取m模型信息
    {
        l = normalized((ModelView * vec4{ light.x, light.y, light.z, 0. }).xyz());
    }

    virtual vec4 vertex(const int face, const int vert)
    {
        vec3 v = model.vert(face, vert);  //face的vert点向量vec4
        vec4 gl_Position = ModelView * vec4{ v.x,v.y,v.z,1. };
        tri[vert] = gl_Position.xyz();
        return Perspective * gl_Position;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const
    {
        TGAColor gl_FragColor{ 255,255,255,255 };
        vec3 n = normalized(cross(tri[1] - tri[0], tri[2] - tri[0]));
        vec3 r = normalized(n * (n * l) * 2 - l);
        double ambient = .1;
        double diff = std::max(0., n * l);
        double spec = std::pow(std::max(r.z, 0.), 100000);
        for (int channel : {0, 1, 2})
            gl_FragColor[channel] *= std::min(1., ambient + .4 * diff + .9 * spec);
        return { false, gl_FragColor };
    }
};

int main(int argc, char** argv) {
    constexpr int width = 800;      // output image size
    constexpr int height = 800;
    constexpr vec3    eye{ -1, 0, 2 }; // camera position
    constexpr vec3 center{ 0, 0, 0 }; // camera direction
    constexpr vec3     up{ 0, 1, 0 }; // camera up vector

    init_modelview(eye, center, up);                                   // build the ModelView   matrix
    init_perspective(norm(eye - center));                        // build the Perspective matrix
    init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8); // build the Viewport    matrix
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB);


    Model model("models/diablo3_pose.obj");                     // load the data
    PhongShader shader({0,2,2},model);
        for (int f = 0; f < model.nfaces(); f++) {      // iterate through all facets
            //shader.color = {
            //    static_cast<unsigned char>(std::rand() % 255),
            //    static_cast<unsigned char>(std::rand() % 255),
            //    static_cast<unsigned char>(std::rand() % 255),
            //    255 };
            Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            rasterize(clip, shader, framebuffer);   // rasterize the primitive
        }
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}