#pragma once

#include "Vertex.h"

struct SamplePoint{
	Vec pos,n;
	int index;

	int u,v;

	SamplePoint(){index = -1; u = v = -1;}
	SamplePoint(const Vec& position, const Vec& normal, int assignIndex = -1, int U = -1, int V = -1) 
		: pos(position), n(normal), index(assignIndex), u(U), v(V){}

	Ray toRay(){return Ray(pos, n);}
};

typedef SamplePoint SampleSeed;

/* U and V are barycentric coordinates of hit on face 'faceIndex' */
struct SampleHit{
	float u,v;
	int findex;
	
	double blendWeight;

	SampleHit(){u = v = findex = blendWeight = -1;}
	SampleHit(float U, float V, int faceIndex, float blendVal) 
		: u(U), v(V), findex(faceIndex), blendWeight(blendVal) {}
};
