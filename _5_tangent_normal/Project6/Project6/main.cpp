#include <cstdlib>
#include "our_gl.h"
#include "model.h"
#include <cmath>

extern mat<4, 4> ModelView, Perspective, ModelViewLight; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer


struct BlingPhongShader : IShader // Inheritate
{
	const Model& model;
	vec4 l;
	vec3 tri[3];
	vec4 norm[3];
	vec2 uv[3];

	TGAImage diffusemap;
	TGAImage specularmap;

	BlingPhongShader(const vec3 light, const Model& m, const vec3 eye) : model(m)
	{
		// 光线是一个方向向量，转换到观察空间直接乘 ModelView 即可
		l = normalized(ModelView * vec4{ light.x, light.y, light.z, 0. });
	}

	virtual vec4 vertex(const int face, const int vert)
	{

		uv[vert] = model.uv(face, vert);
		vec4 gl_Position = ModelView * model.vert(face, vert);
		tri[vert] = gl_Position.xyz();

		// invert_transpose 防止缩放导致变形
		vec4 n = model.normal(face, vert);
		vec4 normal_direction = ModelView.invert_transpose() * vec4 { n.x, n.y, n.z, 0. };
		norm[vert] = normal_direction;

		diffusemap = model.diffuse();
		specularmap = model.specular();

		return Perspective * gl_Position;
	}

	virtual vec4 vertex_shadow(const int face, const int vert)
	{

		uv[vert] = model.uv(face, vert);
		vec4 shadow_gl_Position = ModelViewLight * model.vert(face, vert);
		return Perspective * shadow_gl_Position;
	}

	virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const
	{

		// bar
		//vec3 frag_pos = tri[0] * bar.x + tri[1] * bar.y + tri[2] * bar.z;
		vec2 frag_uv = uv[0] * bar.x + uv[1] * bar.y + uv[2] * bar.z;

		// norm
		vec4 normal_value = normalized(bar.x * norm[0] + bar.y *  norm[1] + bar.z * norm[2]);

		// TBN
		mat<3, 2> E = { {
			{tri[1].x - tri[0].x , tri[2].x - tri[0].x},
			{tri[1].y - tri[0].y , tri[2].y - tri[0].y},
			{tri[1].z - tri[0].z , tri[2].z - tri[0].z}
			} };
		mat<2, 2> U = { {//此处必须以0作为出发点，1 ，2 作为起点，或者1，2中任意一个作为出发点，任意向量出发点必须一致
			{uv[1].x - uv[0].x , uv[2].x - uv[0].x},
			{uv[1].y - uv[0].y , uv[2].y - uv[0].y},
			} };
		mat<3, 2> temp = E * (U.invert());
		vec3 T = normalized(vec3{ temp[0][0],temp[1][0],temp[2][0] });
		vec3 B = normalized(vec3{ temp[0][1],temp[1][1],temp[2][1] });
		vec3 N = normal_value.xyz();

	
		mat<4, 4> TBN = { {
			{T.x,B.x,N.x,0},
			{T.y,B.y,N.y,0},
			{T.z,B.z,N.z,0},
			{0,0,0,1}//此处0 1 无所谓，但是齐次坐标规范保持1
			} };
		vec4 n = normalized(TBN * model.normal(frag_uv)); //任何向量都需归一化
		// 计算视线向量 v
		//vec3 v = normalized(frag_pos * -1.);
		vec4 r = normalized(n * (n * l) * 2 - l);
		// 环境光 (Ambient)
		double ambient = 0.5;

		// 漫反射 (Diffuse) 
		double diff_intensity = std::max(0.0, n * l);

		// 高光 (Specular)
		double spec_intensity = 0.0;
		if (diff_intensity > 0.0) {
			//vec3 h = normalized(l.xyz() + v);
			spec_intensity = std::pow(std::max(r.z, 0.0), 32);
		}

		// 读取纹理
		TGAColor diffusecolor = sample2d(diffusemap, frag_uv);
		double specular_value = sample2d(specularmap,frag_uv)[0];

		TGAColor gl_FragColor = sample2d(diffusemap, frag_uv);

		// 组合最终颜色
		for (int channel : {0, 1, 2}) {
			double final_light = ambient
				+ diff_intensity
				+ 3 * specular_value / 255. * spec_intensity; // 此处依然有问题！specular always wrong...

			gl_FragColor[channel] = std::min(255.0, std::max(0.0, gl_FragColor[channel] * final_light));
		}
		return { false, gl_FragColor };
	}

	virtual TGAColor shadow(const vec3 shadow_bar, const TGAColor re_color) const //一旦丢失这个const会与原虚函数声明完全不同!导致编译器认为未实现
	{
		TGAColor color = re_color;
		for (int channel : {0, 1, 2}) {
			color[channel] = std::min(255.0, std::max(0.0, color[channel] * .3));
		}
		return color;
	}
};

int main(int argc, char** argv) {
	if (argc < 1) {
		std::cerr << "Usage: " << argv[0] << " model" << std::endl;
		return 1;
	}
	constexpr int width = 800;      // output image size
	constexpr int height = 800;
	constexpr vec3    eye{ -1, 0, 2 };// camera position
	constexpr vec3 center{ 0, 0, 0 }; // camera direction
	constexpr vec3     up{ 0, 1, 0 }; // camera up vector
	constexpr vec3  light{ 1, 1, 1 };
		
	init_modelview(eye, center, up);                             // build the ModelView   matrix
	init_modelview_light(light, center, up);
	init_perspective(norm(eye - center));                        // build the Perspective matrix
	init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8); // build the Viewport    matrix
	init_zbuffer(width, height);
	TGAImage framebuffer(width, height, TGAImage::RGB);


	for (int m = 1; m < argc; m++)
	{
		Model model(argv[m]);
		BlingPhongShader shader(-1 * light, model, eye);
		for (int f = 0; f < model.nfaces(); f++) {      // iterate through all facets
			Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
							  shader.vertex(f, 1),
							  shader.vertex(f, 2) };
			Triangle shadow_clip = { shader.vertex_shadow(f, 0),  // assemble the primitive
							  shader.vertex_shadow(f, 1),
							  shader.vertex_shadow(f, 2) };
			rasterize(clip, shadow_clip, shader, framebuffer);   // rasterize the primitive
		}
	}
	framebuffer.write_tga_file("framebuffer.tga");
	return 0;
}