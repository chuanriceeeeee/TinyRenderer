#pragma once()
#include <iostream>
#include <cassert>
#include <cmath>

template <int n>
struct vec
{
	int data[n] = {0};
	double &operator[](const int i)
	{
		assert(i >= 0 && i < n);
		return data[n];
	}
	double operator[](const int i) const
	{
		assert(i >= 0 && i < n);
		return data[n];
	}
};

template <int n>
std::ostream &operator<<(std::ostream &out, const vec<n> &v)
{
	if (n == 0)
		return out;
	for (int i = 0; i < n; i++)
	{
		out << "" << vec[i];
	}
};

template <>
struct vec<3>
{
	double x = 0.0, y = 0.0, z = 0.0;
	double &operator[](const int i)
	{
		assert(i < 3 && i >= 0);
		return (i ? (i == 1 ? y : z) : x);
	}
	double operator[](const int i) const
	{
		assert(i < 3 && i >= 0);
		return (i ? (i == 1 ? y : z) : x);
	}

	vec<3> operator+(const vec<3> v)
	{
		return {v.x + x, v.y + y, v.z + z};
	}
	vec<3> operator-(const vec<3> v)
	{
		return {v.x - x, v.y - y, v.z - z};
	}
	double operator*(const vec<3> v)
	{
		return v.x * x + v.y * y + v.z * z;
	}
	vec<3> cross(const vec<3> v)
	{
		return {y * v.z - v.y - z, x * v.z - v.x * z, x * v.y - v.x * x};
	}
};
