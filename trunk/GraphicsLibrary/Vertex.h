#pragma once

#include "Macros.h"

// Vector library & operations
#include <QGLViewer/quaternion.h>
using namespace qglviewer;
#define Rotation qglviewer::Quaternion

class Vertex : public Vec
{
public:
	Vertex(){ x = y = z = 0; }

	Vertex(const Vec& from){
		x = from.x;
		y = from.y;
		z = from.z;
	}

	Vertex(const double& X, const double& Y, const double& Z){
		x = X;
		y = Y;
		z = Z;
	}

	Vertex& operator= (const Vertex& from){
		x = from.x;
		y = from.y;
		z = from.z;

		return *this;
	}

	Vertex& operator= (const Vec& from){
		x = from.x;
		y = from.y;
		z = from.z;

		return *this;
	}

	void set(const double& X, const double& Y, const double& Z){
		x = X;
		y = Y;
		z = Z;
	}

	void set(const Vec& from){
		x = from.x;
		y = from.y;
		z = from.z;
	}

	void set(const Vertex& from){
		x = from.x;
		y = from.y;
		z = from.z;
	}

	void add(const double& X, const double& Y, const double& Z){
		x += X;
		y += Y;
		z += Z;
	}

	int above(Vec & pos, Vec & dir){
		double sign = dir * (*this - pos);

		if(sign > 0)
			return 1;
		else if(sign < 0)
			return -1;
		else
			return 0;
	}

	static double cotangent(Vertex & v0, Vertex & v1){
        double f = (v0 ^ v1).norm();
        if(f == 0) return 0;
		return (v0 * v1) / f;
    }

	const Vec& vec(){
		return *this;
	}

	static Vec interpolateVec(double t, const Vec & from, const Vec & to)
	{
		if(t == 0.0)	return from;
		if(t == 1.0)	return to;

		return from + (t * (to - from));
	}

	static Vec MidVec(const Vec & a, const Vec & b)
	{
		return Vec((a.x + b.x) / 2.0, (a.y + b.y) / 2.0, (a.z + b.z) / 2.0);
	}

	static Vec MidVec(const Vector<Vec> & points)
	{
		int N = points.size();

		Vec sum;

		for(Vector<Vec>::const_iterator it = points.begin(); it != points.end(); it++)
			sum += *it;
		
		return sum / (double)N;
	}

	static double maxLength(Vertex &a, Vertex &b, Vertex &c)
	{
		return Max((a-b).norm(), Max((a-c).norm(), (b-c).norm()));
	}

	static double minLength(Vertex &a, Vertex &b, Vertex &c)
	{
		return Min((a-b).norm(), Min((a-c).norm(), (b-c).norm()));
	}

	static double totalLength(Vertex &a, Vertex &b, Vertex &c)
	{
		return (a-b).norm() + (a-c).norm() + (b-c).norm();
	}

	static Vec RotateAround(const Vec & point, const Vec & pivot, const Rotation & q)
	{
		Vec result = point;

		result -= pivot;
		result = q.rotate(result);
		result += pivot;

		return result;
	}

	static double angleBetweenTwo(const Vec &a, const Vec &b)
	{
		double Dot = (a).unit() * (b).unit();

		if(abs(Dot) >= 1.0)	
			Dot = 1.0; 

		return acos(Dot);
	}

	double angleBetween(const Vec &a, const Vec &b)
	{
		double Dot = (a - *this).unit() * (b - *this).unit();

		if(abs(Dot) >= 1.0)	
			Dot = 1.0; 

		return acos(Dot);
	}

	static double minAngle(const Vec &a, const Vec &b, const Vec &c)
	{
		return Min(Vertex(a).angleBetween(b,c), 
			Min(Vertex(b).angleBetween(c,a), 
			Vertex(c).angleBetween(a,b)));
	}

	static double maxAngle(const Vec &a, const Vec &b, const Vec &c)
	{
		return Max(Vertex(a).angleBetween(b,c), 
			Max(Vertex(b).angleBetween(c,a), 
			Vertex(c).angleBetween(a,b)));
	}

	static double halfAlphaTangent(const Vec &a, const Vec &b, const Vec &c)
	{
		double Dot = (a - b).unit() * (c - b).unit();
		
		Dot = RANGED(-1.0, Dot, 1.0); // bound

		return  tan(acos(Dot) / 2.0); // tan (angle / 2)
	}

	static double singed_angle(const Vec &a, const Vec &b, const Vec &axis)
	{
		double cosAngle = a.unit() * b.unit();
		double angle = acos( RANGED(-1.0, cosAngle, 1.0) );

		Vec c = a ^ b;

		if (c * axis < 0)
			return -angle;

		return angle;
	}

	static bool isBetween(const Vec& p, const Vec& a, const Vec& b, double Eps = 0.0)
	{
		double length = (a - b).norm();
		double dist = (p - a).norm() + (p - b).norm();

		if(dist > length + Eps)
			return false;
		else
			return true;
	}

	static bool isSameSide(const Vec& p1, const Vec& p2, const Vec& a, const Vec& b)
	{
		Vec cp1 = (b-a) ^ (p1-a);
		Vec cp2 = (b-a) ^ (p2-a);

		return (cp1 * cp2 >= 0);
	}

	StdString toString()
	{
		char buff[2048];

                sprintf(buff,"v %f %f %f", x, y, z);

		return buff;
	}

        void OrthogonalBasis( Vec &left, Vec &up ) const
        {
                float l, s;

                if ( fabs( z ) > 0.7f ) {
                        l = y * y + z * z;
                        s = 1.0f / sqrt ( l );
                        up[0] = 0;
                        up[1] = z * s;
                        up[2] = -y * s;
                        left[0] = l * s;
                        left[1] = -x * up[2];
                        left[2] = x * up[1];
                }
                else {
                        l = x * x + y * y;
                        s = 1.0f / sqrt ( l );
                        left[0] = -y * s;
                        left[1] = x * s;
                        left[2] = 0;
                        up[0] = -z * left[1];
                        up[1] = z * left[0];
                        up[2] = l * s;
                }
       }
};

typedef Vertex Normal;
typedef unsigned int Index ;

struct VecIndex
{
	Vec v;
	int index;

	VecIndex(){	index = -1; }
	VecIndex(Vec vec, int Index) : v(vec), index(Index){}
	operator const Vec() const {	return v;	}
};
