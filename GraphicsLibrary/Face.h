#ifndef FACE_H
#define FACE_H

#include "Vertex.h"
#include "Edge.h"
#include "Triangle.h"

enum FaceFlag{
	FF_CLEAR =  -1,
	FF_INVALID_VINDEX = 2
};

struct VertexAngle{
	int index;
	double angle;
	VertexAngle(int vertIndex, double angleAt) : index(vertIndex), angle(angleAt){};
};

class Face : public BaseTriangle
{
public:
	Vertex * v[3];
	int vIndex[3];

	Face(){v[0] = v[1] = v[2] = NULL; vIndex[0] = vIndex[1] = vIndex[2] = -1; index = -1; flag = -1;}
	Face(int vIndex1, int vIndex2, int vIndex3, Vertex * v1, Vertex * v2, Vertex * v3, int Index);
	Face(const Face& from);
	Face& operator= (const Face& from);

	// DIRECT VERTEX ACCESS
	inline Vertex * operator[]( int subscript ) const { return v[ subscript ]; }
	inline Vertex * & operator[]( int subscript ) { return v[ subscript ]; }
	inline Vertex * P(int subscript) { return v [ subscript ]; }

	// VERTEX ACCESS
	inline int VIndex(const int & i){ return vIndex[i]; }
	Vertex * PointIndexed(int i);
	int closestVertexIndex(Vertex * vertex);
	int LocalIndexPointIndexed(int vi);
	int oppositeVertex(Edge & e);
	int otherVertexIndex(int vi);
	int otherVertexIndex(int vi1, int vi2);
	bool hasPoint(int i);
	void replacePoint(int oldIndex, int newIndex);
	void unsetVertexByIndex(int vi);
	void unset();

	Vec vec(int i) const;

	// COMPUTE NORMAL
	Vec normal() const;

	// COMPUTE CENTER & FROM Barycentrics
	Vec center();
	Vec getBary(double U, double V);

	// EDGE OPERATIONS
	bool hasEdge(Edge & e);
	bool sharesEdge(Face & other);
	int sharesEdge(Face * other, int firstVertex);
	const EdgeSet edgesWithVertex(const int & vertex);
	double longestEdgeLength();
	Edge oppositeEdge(int vi);

	// AREA & VOLUME
	double area();
	static double FaceArea(Vec & A, Vec & B, Vec & C);
	double volume();

	// ANGLES
	double angle(int a);
	double angleCotangent(int a);
	double angleCotangent(Edge & e);
	VertexAngle largestAngle();

	Pair<Vec, Vec> spanAt(int a);

	// INTERSECTION FUNCTIONS
	void intersectionTest(const Ray & ray, HitResult & res, bool allowBack = false) const;
	void intersectionTest2(const Ray & ray, HitResult & res);

	double pointToPointsDistance(Vec & p);

	StdString toString();
};

#define PairFaces Pair<Face *, Face *>

#endif // FACE_H
