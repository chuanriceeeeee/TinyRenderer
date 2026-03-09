#define _USE_MATH_DEFINES
/*constexpr mat<4, 4> project_matrix = { {	{width / 2.0, 0, 0, width / 2.0},
		{0, height / 2.0, 0, height / 2.0},
		{0, 0, 255. / 2.0, 255. / 2.0},
		{0, 0, 0, 1}
		} };


constexpr mat<4, 4> Pers = { {
		{1.0, 0, 0, 0},
		{0, 1.0, 0, 0},
		{0, 0, 1.0, 0},
		{0, 0, -1.0 / (focal_length), 1.0}
		} };

vec4 project(const vec4 vert)
{
	return project_matrix * vert ;
}


vec4 perspective(const vec4 vert)
{
	vec4 temp = Pers * vert;
	return temp / temp . w;
}


mat<4,4> view_matrix_construct(const vec3 center, const vec3 eye, const vec3 up)
{
	vec3 n = normalized(eye - center);
	vec3 l = normalized(cross(n, up));
	vec3 m = normalized(cross(l, n));
	mat <4, 4> center_m = { {
		{1, 0, 0, - center.x},
		{0, 1, 0, - center.y},
		{0, 0, 1, - center.z},
		{0, 0, 0, 1}
		} };
	mat <4, 4> cob= { {
		{l.x, l.y, l.z, 0},
		{m.x, m.y, m.z, 0},
		{n.x, n.y, n.z, 0},
		{0, 0, 0, 1}
		} };
	return cob * center_m;
}

vec4 view_point(const vec4 vert)
{
	return  * vert;
}*/
//constexpr double ang = 45.0 * M_PI / 180.0;
//mat<4, 4> rotate_matrix = { {
//	{std::cos(ang), 0, std::sin(ang), 0 },
//	{0, 1, 0, 0 },
//	{ -(std::sin(ang)), 0, std::cos(ang), 0 },
//	{ 0, 0, 0, 1 }
//	} };
//vec4 rot_y(const vec4 vert)
//{
//	return rotate_matrix * vert;
//}
//

#include "tgaimage.h"
#include <iostream>
#include <chrono>
#include "model.h"
#include "geometry.h"
#include <algorithm>
#include <cmath>
#include <Eigen/Dense>

constexpr TGAColor white = { 255, 255, 255, 255 }; // attention, BGRA order
constexpr TGAColor green = { 0, 255,   0, 255 };
constexpr TGAColor red = { 0,   0, 255, 255 };
constexpr TGAColor blue = { 255, 128,  64, 255 };
constexpr TGAColor yellow = { 0, 200, 255, 255 };

constexpr int width = 800;
constexpr int height = 800;
constexpr double focal_length = 3;
									
mat<4, 4> View, Viewport, Perspective;

void view(const vec3 eye, const vec3 center, const vec3 up) {
	vec3 n = normalized(eye - center);
	vec3 l = normalized(cross(up, n));
	vec3 m = normalized(cross(n, l));
	View = mat<4, 4>{ {{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}} } *
		mat<4, 4>{{{1, 0, 0, -center.x}, { 0,1,0,-center.y }, { 0,0,1,-center.z }, { 0,0,0,1 }}};
}

void perspective(const double f) {
	Perspective = { {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1 / f,1}} };
}

void viewport(const int x, const int y, const int w, const int h) {
	Viewport = { {{w / 2., 0, 0, x + w / 2.}, {0, h / 2., 0, y + h / 2.}, {0,0,255. / 2.0 , 255. / 2.0}, {0,0,0,1}} };
}


float triangle_area(int xa, int  ya, int xb, int yb, int xc, int yc)
{
	return .5 * ((yb - ya) * (xb + xa) + (ya - yc) * (xa + xc) + (yc - yb) * (xc + xb));
}


void triangle(vec4 vert_array[], TGAImage& framebuffer, TGAColor color)
{

	int boxmin_x = vert_array[0].x;
	for (int i = 1; i < 3; i++)
		boxmin_x = std::min(boxmin_x, static_cast<int>(vert_array[i].x));

	int boxmin_y = vert_array[0].y;
	for (int i = 1; i < 3; i++)
		boxmin_y = std::min(boxmin_y, static_cast<int>(vert_array[i].y));
	
	int boxmax_x = vert_array[0].x;
	for (int i = 1; i < 3; i++)
		boxmax_x = std::max(boxmax_x, static_cast<int>(vert_array[i].x));
	
	int boxmax_y = vert_array[0].y;
	for (int i = 1; i < 3; i++)
		boxmax_y = std::max(boxmax_y, static_cast<int>(vert_array[i].y));

	int xa = vert_array[0].x, ya = vert_array[0].y, za = vert_array[0].z, 
		xb = vert_array[1].x, yb = vert_array[1].y, zb = vert_array[1].z, 
		xc = vert_array[2].x, yc = vert_array[2].y, zc = vert_array[2].z;

	double total_area = triangle_area(xa, ya, xb, yb, xc, yc);
	if (total_area < 0.5)
		return;
	//#pragma omp parallel for
	for (int x = boxmin_x; x <= boxmax_x; x++)
	{
		for (int y = boxmin_y; y <= boxmax_y; y++)
		{
			double alpha = triangle_area(xa, ya, xb, yb, x, y) / total_area;
			double beta = triangle_area(xb, yb, xc, yc, x, y) / total_area;
			double gamma = triangle_area(xc, yc, xa, ya, x, y) / total_area;
			if (alpha < 0 || beta < 0 || gamma < 0)
				continue;

			//if (alpha < 0.1 || beta < 0.1 || gamma < 0.1)
			{
				double z_double = zc * alpha + za * beta + zb * gamma;
				double b = gamma * 255.0;
				double g = beta * 255.0;
				double r = alpha * 255.0;
				unsigned char z = static_cast<unsigned char>(z_double);
				color = { static_cast<unsigned char>(b),static_cast<unsigned char>(g),static_cast<unsigned char>(r),255 };
				if (z <= framebuffer.get(x, y)[0]) continue; //记录(x,y)像素点上的最大深度
				framebuffer.set(x, y, { z });
			}
		}
	}
}

int main(int argc, char** argv) {
	TGAImage framebuffer_model(width, height, TGAImage::GRAYSCALE);

	Model* model = new Model("./models/diablo3_pose.obj");
	auto currrent = std::chrono::steady_clock::now();
	constexpr vec3    eye{ 0,0,3}; // camera position
	constexpr vec3 center{ 0,0,0 };  // camera direction
	constexpr vec3     up{ 0,1,0 };  // camera up vector

	view(eye, center, up);											// build the ModelView   matrix
	perspective(norm(eye - center));                        // build the Perspective matrix
	viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8); // build the Viewport    matrix

	for (int i = 0; i < model->nfaces(); i++)
	{
		vec4 vert_array[3];
		for (int j = 0; j < 3; j++)
		{
			vert_array[j] = Viewport * Perspective * View * model->vert(i, j);
			vert_array[j] =vert_array[j] / vert_array[j].w;
		}

		triangle(vert_array, framebuffer_model, {static_cast <unsigned char>(255),static_cast <unsigned char>(255),static_cast <unsigned char>(255),static_cast <unsigned char>(255)});
	}
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - currrent;
	std::cout << elapsed << std::endl;
	framebuffer_model.write_tga_file("framebuffer_model.tga");

	TGAImage framebuffer_homework(width, height, TGAImage::RGBA);
	int x1 = 300, y1 = 350, z1 = 13;
	int x2 = 500, y2 = 600, z2 = 128;
	int x3 = 200, y3 = 650, z3 = 255;
	//triangle(x1, y1, z1, x2, y2, z2, x3, y3, z3, framebuffer_homework, { static_cast <unsigned char>(255),static_cast <unsigned char>(255),static_cast <unsigned char>(255),static_cast <unsigned char>(255) });
	//framebuffer_homework.write_tga_file("framebuffer_homework.tga");

	return 0;
}