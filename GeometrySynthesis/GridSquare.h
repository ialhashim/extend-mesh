#pragma once

#include <vector>
#include "GridPoint.h"

struct SquareDimension
{
	SquareDimension(float u_size, float v_size) : U(u_size), V(v_size){}
	float U,V;
};

class GridSquare
{
private:

public:
	int id;

	int p[4];
	int face1, face2;

	int u, v;

	float height;

	std::vector<GridPoint> points;
	std::vector<int> pointIndices;

	GridSquare()
	{
		id = -1;
		p[0] = p[1] = p[2] = p[3] = -1;
		face1 = face2 = -1;
		u = v = -1;
		height = 0.0f;
	}

	GridSquare(int v1, int v2, int v3, int v4, int f1, int f2, int u, int v, int ID)
	{
		this->id = ID;

		this->p[0] = v1;
		this->p[1] = v2;
		this->p[2] = v3;
		this->p[3] = v4;

		this->face1 = f1;
		this->face2 = f2;

		this->u = u;
		this->v = v;

		this->height = 0;
	}

	GridSquare(const GridSquare& fromSquare)
	{
		this->id = fromSquare.id;

		this->p[0] = fromSquare.p[0];
		this->p[1] = fromSquare.p[1];
		this->p[2] = fromSquare.p[2];
		this->p[3] = fromSquare.p[3];

		this->face1 = fromSquare.face1;
		this->face2 = fromSquare.face2;

		this->u = fromSquare.u;
		this->v = fromSquare.v;

		this->points = fromSquare.points;
		this->pointIndices = fromSquare.pointIndices;

		this->height = fromSquare.height;
	}

	GridSquare& operator= (const GridSquare& fromSquare) 
	{
		this->id = fromSquare.id;

		this->p[0] = fromSquare.p[0];
		this->p[1] = fromSquare.p[1];
		this->p[2] = fromSquare.p[2];
		this->p[3] = fromSquare.p[3];

		this->face1 = fromSquare.face1;
		this->face2 = fromSquare.face2;

		this->u = fromSquare.u;
		this->v = fromSquare.v;

		this->points = fromSquare.points;
		this->pointIndices = fromSquare.pointIndices;

		this->height = fromSquare.height;

		return *this;
	}

	inline void insertPoint(GridPoint & p)
	{
		points.push_back(p);
		pointIndices.push_back(p.corrPoint);
	}

	inline GridPoint* getPoint(int index)
	{
		return &points[index];
	}

	inline void setUV(int toU, int toV)
	{
		this->u = toU;
		this->v = toV;
	}

	inline void setCorners(int v1, int v2, int v3, int v4)
	{
		this->p[0] = v1;
		this->p[1] = v2;
		this->p[2] = v3;
		this->p[3] = v4;
	}

	GridPoint * getPointCorr( int corrIndex );
	double * getWeights(int corrIndex);

	static std::vector<std::vector<double> > uniformSample(int size = 3);

};
