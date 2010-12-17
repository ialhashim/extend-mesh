#pragma once

struct HitResult{
	bool hit;
	double distance;

	double u;
	double v;
	int index;

        HitResult(bool isHit = false, double hitDistance = DBL_MAX) : hit(isHit), distance(hitDistance)
	{
                u = -1.0;
                v = -1.0;
		index = -1;
	}

	HitResult(const HitResult& other)
	{
		this->hit = other.hit;
		this->distance = other.distance;
		this->u = other.u;
		this->v = other.v;
		this->index = other.index;
	}

	HitResult& operator= (const HitResult& other)
	{
		this->hit = other.hit;
		this->distance = other.distance;
		this->u = other.u;
		this->v = other.v;
		this->index = other.index;

		return *this;
	}
};

struct Ray
{
	Vec origin;
	Vec direction;
	int index;

	Ray(const Vec & Origin = Vec(), const Vec & Direction = Vec(), int Index = -1) : origin(Origin), index(Index)
	{
		direction = Direction.unit();
	}

	inline Ray inverse() const { return Ray(origin, -direction); } 

	Ray& operator= (const Ray& other)
	{
		this->origin = other.origin;
		this->direction = other.direction;
		this->index = other.index;

		return *this;
	}
};

/* AABB-triangle overlap test code                      */
/* by Tomas Akenine-Möller                              */

#define X 0
#define Y 1
#define Z 2

#define FINDMINMAX(x0,x1,x2,min,max) \
	min = max = x0;   \
	if(x1<min) min=x1;\
	if(x1>max) max=x1;\
	if(x2<min) min=x2;\
	if(x2>max) max=x2;

/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			       	   \
	p2 = a*v2[Y] - b*v2[Z];			       	   \
	if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return false;

#define AXISTEST_X2(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			           \
	p1 = a*v1[Y] - b*v1[Z];			       	   \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return false;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p2 = -a*v2[X] + b*v2[Z];	       	       	   \
	if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return false;

#define AXISTEST_Y1(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p1 = -a*v1[X] + b*v1[Z];	     	       	   \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return false;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)			   \
	p1 = a*v1[X] - b*v1[Y];			           \
	p2 = a*v2[X] - b*v2[Y];			       	   \
	if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
	if(min>rad || max<-rad) return false;

#define AXISTEST_Z0(a, b, fa, fb)			   \
	p0 = a*v0[X] - b*v0[Y];				   \
	p1 = a*v1[X] - b*v1[Y];			           \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
	if(min>rad || max<-rad) return false;

#undef X
#undef Y
#undef Z

static inline void checkMinMax( Vec& vmin, Vec& vmax, const Vec& point )
{
	if (point.x < vmin.x)		vmin.x = point.x;
	else if (point.x > vmax.x)	vmax.x = point.x;

	if (point.y < vmin.y)		vmin.y = point.y;
	else if (point.y > vmax.y)	vmax.y = point.y;

	if (point.z < vmin.z)		vmin.z = point.z;
	else if (point.z > vmax.z)	vmax.z = point.z;
}
