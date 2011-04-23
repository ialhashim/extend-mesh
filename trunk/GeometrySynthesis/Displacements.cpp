#include "ExtendMeshHeaders.h"
#include "Displacements.h"

#include "SimpleDraw.h"

Displacements::Displacements(Mesh * source, Skeleton * srcSkeleton, int numberSteps, 
							 int numIteration, float smoothStepSize, bool isVolumePreserve)
{
	this->sourceMesh = source;
	this->skeleton = srcSkeleton;

	CreateTimer(timer);

	stats["smoothing"] = Stats("Base extraction (smoothing)");

	// Create smooth stairway structure
	stair = SmoothStairway(source, numberSteps, numIteration, smoothStepSize, isVolumePreserve);

	stats["smoothing"].end();

	printf("\n\n\nSmooth stairway time (%d ms).\n", (int)timer.elapsed());

	this->isVisible = true;
	this->isUserFriendly = false;
	this->isDrawStair = false;

	this->isReady = false;
}

void Displacements::computeField(int gridSquareSize, int fitMethod, int lRotate, int rRotate)
{
	CreateTimer(allTimer);
	printf("\nComputing field for selection..\n");

	// Get faces and points of selected part
	Vector<int> selectedFaces = skeleton->getSelectedFaces(true);

	StdList<Face*> meshFaces = stair.mostBaseMesh()->getFacesFromIndices(selectedFaces);
	Vector<int> points = SET_TO_VECTOR(stair.mostBaseMesh()->getVerticesFromFaces(selectedFaces));

	// Convert selected skeleton part into points in space
	Vector<Vec> skeletonPoints = skeleton->getSelectedSkeletonPoints();

	// Make sure we have a smooth and nicely sampled spine
	while((int)skeletonPoints.size() < gridSquareSize)
	{
		skeleton->smoothSelectedEdges(4);
		skeletonPoints = skeleton->getSelectedSkeletonPoints();
	}

	// Compute length by traveling spine
	float gridLength = 0;
	for(int i = 0; i < (int)skeletonPoints.size() - 1; i++)
		gridLength += Vec(skeletonPoints[i] - skeletonPoints[i+1]).norm();

	// Figure out maximum point on base surface to use as radius (there might be a faster way)
	HistogramFloat histogram (1);
	skeleton->sampleProjectPoints(0.75f, stair.mostBaseMesh(), points, &histogram);

	float radius = histogram.Average();

	if(radius <= 0.0f)	
		radius = stair.mostDetailedMesh()->radius * 0.1; // just in case...

	CreateTimer(gridTimer);

	// CREATE GRID
	grid = Grid(skeletonPoints, radius, gridLength, gridSquareSize, &stair, meshFaces,lRotate, rRotate);

	printf(".Grid time (%d ms).", (int)gridTimer.elapsed());
	CreateTimer(fitTimer);

	// FIT GRID
	switch(fitMethod)
	{
	case 1:	// Fit using cross sections
		grid.FitCrossSections();	break;

	case 2:	// Fit cylinder
		grid.FitCylinder( );		break;

	default: 
		grid.FitNothing( );             break;
	}

	printf(".Fit time (%d ms).", (int)fitTimer.elapsed());
	CreateTimer(gridifyTimer);

	// Assign detailed mesh points into cylindrical grid cells
	grid.Gridify( selectedFaces );

	isReady = true;

	printf(".total Gridify time = %d ms\n", (int)gridifyTimer.elapsed());

	printf("\n\nField time = %d ms\n=======\n", (int)allTimer.elapsed());
}

Grid * Displacements::GetGrid()
{
	return &grid;
}

SmoothStairway * Displacements::Stair()
{
	return &stair;
}

void Displacements::draw()
{
	if(isDrawStair)
		stair.draw();

	if(isReady && isVisible)
	{
		/*if(isUserFriendly)
		{
		stair->mostDetailedMesh()->draw();
		}
		else*/
		{
			// if(grid.isReady)
			{
				grid.drawAsGrid();

				grid.draw();
			}
		}
	}
}

void Displacements::drawPointNames()
{
	if(this->isReady)
		grid.drawPointNames();
}

void Displacements::ToggleVisibility()
{
	this->isVisible = !this->isVisible;
}

void Displacements::ToggleDisplacmentsVisibility()
{
	this->isDrawStair = !this->isDrawStair;
}

void Displacements::setVisibility(bool visible)
{
	this->isVisible = visible;
}
