// Based on jMonkeyEngine, ported from java code (New BSD License)

#pragma once

#include "Mesh.h"
#include "Triangle.h"
#include "HashTable.h"

/**
* BoundingBox defines an axis-aligned cube that defines a
* container for a group of vertices of a particular piece of geometry. This box
* defines a center and extents from that center along the x, y and z axis.
*
* @author Joshua Slack
*/
template <typename FaceType>
class BoundingBox
{

public:
	Vec center;
	double xExtent, yExtent, zExtent;

	BoundingBox() 
	{ 
		this->center = Vec(FLT_MIN,FLT_MIN,FLT_MIN);
	}

	BoundingBox(const Vec& c, double x, double y, double z) 
	{
		this->center = c;

		this->xExtent = x;
		this->yExtent = y;
		this->zExtent = z;
	}

	BoundingBox& operator= (const BoundingBox& other) 
	{
		this->center = other.center;

		this->xExtent = other.xExtent;
		this->yExtent = other.yExtent;
		this->zExtent = other.zExtent;

		return *this;
	}

        void computeFromTris(const Vector<FaceType>& tris)
	{
		Vec vmin (FLT_MAX, FLT_MAX, FLT_MAX);
		Vec vmax (FLT_MIN, FLT_MIN, FLT_MIN);

		double minx, miny, minz, maxx, maxy, maxz;

                minx = maxx = tris[0]->vec(0).x;
                miny = maxy = tris[0]->vec(0).y;
                minz = maxz = tris[0]->vec(0).z;

                for (int i = 0; i < (int)tris.size(); i++)
		{
			for(int v = 0; v < 3; v++)
			{
                                Vec vec = tris[i]->vec(v);

				if (vec.x < minx) minx = vec.x;
				if (vec.x > maxx) maxx = vec.x;
				if (vec.y < miny) miny = vec.y;
				if (vec.y > maxy) maxy = vec.y;
				if (vec.z < minz) minz = vec.z;
				if (vec.z > maxz) maxz = vec.z;
			}
		}

		vmax = Vec(maxx, maxy, maxz);
		vmin = Vec(minx, miny, minz);
		
		center = (vmin + vmax) / 2.0;

		xExtent = vmax.x - center.x;
		yExtent = vmax.y - center.y;
		zExtent = vmax.z - center.z;
	}

	void computeFromTri(const Vec& v1, const Vec& v2, const Vec& v3) 
	{
		Vec vmin (FLT_MAX, FLT_MAX, FLT_MAX);
		Vec vmax (FLT_MIN, FLT_MIN, FLT_MIN);

		checkMinMax(vmin, vmax, v1);
		checkMinMax(vmin, vmax, v2);
		checkMinMax(vmin, vmax, v3);

		center = (vmin + vmax) / 2.0;

		xExtent = vmax.x - center.x;
		yExtent = vmax.y - center.y;
		zExtent = vmax.z - center.z;
	}

	Vector<Vec> getCorners()
	{
		Vector<Vec> corners;

		Vec x = (Vec(1,0,0) * xExtent);
		Vec y = (Vec(0,1,0) * yExtent);
		Vec z = (Vec(0,0,1) * zExtent);

		Vec c = center + x + y + z;

		corners.push_back(c);
		corners.push_back(c - (x*2));
		corners.push_back(c - (y*2));
		corners.push_back(c - (x*2) - (y*2));

		corners.push_back(corners[0] - (z*2));
		corners.push_back(corners[1] - (z*2));
		corners.push_back(corners[2] - (z*2));
		corners.push_back(corners[3] - (z*2));

		return corners;
	}

	bool intersects(const Ray& ray) const
	{
		double rhs;
		double fWdU[3];
		double fAWdU[3];
		double fDdU[3];
		double fADdU[3];
		double fAWxDdU[3];

		Vec UNIT_X(1.0, 0.0, 0.0);
		Vec UNIT_Y(0.0, 1.0, 0.0);
		Vec UNIT_Z(0.0, 0.0, 1.0);

		Vec diff = ray.origin - center;
		Vec wCrossD = ray.direction ^ diff;

		fWdU[0] = ray.direction * UNIT_X;
		fAWdU[0] = abs(fWdU[0]);
		fDdU[0] = diff * UNIT_X;
		fADdU[0] = abs(fDdU[0]);
		if (fADdU[0] > xExtent && fDdU[0] * fWdU[0] >= 0.0)		return false;

		fWdU[1] = ray.direction * UNIT_Y;
		fAWdU[1] = abs(fWdU[1]);
		fDdU[1] = diff * UNIT_Y;
		fADdU[1] = abs(fDdU[1]);
		if (fADdU[1] > yExtent && fDdU[1] * fWdU[1] >= 0.0)		return false;
		
		fWdU[2] = ray.direction * UNIT_Z;
		fAWdU[2] = abs(fWdU[2]);
		fDdU[2] = diff * UNIT_Z;
		fADdU[2] = abs(fDdU[2]);
		if (fADdU[2] > zExtent && fDdU[2] * fWdU[2] >= 0.0)		return false;
		
		fAWxDdU[0] = abs(wCrossD * UNIT_X);
		rhs = yExtent * fAWdU[2] + zExtent * fAWdU[1];
		if (fAWxDdU[0] > rhs)		return false;

		fAWxDdU[1] = abs(wCrossD * UNIT_Y);
		rhs = xExtent * fAWdU[2] + zExtent * fAWdU[0];
		if (fAWxDdU[1] > rhs)		return false;
		
		fAWxDdU[2] = abs(wCrossD * UNIT_Z);
		rhs = xExtent * fAWdU[1] + yExtent * fAWdU[0];
		if (fAWxDdU[2] > rhs)		return false;

		return true;
	}

        inline bool contains(const Vec& point) const
	{
		return abs(center.x - point.x) < xExtent 
			&& abs(center.y - point.y) < yExtent 
			&& abs(center.z - point.z) < zExtent;
	}

	bool planeBoxOverlap(const Vec& normal, double d, const Vec& maxbox) const
	{
		int q;
		Vec vmin, vmax;

		for(q = 0 ; q <= 2 ; q++)
		{
			if(normal[q]>0.0){
				vmin[q] = -maxbox[q];
				vmax[q] = maxbox[q];
			} else {
				vmin[q] = maxbox[q];
				vmax[q] = -maxbox[q];
			}
		}

		if((normal * vmin) + d > 0.0) return false;
		if((normal * vmax) + d >= 0.0) return true;

		return false;
	}

        bool containsTriangle(const Vec& tv1, const Vec& tv2, const Vec& tv3) const
        {
                if(contains(tv1) || contains(tv2) || contains(tv3))
                        return true;

                Vec v0, v1, v2;
                double min,max,d,p0,p1,p2,rad,fex,fey,fez;
                Vec normal,e0,e1,e2;

                Vec boxhalfsize = Vec(xExtent, yExtent, zExtent);
                int X = 0, Y = 1, Z = 2;

                v0 = tv1 - center;	v1 = tv2 - center;	v2 = tv3 - center;
                e0 = v1 - v0;		e1 = v2 - v1;		e2 = v0 - v2;

                /* Bullet 3:  */
                /*  test the 9 tests first (this was faster) */
                fex = abs(e0[X]); fey = abs(e0[Y]); fez = abs(e0[Z]);
                AXISTEST_X01(e0[Z], e0[Y], fez, fey);
                AXISTEST_Y02(e0[Z], e0[X], fez, fex);
                AXISTEST_Z12(e0[Y], e0[X], fey, fex);

                fex = abs(e1[X]); fey = abs(e1[Y]); fez = abs(e1[Z]);
                AXISTEST_X01(e1[Z], e1[Y], fez, fey);
                AXISTEST_Y02(e1[Z], e1[X], fez, fex);
                AXISTEST_Z0(e1[Y], e1[X], fey, fex);

                fex = abs(e2[X]); fey = abs(e2[Y]); fez = abs(e2[Z]);
                AXISTEST_X2(e2[Z], e2[Y], fez, fey);
                AXISTEST_Y1(e2[Z], e2[X], fez, fex);
                AXISTEST_Z12(e2[Y], e2[X], fey, fex);

                /* Bullet 1: */
                FINDMINMAX(v0.x,v1.x,v2.x, min, max);	/* test in X-direction */
                if(min > boxhalfsize.x || max < -boxhalfsize.x) return false;

                FINDMINMAX(v0.y,v1.y,v2.y, min, max);	/* test in Y-direction */
                if(min > boxhalfsize.y || max < -boxhalfsize.y) return false;

                FINDMINMAX(v0.z,v1.z,v2.z, min, max); 	/* test in Z-direction */
                if(min > boxhalfsize.z || max < -boxhalfsize.z) return false;

                /*  test if the box intersects the plane of the triangle */
                normal = e0 ^ e1;
                d = -(v0 * normal);  	/* plane eq: normal.x+d=0 */

                if(!planeBoxOverlap(normal, d, boxhalfsize))
                        return false;

                return true;
        }

	inline bool intersectsBoundingBox(const BoundingBox& bb) 
	{
        if (center.x + xExtent < bb.center.x - bb.xExtent || center.x - xExtent > bb.center.x + bb.xExtent)
            return false;
        else if (center.y + yExtent < bb.center.y - bb.yExtent || center.y - yExtent > bb.center.y + bb.yExtent)
            return false;
        else if (center.z + zExtent < bb.center.z - bb.zExtent || center.z - zExtent > bb.center.z + bb.zExtent)
            return false;
        else
            return true;
    }

	inline bool intersectsSphere(const Vec& sphere_center, double radius) 
	{
		if (abs(center.x - sphere_center.x) < radius + xExtent
			&& abs(center.y - sphere_center.y) < radius + yExtent
			&& abs(center.z - sphere_center.z) < radius + zExtent)
			return true;

		return false;
	}

private:
	void checkMinMax(Vec& vmin, Vec& vmax, const Vec& point) 
	{
		if (point.x < vmin.x)		vmin.x = point.x;
		else if (point.x > vmax.x)	vmax.x = point.x;

		if (point.y < vmin.y)		vmin.y = point.y;
		else if (point.y > vmax.y)	vmax.y = point.y;

		if (point.z < vmin.z)		vmin.z = point.z;
		else if (point.z > vmax.z)	vmax.z = point.z;
	}
};
