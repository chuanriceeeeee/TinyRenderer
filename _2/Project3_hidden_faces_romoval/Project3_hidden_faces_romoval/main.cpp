#define _USE_MATH_DEFINES

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

float triangle_area(int xa, int  ya, int xb, int yb, int xc, int yc)
{
	return .5 * ((yb - ya) * (xb + xa) + (ya - yc) * (xa + xc) + (yc - yb) * (xc + xb));
}


void triangle(int xa, int ya, int za, int xb, int yb, int zb, int xc, int yc, int zc, TGAImage& framebuffer, TGAColor color)
{
	int boxmax_x = std::max(std::max(xa, xb), xc);
	int boxmax_y = std::max(std::max(ya, yb), yc);
	int boxmax_z = std::max(std::max(za, zb), zc);
	int boxmin_x = std::min(std::min(xa, xb), xc);
	int boxmin_y = std::min(std::min(ya, yb), yc);
	int boxmin_z = std::min(std::min(za, zb), zc);

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
				double z_double = za * alpha + zb * beta + zc * gamma;
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


constexpr mat<4, 4> project_matrix = { {
		{width / 2.0, 0, 0, width / 2.0},
		{0, height / 2.0, 0, height / 2.0},
		{0, 0, 255. / 2.0, 255. / 2.0},
		{0, 0, 0, 1}
		} };

constexpr double ang = 45.0 * M_PI / 180.0;
mat<4, 4> rotate_matrix = { {
	{std::cos(ang), 0, std::sin(ang), 0 },
	{0, 1, 0, 0 },
	{ -(std::sin(ang)), 0, std::cos(ang), 0 },
	{ 0, 0, 0, 1 }
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

vec4 rot_y(const vec4 vert)
{
	return rotate_matrix * vert;
}




int main(int argc, char** argv) {
	TGAImage framebuffer_model(width, height, TGAImage::GRAYSCALE);

	Model* model = new Model("./models/diablo3_pose.obj");
	auto currrent = std::chrono::steady_clock::now();
	for (int i = 0; i < model->nfaces(); i++)
	{
		auto [x1, y1, z1, temp1] = project(perspective(rot_y(model->vert(i, 0))));
		auto [x2, y2, z2, temp2] = project(perspective(rot_y(model->vert(i, 1))));
		auto [x3, y3, z3, temp3] = project(perspective(rot_y(model->vert(i, 2))));

		triangle(x1, y1, z1, x2, y2, z2, x3, y3, z3, framebuffer_model, { static_cast <unsigned char>(255),static_cast <unsigned char>(255),static_cast <unsigned char>(255),static_cast <unsigned char>(255) });
	}
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - currrent;
	std::cout << elapsed << std::endl;
	framebuffer_model.write_tga_file("framebuffer_model.tga");

	TGAImage framebuffer_homework(width, height, TGAImage::RGBA);
	int x1 = 300, y1 = 350, z1 = 13;
	int x2 = 500, y2 = 600, z2 = 128;
	int x3 = 200, y3 = 650, z3 = 255;
	triangle(x1, y1, z1, x2, y2, z2, x3, y3, z3, framebuffer_homework, { static_cast <unsigned char>(255),static_cast <unsigned char>(255),static_cast <unsigned char>(255),static_cast <unsigned char>(255) });
	framebuffer_homework.write_tga_file("framebuffer_homework.tga");

	return 0;
}