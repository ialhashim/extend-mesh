#include "Face.h"

Face::Face(int vIndex1, int vIndex2, int vIndex3, Vertex * v1, Vertex * v2, Vertex * v3, int Index) 
{
	vIndex[0] = vIndex1;
	vIndex[1] = vIndex2;
	vIndex[2] = vIndex3;

	v[0] = v1;
	v[1] = v2;
	v[2] = v3;

	index = Index;

	flag = FF_CLEAR;
}

Face::Face(const Face& from)
{
	vIndex[0] = from.vIndex[0];
	vIndex[1] = from.vIndex[1];
	vIndex[2] = from.vIndex[2];
	v[0] = from.v[0];
	v[1] = from.v[1];
	v[2] = from.v[2];
	index = from.index;
	flag = from.flag;
}

Face& Face::operator= (const Face& from)
{
	if (this != &from) {
		vIndex[0] = from.vIndex[0];
		vIndex[1] = from.vIndex[1];
		vIndex[2] = from.vIndex[2];
		v[0] = from.v[0];
		v[1] = from.v[1];
		v[2] = from.v[2];
		index = from.index;
		flag = from.flag;
	}
	return *this;
}

Vec Face::vec(const int i) const
{
	return *v[i];
}

Vec Face::normal()
{
	Vec n = (*v[1] - *v[0]) ^ (*v[2] - *v[0]);

	double length = n.norm();

	if(length < 1.0E-10)
		return n;
	else
		return n /= length;
}

Vec Face::center()
{
	Vec c;

	c.x = (v[0]->x + v[1]->x + v[2]->x) / 3.0;
	c.y = (v[0]->y + v[1]->y + v[2]->y) / 3.0;
	c.z = (v[0]->z + v[1]->z + v[2]->z) / 3.0;

	return c;
}

Vec Face::getBary(double U, double V)
{
	if(U == 1.0) return *v[1];
	if(V == 1.0) return *v[2];

	double b1 = U;
	double b2 = V;
	double b3 = 1.0 - (U + V);

	Vec p, V1 = *v[1], V2 = *v[2], V3 = *v[0];

	p.x = (b1 * V1.x) + (b2 * V2.x) + (b3 * V3.x);
	p.y = (b1 * V1.y) + (b2 * V2.y) + (b3 * V3.y);
	p.z = (b1 * V1.z) + (b2 * V2.z) + (b3 * V3.z);

	return p;
}

const EdgeSet Face::edgesWithVertex(const int & vertex)
{
	EdgeSet eSet;

	if (vertex == vIndex[0]){
		eSet.insert(Edge(vertex, vIndex[1]));
		eSet.insert(Edge(vertex, vIndex[2]));
	}
	else if (vertex == vIndex[1]){
		eSet.insert(Edge(vertex, vIndex[0]));
		eSet.insert(Edge(vertex, vIndex[2]));
	}
	else if (vertex == vIndex[2]){
		eSet.insert(Edge(vertex, vIndex[0]));
		eSet.insert(Edge(vertex, vIndex[1]));
	}

	return eSet;
}

bool Face::hasEdge(Edge & e)
{
	int count = 0;

	for(int i = 0; i < 3; ++i)
	{
		if(e.vIndex[0] == vIndex[i] || e.vIndex[1] == vIndex[i])
			count++;
	}

	return (count > 1);
}

bool Face::sharesEdge(Face & other)
{
	int count = 0;

	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			if(vIndex[i] == other.vIndex[j])
			{
				if(++count >= 2) 
					return true;
			}
		}
	}

	return false;
}

int Face::sharesEdge(Face * other, int firstVertex)
{
	int count = 0;
	int edgeIndices[4] = {-1, -1, -1, -1};

	for(int i = 0; i < 3; ++i){
		for(int j = 0; j < 3; ++j){
			if(vIndex[i] == other->vIndex[j])
					edgeIndices [count++] = vIndex[i];
		}
	}

	if(count >= 2){
		if(firstVertex == edgeIndices[0])
				return edgeIndices[1];
		else
				return edgeIndices[0];
	}

	return -1;
}

int Face::oppositeVertex(Edge & e)
{
	if(e[0] == vIndex[0])		
		return (e[1] == vIndex[1]) ? vIndex[2] : vIndex[1];
	else if(e[0] == vIndex[1])	
		return (e[1] == vIndex[0]) ? vIndex[2] : vIndex[0];
	else	
		return (e[1] == vIndex[0]) ? vIndex[1] : vIndex[0];
}

int Face::otherVertexIndex(int vi)
{
	if		(vIndex[0] == vi)		return vIndex[1];
	else if	(vIndex[1] == vi)		return vIndex[0];
	else if	(vIndex[2] == vi)		return vIndex[1];
	else return -1;	
}

int Face::otherVertexIndex(int vi1, int vi2)
{
    Edge e (vi1, vi2);
        return oppositeVertex(e);
}

Vertex * Face::PointIndexed(int i)
{
	if		(vIndex[0] == i)		return v[0];
	else if	(vIndex[1] == i)		return v[1];
	else if	(vIndex[2] == i)		return v[2];
	else return NULL;
}

int Face::closestVertexIndex(Vertex * vertex)
{
	double minDist = FLT_MAX;
	int closest = 0;
	
	for(int i = 0; i < 3; i++)
	{
		double dist = (*v[i] - *vertex).norm();

		if(dist < minDist)
		{
			minDist = dist;
			closest = i;
		}
	}

	return vIndex[closest];
}

int Face::LocalIndexPointIndexed(int vi)
{
	if		(vIndex[0] == vi)		return 0;
	else if	(vIndex[1] == vi)		return 1;
	else if	(vIndex[2] == vi)		return 2;
	else return -1;
}

bool Face::hasPoint(int i)
{
	if(vIndex[0] == i || vIndex[1] == i || vIndex[2] == i) return true;
	else return false;
}

void Face::replacePoint(int oldIndex, int newIndex)
{
	if(vIndex[0] == oldIndex) vIndex[0] = newIndex;
	else if(vIndex[1] == oldIndex) vIndex[1] = newIndex;
	else if(vIndex[2] == oldIndex) vIndex[2] = newIndex;
}

void Face::unsetVertexByIndex(int vi)
{
	int i = LocalIndexPointIndexed(vi);

	if(i >= 0)
	{
		vIndex[i] = -1;
		v[i] = NULL;
		flag = FF_INVALID_VINDEX;
	}
}

void Face::unset()
{
	vIndex[0] = -1;
	vIndex[1] = -1;
	vIndex[2] = -1;

	v[0] = v[1] = v[2] = NULL;
	
	flag = FF_INVALID_VINDEX;
}

double Face::longestEdgeLength()
{
	double len1 = (*v[0] - *v[1]).norm();
	double len2 = (*v[1] - *v[2]).norm();
	double len3 = (*v[2] - *v[0]).norm();

	return Max(len1, Max(len2, len3));
}

Edge Face::oppositeEdge(int vi)
{
	if		(vIndex[0] == vi)	return Edge(vIndex[1],vIndex[2]);
	else if	(vIndex[1] == vi)	return Edge(vIndex[2],vIndex[0]);
	else if	(vIndex[2] == vi)	return Edge(vIndex[0],vIndex[1]);
	else return Edge(-1,-1); // should not happen
}

double Face::volume()
{
	Vertex p1 = *v[0];
	Vertex p2 = *v[1];
	Vertex p3 = *v[2];

	Vec g = (p1 + p2 + p3) / 3.0;
	Vec n = (p2 - p1) ^ (p3 - p1);

	return (g[0]*n[0] + g[1]*n[1] + g[2]*n[2]) / 6.0;
}

double Face::area()
{
	// given vector t = (B-A) x (C-A), then area = 0.5 * t.Length();
	Vec t = (*v[1] - *v[0]) ^ (*v[2] - *v[0]);
	return 0.5 * t.norm();
}

double Face::FaceArea(Vec & A, Vec & B, Vec & C)
{
	Vec t = (B - A) ^ (C - A);
	return 0.5f * t.norm();
}

double Face::angle(int a)
{
	int b = 1, c = 2;

	if (a == 1)			b = 0;
	else if (a == 2)	c = 0;
	
	Vertex v1 = *v[b] - *v[a];
	Vertex v2 = *v[c] - *v[a];

	double Dot = v1.unit() * v2.unit();

	if(abs(Dot) >= 1.0)	
		Dot = 1.0; 

	return acos(Dot);
}

double Face::angleCotangent(int a)
{
	double tangent = tan(this->angle(a));

	if(tangent == 0.0)	
		return 10000.0; // is this correct?
	else
		return 1.0 / tangent;
}

double Face::angleCotangent(Edge & e)
{
	int v = oppositeVertex(e);
	if		(v == vIndex[0])	return angleCotangent(0);
	else if	(v == vIndex[1])	return angleCotangent(1);
	else						return angleCotangent(2);
}

VertexAngle Face::largestAngle()
{
	double angle0 = angle(0);
	double angle1 = angle(1);
	double angle2 = angle(2);

	double maxAngle = Max(angle0, Max(angle1, angle2));

	if(angle0 == maxAngle) 
		return VertexAngle(vIndex[0], maxAngle);
	else if(angle1 == maxAngle)
		return VertexAngle(vIndex[1], maxAngle);
	else
		return VertexAngle(vIndex[2], maxAngle);
}

Pair<Vec, Vec> Face::spanAt(int a)
{
	int b = 1;
	int c = 2;
	if (a == 1)	b = 0;
	if (a == 2)	c = 0;
	
	Vertex v1 = *v[b] - *v[a];
	Vertex v2 = *v[c] - *v[a];

	v1.normalize();
	v2.normalize();

	return Pair<Vec,Vec>(v1, v2);
}

double Face::pointToPointsDistance(Vec & p)
{
	double a = (p - *v[0]).norm();
	double b = (p - *v[1]).norm();
	double c = (p - *v[2]).norm();
	return Min(a,Min(b,c));
}

StdString Face::toString()
{
	char buff[2048];
	sprintf(buff,"f %d %d %d", vIndex[0] + 1, vIndex[1] + 1, vIndex[2] + 1);
	return buff;
}

void Face::intersectionTest(const Ray & ray, HitResult & res, bool allowBack)
{
	res.hit = false;
	res.distance = FLT_MAX;

	double EPS = 0.0001;

	Vec vertex1 = *v[0];
	Vec vertex2 = *v[1];
	Vec vertex3 = *v[2];

	// Compute vectors along two edges of the triangle.
	Vec edge1 = vertex2 - vertex1;
	Vec	edge2 = vertex3 - vertex1;

	// Compute the determinant.
	Vec directionCrossEdge2 = ray.direction ^ edge2;

	double determinant = edge1 * directionCrossEdge2;

	// If the ray is parallel to the triangle plane, there is no collision.
	if (abs(determinant) < Epsilon)
		return;

	double inverseDeterminant = 1.0 / determinant;

	// Calculate the U parameter of the intersection point.
	Vec distanceVector = ray.origin - vertex1;
	double triangleU = distanceVector * directionCrossEdge2;
	triangleU *= inverseDeterminant;

	// Make sure it is inside the triangle.
	if (triangleU < 0 - EPS || triangleU > 1 + EPS)
		return;

	// Calculate the V parameter of the intersection point.
	Vec distanceCrossEdge1 = distanceVector ^ edge1;
	double triangleV = ray.direction * distanceCrossEdge1;
	triangleV *= inverseDeterminant;

	// Make sure it is inside the triangle.
	if (triangleV < 0 - EPS || triangleU + triangleV > 1 + EPS)
		return;

	// Compute the distance along the ray to the triangle.
	double rayDistance = edge2 * distanceCrossEdge1;
	rayDistance *= inverseDeterminant;

	if(!allowBack)
	{
		// Is the triangle behind the ray origin?
		if (rayDistance < 0)
			return;
	}

	res.hit = true;
	res.distance = rayDistance;

	res.u = triangleU;
	res.v = triangleV;

	res.index = this->index;
}

// Based on http://www.gamedev.net/community/forums/topic.asp?topic_id=447861
void Face::intersectionTest2(const Ray & ray, HitResult & res)
{
	// structure for holding the result of the test
	res.distance = FLT_MAX;
	res.hit = false;
	
	Vec edge1(v[1]->x - v[0]->x, v[1]->y - v[0]->y, v[1]->z - v[0]->z); //  v1 - v0;
	Vec edge2(v[2]->x - v[0]->x, v[2]->y - v[0]->y, v[2]->z - v[0]->z); //  v2 - v0;
	
	// Find the cross product of edge2 and the ray direction
	Vec s1(	ray.direction.y*edge2.z - ray.direction.z*edge2.y, 
			ray.direction.z*edge2.x - ray.direction.x*edge2.z, 
			ray.direction.x*edge2.y - ray.direction.y*edge2.x); //ray.direction ^ edge2;
	
	// Find the divisor, if its zero, return false as the triangle is degenerated
	double divisor = s1 * edge1;
	if (divisor == 0.0)
		return;
	
	// A inverted divisor, as multiplying is faster then division
	double invDivisor = 1.0 / divisor;
	
	// Calculate the first barycentric coordinate. barycentric coordinates are between 0.0 and 1.0
	Vec distance (ray.origin.x - v[0]->x, ray.origin.y - v[0]->y, ray.origin.z - v[0]->z);
	double barycCoord_1 = (distance * s1) * invDivisor;
	if (barycCoord_1 < 0.0 || barycCoord_1 > 1.0)
		return;
	
	// Calculate the second barycentric coordinate
	Vec s2 = distance ^ edge1;
	double barycCoord_2 = (ray.direction * s2) * invDivisor;
	if (barycCoord_2 < 0.0 || (barycCoord_1 + barycCoord_2) > 1.0)
		return;
	
	// After doing the barycentric coordinate test we know if the ray hits or 
	// not. If we got this far the ray hits. 

	res.hit = true;
	res.distance = (edge2 * s2) * invDivisor;  // Calculate the distance to the intersection point 
	res.index = this->index;
}
