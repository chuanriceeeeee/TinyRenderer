#pragma once
#include "tgaimage.h"
#include "geometry.h"

void line(const int xa, const int ya, const int xb, const int yb, TGAImage& framebuffer, TGAColor color);
void rasterize()