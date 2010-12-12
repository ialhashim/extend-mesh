#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct Point
{
public:
	int x;
	int y;

	Point(){x = y = 0;}
	Point(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
	Point(const Point& fromPoint)
	{
		this->x = fromPoint.x;
		this->y = fromPoint.y;
	}

	friend bool operator< (const Point &a, const Point &b)
	{
		return (a.x < b.x) || ( (a.x == b.x) && (a.y < b.y) );
	}

	Point operator* (const int scale)
	{
		Point result(*this);
		result.x *= scale;
		result.y *= scale;
		return result;
	}

	Point operator+ (const Point delta)
	{
		Point result(*this);
		result.x += delta.x;
		result.y += delta.y;
		return result;
	}

	Point& operator= (const Point& fromPoint) 
	{
		this->x = fromPoint.x;
		this->y = fromPoint.y;
		return *this;
	}

	inline bool operator== (const Point& p) const
	{
		return (this->x == p.x && this->y == p.y);
	}

	inline void hash_combine(size_t& seed, int v) const
	{
		seed ^= static_cast<size_t>(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
	}

	operator size_t() const
	{
		size_t seed = 0;
		hash_combine(seed, x);
		hash_combine(seed, y);
		return seed;
        }

	inline void add(int deltaX, int deltaY)
	{
		x += deltaX;
		y += deltaY;
	}

	inline double distanceTo(const Point& otherPoint)
	{
		return sqrt( pow(2.0, x - otherPoint.x) + pow(2.0, y - otherPoint.y) );
	}

	inline double DistSq() const { return x*x + y*y; }
};

#define Vector2DPoint std::vector<std::vector<Point> >

struct Rect
{
	int l,r;
	int t,b;

	Rect(int Left, int Right, int Top, int Bottom)
	{l = Left; r = Right; t = Top; b = Bottom;}
	Rect(){l = r = t = b = 0;}

	int width() const{return abs(r - l);}
	int height() const{return abs(b - t);}
};
