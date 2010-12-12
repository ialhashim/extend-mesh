#include "SmoothStairway.h"

#include "SimpleDraw.h"

SmoothStairway::SmoothStairway(Mesh * sourceMesh, int numberSteps, int numIteration, float smoothStepSize, bool isVolumePreserve)
{
	this->source = sourceMesh;
	this->step.reserve(numberSteps);

	this->numSteps = numberSteps;

	// Optimization ?
	this->source->tempUmbrellas.reserve(source->numberOfVertices());
	this->source->getUmbrellas();

	// First step is special
	step.push_back( SmoothStep(source, smoothStepSize, numIteration, isVolumePreserve) );

	// Create steps
	for(int i = 0; i < numSteps - 1; i++)
	{
		step.push_back( SmoothStep ( step[i].baseMesh(), smoothStepSize, numIteration, isVolumePreserve) );
	}

	// Last base is visible
	mostBaseMesh()->isDrawSmooth = false;
	//mostBaseMesh()->setColor(0, 128, 128, 170);

	this->isDrawDisplacment = true;
}

Mesh * SmoothStairway::mostDetailedMesh()
{
	// Top most detailed mesh
	return step[0].detailedMesh();
}

Mesh * SmoothStairway::mostBaseMesh()
{
	// Last base mesh
	return step.back().baseMesh();
}

SmoothStep * SmoothStairway::mostBase()
{
	return &step[step.size()-1];
}

SmoothStep * SmoothStairway::getStep(int index)
{
	return &step[index];
}

Mesh * SmoothStairway::getStepBase(int index)
{
	return step[index].baseMesh();
}

Mesh * SmoothStairway::getStepDetailed(int index)
{
	return step[index].detailedMesh();
}

int SmoothStairway::numberOfSteps()
{
	return step.size();
}

float SmoothStairway::maxDisplacement()
{
	Mesh * detail = mostDetailedMesh();
	Mesh * base = mostBaseMesh();

	float maxDist = FLT_MIN;

	for(int i = 0; i < base->numberOfVertices(); i++)
	{
		maxDist = Max((base->v(i)->vec() - detail->v(i)->vec()).norm(), maxDist);
	}

	return maxDist;
}

void SmoothStairway::draw()
{
	Mesh * detail = mostDetailedMesh();
	Mesh * base = mostBaseMesh();

	// Draw steps
	for(int i = 0; i < numSteps; i++)
		step[i].draw();

	// Draw displacement vectors
	if(isDrawDisplacment)
	{
		for(int i = 0; i < base->numberOfVertices(); i++)
		{
			SimpleDraw::IdentifyArrow(base->v(i)->vec(), detail->v(i)->vec());
		}
	}

	// Draw last base mesh
	mostBaseMesh()->draw();
}
