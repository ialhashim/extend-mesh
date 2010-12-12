#pragma once

#include "MeshPatch.h"
class GridMesh;

#include "Sampling.h"

class Reconstructor
{
private:
	GridMesh * gm;
	Vector<MeshPatch> * patch;
	int numPatches;

	Vector<SamplePoint> samples;
	Vector<SampleSeed> seeds;

	void Sample();
	void Triangulate();
	void Combine();

	bool isDone;

	Vec PointOnUnitSquare(const Vector<double>& w);

	// Debug:
	Vector<Vec> testPoints;
        Vector<Vector< Vector<Vec> > > patches;

public:
	Reconstructor(GridMesh * srcGridMesh);

	void draw();

};
