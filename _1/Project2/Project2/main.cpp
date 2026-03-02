#include "tgaimage.h"
#include <iostream>
#include <chrono>
#include "model.h"
#include "geometry.h"

constexpr TGAColor white = { 255, 255, 255, 255 }; // attention, BGRA order
constexpr TGAColor green = { 0, 255,   0, 255 };
constexpr TGAColor red = { 0,   0, 255, 255 };
constexpr TGAColor blue = { 255, 128,  64, 255 };
constexpr TGAColor yellow = { 0, 200, 255, 255 };

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
			ierror -= 2*(xb - xa);//mistake 2
			y += yb > ya ? 1 : (-1);
		}
	}
}

void triangle_out(int xa, int ya, int xb, int yb, int xc, int yc, TGAImage& framebuffer, TGAColor color) {
	if (yc > yb)
	{
		std::swap(xc,xb);
		std::swap(yc,yb);
	}
	if (yb > ya)
	{
		std::swap(xa,xb);
		std::swap(ya,yb);
	}
	if (yc > yb)
	{
		std::swap(xc,xb);
		std::swap(yc,yb);
	}
	
	int total_height = (ya - yc);
	
	if (xb != xc)
	{
		int segment_height = (yb - yc);
		int x1 = xa;
		int x2 = xa;
		for (int y = ya;y >= yb; y--)
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

void triangle(int xa, int ya, int xb, int yb, int xc, int yc, TGAImage& framebuffer, TGAColor color)
{
	int boxmax_x = std::max(std::max(xa, xb), xc);
	int boxmax_y = std::max(std::max(ya, yb), yc);
	int boxmin_x = std::min(std::min(xa, xb), xc);
	int boxmin_y = std::min(std::min(ya, yb), yc);
	double total_area = triangle_area(xa, ya, xb, yb, xc, yc);
	if (total_area < 1) 
		return;

#pragma omp parallel for
	for (int x = boxmin_x; x <= boxmax_x; x++)
	{
		for (int y = boxmin_y; y <= boxmax_y; y++)
		{
			double PAB = triangle_area(xa, ya, xb, yb, x, y) / total_area;
			double PAC = triangle_area(xb, yb, xc, yc, x, y) / total_area;
			double PBC = triangle_area(xc, yc, xa, ya, x, y) / total_area;
			if (PAB < 0 || PAC < 0 || PBC < 0) 
				continue;
			framebuffer.set(x, y, { static_cast <unsigned char>(255),static_cast <unsigned char>(255),static_cast <unsigned char>(255),static_cast <unsigned char>()});
		}
	}
}

int main(int argc, char** argv) {
	constexpr int width = 800;
	constexpr int height = 800;
	TGAImage framebuffer_model(width, height, TGAImage::RGB);
	/*TGAImage framebuffer(width, height, TGAImage::RGB);
	triangle(7, 45, 35, 100, 45, 60, framebuffer, red);
	triangle(120, 35, 90, 5, 45, 110, framebuffer, white);
	triangle(115, 83, 80, 90, 85, 120, framebuffer, green);
	*/
	Model* model = new Model("./obj/diablo3_pose.obj");
	auto currrent = std::chrono::steady_clock::now();
	for (int i = 0; i < model->nfaces(); i++)
	{
		vec4 vert_1 = model->vert(i, 0);
		vec4 vert_2 = model->vert(i, 1);
		vec4 vert_3 = model->vert(i, 2);
		auto [x1,y1,z1] = (vert_1.x + 1) * width / 2, (vert_1.x + 1)* width / 2,(vert_1.x + 1) * width / 2;
		
		triangle(x1, y1, x2, y2, x3, y3, framebuffer_model, { static_cast < unsigned char>(255),static_cast < unsigned char>(255),static_cast < unsigned char>(255),static_cast <unsigned char>(255)});
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - currrent;
	std::cout << elapsed<< std::endl;
	framebuffer_model.write_tga_file("framebuffer_model.tga");
	return 0;
}