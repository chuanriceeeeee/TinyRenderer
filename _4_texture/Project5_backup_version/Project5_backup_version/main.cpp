#include <cstdlib>
#include "our_gl.h"
#include "model.h"
#include <cmath>

extern mat<4, 4> ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer


struct BlingPhongShader : IShader // Inheritate
{
	const Model& model;
	TGAColor color = { };
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
		//vec3 v = model.vert(face, vert);
		//vec4 gl_Position = ModelView * vec4{ v.x, v.y, v.z, 1. };
		//tri[vert] = gl_Position.xyz();

		uv[vert] = model.uv(face, vert);
		vec4 gl_Position = ModelView * model.vert(face, vert);
		//// invert_transpose 防止缩放导致变形
		vec4 n = model.normal(face,vert); // tangent normal

		vec4 normal_direction = ModelView.invert_transpose() * vec4 { n.x, n.y, n.z, 0. };
		norm[vert] = normal_direction;

		diffusemap = model.diffuse();
		specularmap = model.specular();

		return Perspective * gl_Position;
	}

	virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const
	{
		TGAColor gl_FragColor{ 255, 255, 255, 255 };

		// 1. bar
		//vec3 frag_pos = tri[0] * bar.x + tri[1] * bar.y + tri[2] * bar.z;
		vec2 frag_uv = uv[0] * bar.x + uv[1] * bar.y + uv[2] * bar.z;

		// 2. norm
		vec4 n = normalized(bar.x * norm[0] + bar.y * norm[1] + bar.z * norm[2]);

		// 3. 计算视线向量 v
		//vec3 v = normalized(frag_pos * -1.);
		vec4 r = normalized(n * (n * l) * 2 - l);
		// 4. 环境光 (Ambient)
		double ambient = 0.6; 

		// 5. 漫反射 (Diffuse) 
		double diff_intensity = std::max(0.0, n * l);

		// 6. 高光 (Specular)
		double spec_intensity = 0.0;
		if (diff_intensity > 0.0) {
			//vec3 h = normalized(l.xyz() + v);
			spec_intensity = std::pow(std::max(r.z , 0.0), 32);
		}

		// 7. 读取纹理
		TGAColor basecolor = diffusemap.get(diffusemap.width() * frag_uv.x, diffusemap.height() * frag_uv.y);
		TGAColor specular_value = specularmap.get(specularmap.width() * frag_uv.x, specularmap.height() * frag_uv.y);

		// 8. 组合最终颜色
		for (int channel : {0, 1, 2}) {
			double diffuse_color = gl_FragColor[channel] * (ambient * basecolor[channel]
				+ basecolor[channel] * diff_intensity); // 此处依然有问题！specular always wrong...
			double specular_color = 255. * (specular_value[0] / 255.) * spec_intensity;
			double final_color = diffuse_color + specular_color;
			gl_FragColor[channel] = std::min(255.0, std::max(0.0, final_color));
		}

		return { false, gl_FragColor };
	}
};

int main(int argc, char** argv) {
	if (argc < 2) {
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
	init_perspective(norm(eye - center));                        // build the Perspective matrix
	init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8); // build the Viewport    matrix
	init_zbuffer(width, height);
	TGAImage framebuffer(width, height, TGAImage::RGB);


	for (int m = 1; m < argc; m++)
	{
		Model model(argv[m]);
		BlingPhongShader shader(light, model, eye);
		for (int f = 0; f < model.nfaces(); f++) {      // iterate through all facets
			Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
							  shader.vertex(f, 1),
							  shader.vertex(f, 2) };
			rasterize(clip, shader, framebuffer);   // rasterize the primitive
		}
	}
	framebuffer.write_tga_file("framebuffer.tga");
	return 0;
}