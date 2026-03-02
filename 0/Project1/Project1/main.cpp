//FUCTION
//1. std::abs
//2. TGAColor TGAImage
//3. std::round
//4. (TGAImage) framebuffer.set(x,y,(TGAColor)color)



//mistake1: forget to usze & in a function that need reference

#include "tgaimage.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

#include <ctime>
#include <chrono>

constexpr TGAColor white = { 255, 255, 255, 255 }; // attention, BGRA order
constexpr TGAColor green = { 0, 255,   0, 255 };
constexpr TGAColor red = { 0,   0, 255, 255 };
constexpr TGAColor blue = { 255, 128,  64, 255 };
constexpr TGAColor yellow = { 0, 200, 255, 255 };


void line(int x_a, int y_a, int x_b, int y_b, TGAImage & framebuffer, TGAColor color)
{
    int temp_x = x_b - x_a;
    int temp_y = y_b - y_a;
    for (float temp=0; temp <= 1; temp += .03)
    {
        int x = std::round(x_a + temp * temp_x);
        int y = std::round(y_a + temp * temp_y);
        framebuffer.set(x, y, color);
    }
    return;
}


void line2(int x_a, int y_a, int x_b, int y_b, TGAImage& framebuffer, TGAColor color)
{
    int gap_x = std::abs(x_a - x_b);
    int gap_y = std::abs(y_b - y_a);
    if(gap_x > gap_y)
    {
        if (x_b < x_a)
        {
            std::swap(x_a, x_b);
            std::swap(y_a, y_b);
        }
        for (int temp_x = x_a; temp_x <= x_b; temp_x++)
        {
            //integer division may cause several bugs, here translate it into float division
            float t = (temp_x - x_a) / static_cast<float>(x_b - x_a);
            int y = std::round(y_a + t * (y_b-y_a));//y_b-y_a because we need its 
            framebuffer.set(temp_x, y, color);

        }
    }
    else
    {
        if (y_b < y_a)
        {
            std::swap(x_a, x_b);
            std::swap(y_a, y_b);
        }
        for (int temp_y = y_a; temp_y <= y_b; temp_y++)
        {
            //integer division may cause several bugs, here translate it into float division
            float t = (temp_y - y_a) / static_cast<float>(y_b - y_a);
            int x = std::round(x_a + t * (x_b-x_a));
            framebuffer.set(x, temp_y, color);

        }
    }
    return;
}

//LERP 线性插值
void line3(int x_a, int y_a, int x_b, int y_b, TGAImage& framebuffer, TGAColor color)
{
    bool steep = std::abs(x_b-x_a) < std::abs(y_b-y_a);
    if (steep)
    {
        std::swap(x_a, y_a);
        std::swap(x_b, y_b);

    }

    if (x_a > x_b)
    {
        std::swap(x_a,x_b);
        std::swap(y_a,y_b);
    }
    for (int x= x_a; x <= x_b; x ++)
    {
        float t = (x - x_a) / static_cast<float>(x_b-x_a);
        int y = std::round(y_a + t * (y_b-y_a));
        if (steep)
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
    }
    return;
}
//optimization1
void line4(int x_a, int y_a, int x_b, int y_b, TGAImage& framebuffer, TGAColor color)
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
    // 此处使用int，c++中对int和float相加的处理是直接截断小数部分，而(x - x_a) / static_cast<float>(x_b - x_a)始终小于一导致小数一直加不上去
    float  y = y_a;
    for (int x = x_a; x <= x_b; x++)
    {

        if (steep)
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        y += (y_b - y_a) / static_cast<float>(x_b - x_a);
    }
    return;
}
//optimization2
void line5(int x_a, int y_a, int x_b, int y_b, TGAImage& framebuffer, TGAColor color)
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
    float error = 0;
    for (int x = x_a; x <= x_b; x++)
    {
        if (steep)
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        error += std::abs(y_b - y_a) / static_cast<float>(x_b - x_a);
        if (error >= .5)
        {
            y += y_b > y_a ? 1 : -1;
          error -= 1.;
        }
    }
    return;
}

//optimization3 Bresenham's line-drawing algorithm
void line6(int x_a, int y_a, int x_b, int y_b, TGAImage& framebuffer, TGAColor color)
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
        if (ierror >= x_b-x_a)
        {
            y += y_b > y_a ? 1 : (-1);
            ierror -= 2 * (x_b - x_a);
        }
    }
    return;
}


int main(int argc, char** argv) {
    /*constexpr int width = 64;
    constexpr int height = 64; 
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax = 7, ay = 3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.write_tga_file("framebuffer.tga");*/
    constexpr int width = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    TGAImage framebuffer2(width, height, TGAImage::RGB);
    
    TGAImage framebuffer3(width, height, TGAImage::RGB);

    TGAImage framebuffer4(width, height, TGAImage::RGB);

    TGAImage framebuffer5(width, height, TGAImage::RGB);

    TGAImage framebuffer6(width, height, TGAImage::RGB);

    int ax = 7, ay = 3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;
    line(ax, ay, bx, by, framebuffer, blue);
    line(cx, cy, bx, by, framebuffer, green);
    line(cx, cy, ax, ay, framebuffer, yellow);
    line(ax, ay, cx, cy, framebuffer, red);
    framebuffer.write_tga_file("line1.tga");
    //sample way 2
    line2(ax, ay, bx, by, framebuffer2, blue);
    line2(cx, cy, bx, by, framebuffer2, green);
    line2(cx, cy, ax, ay, framebuffer2, yellow);
    line2(ax, ay, cx, cy, framebuffer2, red);
    framebuffer2.write_tga_file("line2.tga");
    line3(ax, ay, bx, by, framebuffer3, blue);
    //sample way 3
    line3(ax, ay, bx, by, framebuffer3, blue);
    line3(cx, cy, bx, by, framebuffer3, green);
    line3(cx, cy, ax, ay, framebuffer3, yellow);
    line3(ax, ay, cx, cy, framebuffer3, red);

    std::srand(std::time({}));
    // time method2
    auto start_4 = std::chrono::steady_clock::now();

    for (int i = 0; i < (1 << 24); i++) {
        ax = rand() % width, ay = rand() % height;
        bx = rand() % width, by = rand() % height;
        line4(ax, ay, bx, by, framebuffer4, { static_cast<unsigned char>(rand() % 255), static_cast<unsigned char>(rand() % 255), static_cast<unsigned char>(rand() % 255), static_cast<unsigned char>(rand() % 255 )});
    }
    framebuffer4.write_tga_file("line4_v2.tga");
    auto end_4 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed_4 = end_4 - start_4;
    std::cout << elapsed_4.count() << "ms" << std::endl;

    auto start_5 = std::chrono::steady_clock::now();
    for (int i = 0; i < (1 << 24); i++)
    {
        ax = std::rand() % width, ay = std::rand() %height;
        bx = std::rand() % width, by = std::rand() % height;
        line5(ax, ay, bx, by, framebuffer5, { static_cast<unsigned char>(rand() % 255),static_cast<unsigned char>(rand() % 255) ,static_cast<unsigned char>(rand() % 255) ,static_cast<unsigned char>(rand() % 255) ,static_cast<unsigned char>(rand() % 255) });
    }
    auto end_5 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed_5 = end_5 - start_5;
    std::cout << elapsed_5.count() << "ms" << std::endl;

    framebuffer5.write_tga_file("line5.tga");

    auto start_6=std::chrono::steady_clock::now();
    for (int i = 0; i < (1 << 24); i++)
    {
        ax = std::rand() % width, ay = std::rand() % height;
        bx = std::rand() % width, by = std::rand() % height;
        line6(ax, ay, bx, by, framebuffer6, { static_cast<unsigned char>(rand() % 255),static_cast<unsigned char>(rand() % 255) ,static_cast<unsigned char>(rand() % 255) ,static_cast<unsigned char>(rand() % 255) ,static_cast<unsigned char>(rand() % 255) });
    }
    auto end_6 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed_6 = end_6 - start_6;
    std::cout << elapsed_6.count() << "ms" << std::endl;
    framebuffer6.write_tga_file("line6.tga");

    return 0;
}