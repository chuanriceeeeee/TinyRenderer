#pragma once
#include <iostream>
#include <cassert>
#include <cmath>

template <int n>
struct vec
{
	int data[n] = { 0 };
	double& operator[](const int i) { assert(i >= 0 && i < n); return data[n]; }
	double operator[](const int i) const { assert(i >= 0 && i < n); return data[n]; }
};

