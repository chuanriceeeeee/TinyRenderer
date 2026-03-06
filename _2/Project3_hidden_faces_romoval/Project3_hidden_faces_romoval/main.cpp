#define _USE_MATH_DEFINES

#include "tgaimage.h"
#include <iostream>
#include <chrono>
#include "model.h"
#include "geometry.h"
#include <algorithm>
#include <cmath>

constexpr TGAColor white = { 255, 255, 255, 255 }; // attention, BGRA order
constexpr TGAColor green = { 0, 255,   0, 255 };
constexpr TGAColor red = { 0,   0, 255, 255 };
constexpr TGAColor blue = { 255, 128,  64, 255 };
constexpr TGAColor yellow = { 0, 200, 255, 255 };

constexpr int width = 800;
constexpr int height = 800;


void line(int xa, int ya, int xb, int yb, TGAImage& framebuffer, TGAColor color)
{
	bool steep = std::abs(xb - xa) < std::abs(yb - ya);//mistake 3 std::(yb-ya) mistake 4 > <
	if (steep)
	{
		std::swap(xa, ya);
		std::swap(xb, yb);
	}
	//transpose
	if (xa > xb)
	{
		std::swap(xa, xb);
		std::swap(ya, yb);
	}
	int temp = std::abs(yb - ya);//mistake 1
	int y = ya;
	int ierror = 0;
	for (int x = xa; x <= xb; x++)
	{//1 0 0 + 
		ierror += temp * 2;
		if (steep)
			framebuffer.set(y, x, color);
		else
			framebuffer.set(x, y, color);
		if (ierror >= xb - xa)
		{
			ierror -= 2 * (xb - xa);//mistake 2
			y += yb > ya ? 1 : (-1);
		}
	}
}

void triangle_out(int xa, int ya, int xb, int yb, int xc, int yc, TGAImage& framebuffer, TGAColor color) {
	if (yc > yb)
	{
		std::swap(xc, xb);
		std::swap(yc, yb);
	}
	if (yb > ya)
	{
		std::swap(xa, xb);
		std::swap(ya, yb);
	}
	if (yc > yb)
	{
		std::swap(xc, xb);
		std::swap(yc, yb);
	}

	int total_height = (ya - yc);

	if (xb != xc)
	{
		int segment_height = (yb - yc);
		int x1 = xa;
		int x2 = xa;
		for (int y = ya; y >= yb; y--)
		{
			x1 = xa + (xb - xa) * (ya - y) / static_cast<float>(total_height - segment_height);
			x2 = xa + (xc - xa) * (ya - y) / static_cast<float>(total_height);
			for (int x = std::min(x1, x2); x <= std::max(x1, x2); x++)
			{
				framebuffer.set(x, y, color);
			}
		}
		for (int y = yc; y <= yb; y++)
		{
			x1 = xc + (xb - xc) * (y - yc) / static_cast<float>(segment_height);
			x2 = xc + (xa - xc) * (y - yc) / static_cast<float>(total_height);
			for (int x = std::min(x1, x2); x <= std::max(x1, x2); x++)
			{
				framebuffer.set(x, y, color);
			}
		}
	}


}
float triangle_area(int xa, int  ya, int xb, int yb, int xc, int yc)
{
	return .5 * ((yb - ya) * (xb + xa) + (ya - yc) * (xa + xc) + (yc - yb) * (xc + xb));
}


//final version
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


vec4 project(vec4 vert)
{
	return vec4(
		(vert.x + 1) * width / 2,
		(vert.y + 1) * height / 2,
		(vert.z + 1) / 2.0 * 255.0,
		0
	);
}
vec4 perspective(vec4 vert)
{
	constexpr double t = 3.0 ;
	return vert / ( 1.0 - vert.z / t);
}

vec4 rot_y(vec4 vert)
{
	constexpr double ang = 45.0 * M_PI / 180.0;

	mat<4, 4> R = {{
		{std::cos(ang), 0, std::sin(ang), 0 }, 
		{0, 1, 0, 0 }, 
		{ -(std::sin(ang)), 0, std::cos(ang), 0 }, 
		{ 0, 0, 0, 1 }
		}};
	return R * vert;
}


int main(int argc, char** argv) {
	TGAImage framebuffer_model(width, height, TGAImage::GRAYSCALE);
	/*TGAImage framebuffer(width, height, TGAImage::RGB);
	triangle(7, 45, 35, 100, 45, 60, framebuffer, red);
	triangle(120, 35, 90, 5, 45, 110, framebuffer, white);
	triangle(115, 83, 80, 90, 85, 120, framebuffer, green);
	*/
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