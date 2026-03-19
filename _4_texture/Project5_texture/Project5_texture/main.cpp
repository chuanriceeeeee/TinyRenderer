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
#include <cstdlib>
#include "our_gl.h"
#include "model.h"
#include <cmath>

extern mat<4, 4> ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer

//
//struct  BlingPhongShader : IShader // Inheritate
//{
//	const Model& model;
//	TGAColor color = { };
//	//vec3 l;
//	vec4 l;
//	vec3 eye_real_position;
//	vec3 tri[3];
//	vec3 norm[3];
//	vec2 uv[3];
//
//	TGAImage diffusemap;
//	TGAImage specularmap;
//
//	BlingPhongShader(const vec3 light, const Model& m, const vec3 eye) : model(m)//输入模型时使用model初始化提取m模型信息
//	{
//		l = normalized(ModelView.invert_transpose() * vec4{ light.x, light.y, light.z, 0. });
//		//eye_real_position = (ModelView * vec4{ eye.x,eye.y,eye.z,1. }).xyz();//无需归一化, eye的坐标经过平移旋转后的位置，不使用Perspective防止缩放后变形
//
//	}
//
//	virtual vec4 vertex(const int face, const int vert)
//	{
//		vec3 v = model.vert(face, vert);  //face的vert点向量vec4
//		vec4 gl_Position = ModelView * vec4{ v.x,v.y,v.z,1. }; //相对于{0，0，0}位置相机的视线方向
//		tri[vert] = gl_Position.xyz();//读取当前点的转换后实际3d位置
//		vec2 uv_value = model.uv(face, vert);//读取uv值
//		uv[vert] = uv_value;
//
//		vec4 n = model.normal(face, vert);
//		vec4 normal_direction = ModelView.invert_transpose() * vec4 { n.x, n.y, n.z, 0. }; //防止法线经过缩放后不再垂直
//		norm[vert] = normal_direction.xyz();
//
//		diffusemap = model.diffuse();
//		specularmap = model.specular();
//
//		return Perspective * gl_Position;
//	}
//
//	//virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const
//	//{
//	//	TGAColor gl_FragColor{ 255,255,255,255 }; 
//	//	vec3 frag_pos = tri[0] * bar.x + tri[1] * bar.y + tri[2] * bar.z;//实际坐标
//	//	vec2 frag_uv = uv[0] * bar.x + uv[1] * bar.y + uv[2] * bar.z;
//
//	//	vec3 n = normalized(bar.x * norm[0] + bar.y * norm[1] + bar.z * norm[2]);
//	//	vec3 r = normalized(n * (n * l) * 2 - l);
//	//	vec3 v = normalized(frag_pos * -1.);//此处无需normalize，因为normalized会导致
//
//	//	//TGAColor temp = diffusemap.get(frag_uv.x * diffusemap.width(), frag_uv.y * diffusemap.height()); // 此处需要乘以对应map的宽度和高度，否则无法得到正确rbg值
//	//	//TGAColor temp2 = specularmap.get(frag_uv.x * specularmap.width(), frag_uv.y * specularmap.height());
//	//	double ambient = .3;
//	//	double diff = std::max(0., n * l); //漫反射强度，法线和光线的距离，越近越亮
//	//	double spec = 0.;
//	//	if (diff > 0.0) { // 只有当前片段朝向光源（漫反射大于0）时，才会有高光！防止背面高光透射。
//	//		vec3 h = normalized(l + v); //Bling-Phong shader 半程向量 通过原光线夹角和视线夹角的半程向量和法线的夹角来决定反射光线和视线的夹角是否有差
//	//		spec = std::pow(std::max(n * h, 0.0), 32);	
//	//	}
//	//	double final_light = std::min(1., ambient + .4 * diff + .9 * spec);
//
//	//	for (int channel : {0, 1, 2})
//	//		gl_FragColor[channel] *= final_light; // .4 .9 即为漫反射系数 高光反射系数 
//	//	return { false, gl_FragColor };
//	//}
//
//
//	virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const
//	{
//		TGAColor gl_FragColor{ 255, 255, 255, 255 };
//
//		// 1. 获取片段的真实坐标和UV
//		vec3 frag_pos = tri[0] * bar.x + tri[1] * bar.y + tri[2] * bar.z;
//		vec2 frag_uv = uv[0] * bar.x + uv[1] * bar.y + uv[2] * bar.z;
//
//		// 2. 插值法线并归一化
//		vec3 n = normalized(ModelView.invert_transpose() * model.normal(frag_uv));
//
//		// 3. 计算视线向量 v (必须归一化！)
//		vec3 v = normalized(frag_pos * -1.);
//
//		// 4. 环境光 (Ambient)
//		double ambient = 0.6;
//
//		// 5. 漫反射 (Diffuse) n +
//		double diff_intensity = std::max(0.0, n * l);
//
//		// 6. 高光 (Specular) - 采用纯正的 Blinn-Phong
//		double spec_intensity = 0.0;
//		if (diff_intensity > 0.0) { // 核心防漏光机制：只有被光照到的正面才有高光
//			vec3 h = normalized(l.xyz() + v); // 半程向量
//			spec_intensity = std::pow(std::max(n * h, 0.0), 32); // Blinn-Phong 的指数通常比 Phong 大一倍，64 效果较好
//		}
//
//		// 7. 纹理 diffusemap 
//		TGAColor basecolor = diffusemap.get(diffusemap.width() * frag_uv.x, diffusemap.height() * frag_uv.y);
//
//		// 8. 纹理 specularmap
//		TGAColor specular_value = specularmap.get(specularmap.width() * frag_uv.x, specularmap.height() * frag_uv.y);
//
//		// . 组合最终颜色
//		for (int channel : {0, 1, 2}) {
//			double final_light = ambient * basecolor[channel] + basecolor[channel] * diff_intensity + specular_value[channel] * spec_intensity;
//			gl_FragColor[channel] = final_light;
//		}
//
//		return { false, gl_FragColor };
//	}
//
//};


struct BlingPhongShader : IShader // Inheritate
{
	const Model& model;
	TGAColor color = { };
	vec4 l;
	vec3 tri[3];
	vec3 norm[3];
	vec2 uv[3];

	TGAImage diffusemap;
	TGAImage specularmap;

	BlingPhongShader(const vec3 light, const Model& m, const vec3 eye) : model(m)
	{
		// 修正 1：光线是一个方向向量，转换到观察空间直接乘 ModelView 即可
		l = normalized(ModelView * vec4{ light.x, light.y, light.z, 0. });
	}

	virtual vec4 vertex(const int face, const int vert)
	{
		vec3 v = model.vert(face, vert);
		vec4 gl_Position = ModelView * vec4{ v.x, v.y, v.z, 1. };
		tri[vert] = gl_Position.xyz();

		vec2 uv_value = model.uv(face, vert);
		uv[vert] = uv_value;

		vec4 n = model.normal(face, vert);
		// 这里的 norm 算得非常完美，是从 Object Space 到 View Space 的正确法线
		vec4 normal_direction = ModelView.invert_transpose() * vec4 { n.x, n.y, n.z, 0. };
		norm[vert] = normal_direction.xyz();

		diffusemap = model.diffuse();
		specularmap = model.specular();

		return Perspective * gl_Position;
	}

	virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const
	{
		TGAColor gl_FragColor{ 255, 255, 255, 255 };

		// 1. 获取片段的真实坐标和UV
		vec3 frag_pos = tri[0] * bar.x + tri[1] * bar.y + tri[2] * bar.z;
		vec2 frag_uv = uv[0] * bar.x + uv[1] * bar.y + uv[2] * bar.z;

		// ==========================================
		// 🚨 修正 2：绝地反击就在这一行！
		// 放弃直接使用未经 TBN 矩阵转换的法线贴图。
		// 改用重心坐标去插值你在 vertex 阶段算好的 3 个顶点的 View Space 法线！
		vec3 n = normalized(norm[0] * bar.x + norm[1] * bar.y + norm[2] * bar.z);
		// ==========================================

		// 3. 计算视线向量 v (View Space 下相机在原点，所以视线就是从片段指向原点)
		vec3 v = normalized(frag_pos * -1.);

		// 4. 环境光 (Ambient)
		double ambient = 0.2; // 建议调暗一点，0.6会导致整体泛白

		// 5. 漫反射 (Diffuse) 
		// 这里加上 .xyz() 更安全，防止 vec4 和 vec3 点乘出现隐式转换问题
		double diff_intensity = std::max(0.0, n * l.xyz());

		// 6. 高光 (Specular)
		double spec_intensity = 0.0;
		if (diff_intensity > 0.0) {
			vec3 h = normalized(l.xyz() + v);
			spec_intensity = std::pow(std::max(n * h, 0.0), 32);
		}

		// 7. 读取纹理
		TGAColor basecolor = diffusemap.get(diffusemap.width() * frag_uv.x, diffusemap.height() * frag_uv.y);
		TGAColor specular_value = specularmap.get(specularmap.width() * frag_uv.x, specularmap.height() * frag_uv.y);

		// 8. 组合最终颜色
		for (int channel : {0, 1, 2}) {
			double final_light = ambient * basecolor[channel]
				+ basecolor[channel] * diff_intensity
				+ specular_value[0] * spec_intensity; // 此处依然有问题！

			// 严格限制在 0-255，防止高光溢出变成黑洞
			gl_FragColor[channel] = std::min(255.0, std::max(0.0, final_light));
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
	constexpr vec3    eye{ -1, 0, 2 }; // camera position
	constexpr vec3 center{ 0, 0, 0 }; // camera direction
	constexpr vec3     up{ 0, 1, 0 }; // camera up vector
	constexpr vec3  light{ 1, 1, 1 };

	init_modelview(eye, center, up);                                   // build the ModelView   matrix
	init_perspective(norm(eye - center));                        // build the Perspective matrix
	init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8); // build the Viewport    matrix
	init_zbuffer(width, height);
	TGAImage framebuffer(width, height, TGAImage::RGB);


	for (int m = 1; m < argc; m++)
	{
		Model model(argv[m]);
		BlingPhongShader shader(light, model, eye);
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
	}
	framebuffer.write_tga_file("framebuffer.tga");
	return 0;
}