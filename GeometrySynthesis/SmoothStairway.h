#pragma once

#include "SmoothStep.h"

class SmoothStairway
{
private:
	Vector<SmoothStep> step;
	Mesh * source;

	int numSteps;

public:
	SmoothStairway(){ numSteps = 0; source = NULL; };
	SmoothStairway(Mesh * sourceMesh, int numberSteps, int numIteration, float smoothStepSize, bool isVolumePreserve);
	
	Mesh * mostBaseMesh();
	Mesh * mostDetailedMesh();

	SmoothStep * mostBase();

	SmoothStep * getStep(int index);
	Mesh * getStepBase(int index);
	Mesh * getStepDetailed(int index);

	int numberOfSteps();

	float maxDisplacement();

	// VISUALIZATION
	void draw();

	bool isDrawDisplacment;
};
