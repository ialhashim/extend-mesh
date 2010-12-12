#pragma once

#include "MeshPatch.h"
#include "Seam.h"

struct CurveSample
{
	ExtendedPolygon A, B;
	CurveSample(const ExtendedPolygon& a, const ExtendedPolygon& b) : A(a), B(b){};
};

class Blender
{
private:
	GridMesh * gm;
	Vector<MeshPatch> * tri_patch;

	Mesh M; // working mesh

	Vector<Mesh> reconPatch;

	BezierSpline spline;

	Vector<Seam> seams;
	Vector<SeamTime> seamTime;
	Vector<int> comulativeNumV;

        Vector<Vector<CurveSample> > curve_samples;
        Vector<Vector<ExtendedPolygon> > blended_curves;

	void FindSeamTimes();

	void Sample();

	void Reconstruct();

	void Export();

public:
	Blender(GridMesh * gm);

	void draw();
	bool isDone;
};
