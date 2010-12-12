#pragma once

#include "Vertex.h"
#include "Intersection.h"

#define MODIFIED_TRI 5

class Triangle
{
private:

public:
	Vec p[3];
	int index;
	int flag;

	Triangle(Vec point1 = Vec(), Vec point2 = Vec(), Vec point3 = Vec(), 
		int tri_index = -1, int tri_flag = -1) : index(tri_index), flag(tri_flag)
	{
		p[0] = point1;
		p[1] = point2;
		p[2] = point3;
	}

	inline Vec& vec(int i){ return p[i]; }

	inline float edgeLenY(int e) const;

	void unloop(float threshold, Triangle & other);

	Triangle shiftY(float offsetY);

	void intersectionTest(const Ray & ray, HitResult & res, bool allowBack = false);
};
