#include <cstdlib>
#include "our_gl.h"
#include "model.h"
#include <cmath>

extern mat<4, 4> ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer

//struct RandomShader : IShader // Inheritate
//{
//    const Model& model;
//    TGAColor color = { };
//    vec3 l;
//    vec3 tri[3];
//
//    RandomShader(const Model& m) : model(m)//输入模型时使用model初始化提取m模型信息
//    {}
//
//    virtual vec4 vertex(const int face, const int vert)
//    {
//        vec3 v = model.vert(face, vert);  //face的vert点向量vec4
//        vec4 gl_Position = ModelView * vec4{ v.x,v.y,v.z,1. };
//        tri[vert] = gl_Position.xyz();
//        return Perspective * gl_Position;
//    }
//
//    //virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const
//    //{
//    //    TGAColor gl_FragColor{ 255,255,255,255 };
//    //    vec3 n = normalized(cross(tri[1] - tri[0], tri[2] - tri[0]));
//    //    vec3 r = normalized(n * (n * l) * 2 - l);
//    //    return { false, color };
//    //}
//};

struct PhongShader : IShader // Inheritate
{
    const Model& model;
    TGAColor color = { };
    vec3 l;
    vec3 eye_real_position;
    vec3 tri[3];
    PhongShader(const vec3 light, const Model& m,const vec3 eye) : model(m)//输入模型时使用model初始化提取m模型信息
    {
        l = normalized((ModelView * vec4{ light.x, light.y, light.z, 0. }).xyz());
        eye_real_position = (ModelView * vec4{ eye.x,eye.y,eye.z,0. }).xyz();//无需归一化因为这是一个坐标不是方向
    }

    virtual vec4 vertex(const int face, const int vert)
    {
        vec3 v = model.vert(face, vert);  //face的vert点向量vec4
        vec4 gl_Position = ModelView * vec4{ v.x,v.y,v.z,1. }; //相对于{0，0，0}位置相机的视线方向
        tri[vert] = gl_Position.xyz();
        varying_tri[vert] = gl_Position.xyz();
        return Perspective * gl_Position;
    }

    virtual vec3 normal(const int face, const int vert)
    {
        vec4 n = model.normal(face, vert);
        vec4 normal_direction = ModelView.invert_transpose() * vec4{ n.x,n.y,n.z,0. }; //防止法线经过缩放后不再垂直
        return normal_direction;//此处不要乘以缩放矩阵
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar,const vec3 normal, const vec3 frag_pos) const
    {
        TGAColor gl_FragColor{ 255,255,255,255 };
        vec3 n = normal;
        vec3 r = normalized(n * (n * l) * 2 - l);
        vec3 v = normalized(eye_real_position - frag_pos);//已在rasterize中normalized
        double ambient = .3; 
        double diff = std::max(0., n * l); //漫反射强度，法线和光线的距离，越近越亮
        double spec = std::pow(std::max(r*v, 0.), 32); //高光强度，调整次方系数实现夹角固定
        for (int channel : {0, 1, 2})
            gl_FragColor[channel] *= std::min(1., ambient + .4 * diff + .9 * spec); // .4 .9 即为漫反射系数 高光反射系数 
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


    Model model("models/african_head/african_head.obj");                     // load the data
    PhongShader shader({1,1,1},model,eye);
        for (int f = 0; f < model.nfaces(); f++) {      // iterate through all facets
            //shader.color = {
            //    static_cast<unsigned char>(std::rand() % 255),
            //    static_cast<unsigned char>(std::rand() % 255),
            //    static_cast<unsigned char>(std::rand() % 255),
            //    255 };
            vec3 normal[3] = {
                shader.normal(f,0),
                shader.normal(f,1),
                shader.normal(f,2) };

            Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            rasterize(clip, normal, shader, framebuffer,eye);   // rasterize the primitive
        }
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}