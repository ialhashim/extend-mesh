#pragma once

#include "Mesh.h"
#include "Skeleton.h"
#include "Grid.h"

#include "Smoother.h"
#include "SmoothStairway.h"

#define NOT_SET FLT_MAX

class Displacements
{
private:
	Mesh * sourceMesh;
	Skeleton * skeleton;
	Vector<SkeletonEdge *> localSkeleton;

	SmoothStairway stair;

	Grid grid;

	bool isVisible;
	bool isDrawStair;
	bool isUserFriendly;

public:

	bool isReady;

	// CONSTRUCTORS
	Displacements(Mesh * source, Skeleton * srcSkeleton,
		int numberSteps, int numIteration, float smoothStepSize, bool isVolumePreserve);

	// ACCESSORS
	Grid * GetGrid();
	SmoothStairway * Stair();

	// MODIFIERS
	void computeField(int gridSquareSize, int fitMethod = 1, int lRotate = 0, int rRotate = 0);

	// VISUALIZATION
	void draw();
	void ToggleVisibility();
	void ToggleDisplacmentsVisibility();
	void setVisibility(bool visible);

	// SELECTION
	void drawPointNames();
};
