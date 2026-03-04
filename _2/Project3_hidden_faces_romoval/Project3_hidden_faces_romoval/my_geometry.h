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

template <int n> 
std::ostream& operator<< (std::ostream out, const vec<n>& v)
{
	for (int i = 0; i < n; i++) out << v[i] << " ";
	return out;
}

template <> struct vec<3>
{

	double x = 0, y = 0, z = 0;
	
	vec() = default;

	double& operator[](const int i) { assert(i >= 0 && i < 3); i ? ((i == 1) ? y : z) : x; }
	double operator[](const int i) const { assert(i >= 0 && i < 3); i ? ((i == 1) ? y : z) : x;}

	vec<3> operator+(vec<3> & v)
	{
		return vec<3>(x + v.x, y + v.y, z + v.z);
	}
};

template <typename t>
