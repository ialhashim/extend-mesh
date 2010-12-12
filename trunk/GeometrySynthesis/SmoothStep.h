#pragma once

#include "Mesh.h"
#include "Smoother.h"

#define NOT_SET FLT_MAX

class SmoothStep
{
private:
	Mesh * detailed;
	Mesh base;

	float smoothStep;
	int numIterations;

	Vector<Vec> height;
	Vector<Vec> shift;

	bool isVisible;

public:
	SmoothStep(Mesh * sourceMesh, float smoothStepSize, int numberIterations, bool isVolumePreserve);
	
	Mesh * detailedMesh();
	Mesh * baseMesh();

	float IntersectionSearch(Mesh * mesh, VertexDetail * vd, Ray & ray, HitResult & res, int flag, int depth = 0);

	// VISUALIZATION
	void draw();
};
