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
class BoundingBox
{

public:
	Vec center;
	double xExtent, yExtent, zExtent;

	BoundingBox();
	BoundingBox(const Vec& c, double x, double y, double z);
	BoundingBox& operator= (const BoundingBox& other);

	void computeFromTris(const Vector<BaseTriangle*>& tris);
	void computeFromTri(const Vec& v1, const Vec& v2, const Vec& v3);

	bool intersects(const Ray& ray) const;

	bool contains(const Vec& point) const;

	bool containsTriangle(const Vec& tv1, const Vec& tv2, const Vec& tv3) const;
	bool planeBoxOverlap(const Vec& normal, double d, const Vec& maxbox) const;
	bool intersectsBoundingBox(const BoundingBox& bb) const;
	bool intersectsSphere(const Vec& sphere_center, double radius);

	Vector<Vec> getCorners();
};
