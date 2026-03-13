#include "our_gl.h"

mat<4, 4> ModelView, Viewport, Perspective;
std::vector<double> zbuffer;

void init_modelview(const vec3 eye, const vec3 center, const vec3 up) {
	vec3 n = normalized(eye - center);
	vec3 l = normalized(cross(up, n));
	vec3 m = normalized(cross(n, l));
	ModelView = mat<4, 4>{ {{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}} } *
		mat<4, 4>{{{1, 0, 0, -center.x}, { 0,1,0,-center.y }, { 0,0,1,-center.z }, { 0,0,0,1 }}};
}

void init_perspective(const double f) {
	Perspective = { {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1 / f,1}} };
}

void init_viewport(const int x, const int y, const int w, const int h) {
	Viewport = { {{w / 2., 0, 0, x + w / 2.}, {0, h / 2., 0, y + h / 2.}, {0,0,255. / 2.0 , 255. / 2.0}, {0,0,0,1}} };
}

void init_zbuffer(const int width, const int height)
{
	zbuffer = std::vector(width * height, -1000.);
}

float triangle_area(int xa, int  ya, int xb, int yb, int xc, int yc)
{
	return .5 * ((yb - ya) * (xb + xa) + (ya - yc) * (xa + xc) + (yc - yb) * (xc + xb));
}

void rasterize(const Triangle& clip, const IShader& shader, TGAImage& framebuffer)
{
	vec4 ndc[3] = { clip[0] / clip[0].w, clip[1] / clip[1].w , clip[2] / clip[2].w };
	vec2 screen[3] = { (Viewport * ndc[0]).xy(),(Viewport * ndc[1]).xy(),(Viewport * ndc[2]).xy() };

	mat<3, 3> ABC = { {
		{screen[0].x,screen[0].y,1},
		{screen[1].x,screen[1].y,1},
		{screen[2].x,screen[2].y,1},
		} };
	if (ABC.det() < 1) return; // 矩阵行列式

	auto [bbminx, bbmaxx] = std::minmax({ screen[0].x , screen[1].x, screen[2].x});
	auto [bbminy, bbmaxy] = std::minmax({ screen[0].y , screen[1].y, screen[2].y });
#pragma omp parallel for
	for (int x = std::max<int>(bbminx, 0); x <= std::min<int>(bbmaxx, framebuffer.width() - 1);x++)
		for (int y = std::max<int>(bbminy, 0); x <= std::min<int>(bbmaxy, framebuffer.height() - 1); y++)
		{
			vec3 bc = ABC.invert_transpose() * vec3 { static_cast<double>(x), static_cast<double>(y), 1. };


		}


	//int boxmin_x = clip[0].x;
	//for (int i = 1; i < 3; i++)
	//	boxmin_x = std::min(boxmin_x, static_cast<int>(clip[i].x));

	//int boxmin_y = clip[0].y;
	//for (int i = 1; i < 3; i++)
	//	boxmin_y = std::min(boxmin_y, static_cast<int>(clip[i].y));

	//int boxmax_x = clip[0].x;
	//for (int i = 1; i < 3; i++)
	//	boxmax_x = std::max(boxmax_x, static_cast<int>(clip[i].x));

	//int boxmax_y = clip[0].y;
	//for (int i = 1; i < 3; i++)
	//	boxmax_y = std::max(boxmax_y, static_cast<int>(clip[i].y));

	//int xa = clip[0].x, ya = clip[0].y, za = clip[0].z,
	//	xb = clip[1].x, yb = clip[1].y, zb = clip[1].z,
	//	xc = clip[2].x, yc = clip[2].y, zc = clip[2].z;

	//double total_area = triangle_area(xa, ya, xb, yb, xc, yc);
	//if (total_area < 0.5)
	//	return;
	////#pragma omp parallel for
	//for (int x = boxmin_x; x <= boxmax_x; x++)
	//{
	//	for (int y = boxmin_y; y <= boxmax_y; y++)
	//	{
	//		double alpha = triangle_area(xa, ya, xb, yb, x, y) / total_area;
	//		double beta = triangle_area(xb, yb, xc, yc, x, y) / total_area;
	//		double gamma = triangle_area(xc, yc, xa, ya, x, y) / total_area;
	//		if (alpha < 0 || beta < 0 || gamma < 0)
	//			continue;

	//		//if (alpha < 0.1 || beta < 0.1 || gamma < 0.1)
	//		{
	//			double z_double = zc * alpha + za * beta + zb * gamma;
	//			double b = gamma * 255.0;
	//			double g = beta * 255.0;
	//			double r = alpha * 255.0;
	//			unsigned char z = static_cast<unsigned char>(z_double);
	//			shader.color = { static_cast<unsigned char>(b),static_cast<unsigned char>(g),static_cast<unsigned char>(r),255 };
	//			if (z <= framebuffer.get(x, y)[0]) continue; //记录(x,y)像素点上的最大深度
	//			framebuffer.set(x, y, { z });
	//		}
	//	}
	//}
}