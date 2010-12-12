#pragma once

#include "Vertex.h"

struct SimpleSquare
{
	int u, v;
	int src_u, src_v;

	int parentPatch;

	enum Flag {none = -1, border = 2, special = 4} flag;

	SimpleSquare(){u = v = src_u = src_v = -1; flag = none;};
	SimpleSquare(int U, int V, int srcU, int srcV, SimpleSquare::Flag f = none): 
	u(U), v(V), src_u(srcU), src_v(srcV), flag(f){ parentPatch = -1; }

	inline bool isEmpty() const {return u == -1;}
	inline float value() const {if (u == -1) return 0; else return 1.0f;}

	static Vector<Vector<float> > toVector2D(const Vector<Vector<SimpleSquare> > & patch)
	{
		Vector<Vector<float> > result(patch.size(), Vector<float>(patch[0].size()));
                for(unsigned int i = 0; i < patch.size(); i++)
                        for(unsigned int j = 0; j < patch[0].size(); j++)
				result[i][j] = patch[i][j].value();
		return result;
	};
};

struct SimpleTriangle
{
	int v1, v2, v3, index;

	SimpleTriangle(int V1, int V2, int V3, int Index) : v1(V1), v2(V2), v3(V3), index(Index){};
};
