#include "tgaimage.h"
#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include "model.h"
#include "geometry.h"

void line(int x_a, int y_a, int x_b, int y_b, TGAImage& framebuffer, TGAColor color)
{
    bool steep = std::abs(x_b - x_a) < std::abs(y_b - y_a);
    if (steep)
    {
        std::swap(x_a, y_a);
        std::swap(x_b, y_b);

    }

    if (x_a > x_b)
    {
        std::swap(x_a, x_b);
        std::swap(y_a, y_b);
    }
    int  y = y_a;
    int ierror = 0;
    for (int x = x_a; x <= x_b; x++)
    {
        if (steep)
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ierror += 2 * std::abs(y_b - y_a);
        if (ierror >= x_b - x_a)
        {
            y += y_b > y_a ? 1 : -1;
            ierror -= 2 * (x_b - x_a);
        }
    }
    return;
}

int main()
{
	constexpr int width = 800;
	constexpr int height = 800;

	Model *model = new Model("./obj/diablo3_pose.obj");
	TGAImage work1(width, height, TGAImage::RGB);

	for (int i = 0; i < model->nfaces(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			vec4 v0 = model->vert(i, j);
            vec4 v1;
            if (j != 2)
                v1 = model->vert(i, j + 1);
            else
                v1 = model->vert(i, 0);
            auto x1 = (v1.x + 1) * width / 2;
            auto y1 = (v1.y + 1) * height / 2;
            auto x0 = (v0.x + 1) * width / 2;
            auto y0 = (v0.y + 1) * height / 2;
            line(x0, y0, x1, y1, work1, {static_cast<unsigned char> (rand()%255),static_cast<unsigned char>(rand() % 255),static_cast<unsigned char>(rand() % 255),static_cast<unsigned char>(rand() % 255) });
		}
	}
    work1.write_tga_file("work1.tga");
    return 0;
}