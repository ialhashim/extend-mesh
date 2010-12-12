#include "SmoothStep.h"

#include "SimpleDraw.h"

SmoothStep::SmoothStep(Mesh * sourceMesh, float smoothStepSize, int numberIterations, bool isVolumePreserve)
{
	this->detailed = sourceMesh;
	this->smoothStep = smoothStepSize;
	this->numIterations = numberIterations;

	// Make a copy for the source mesh
	this->base = Mesh(*sourceMesh);
	this->base.id = this->base.id + "_base";

	// Perform smoothing
	Smoother::MeanCurvatureFlow(&this->base, smoothStepSize, numIterations, isVolumePreserve);

	int numVertices = detailed->numberOfVertices();

	// Compute displacement vectors
	Vec rayOrigin, rayDirection;

	HitResult res;
	Ray ray;

	// Prepare mesh (hight change statistics)
	/*Vector<float> distance(numVertices, NOT_SET);
	base.clearAllFaceFlag();

	for(int i = 0; i < numVertices; i++)
	{
		ray.origin = detailed->v(i)->vec();
		ray.direction = -base.n(i)->vec();

		VertexDetail * vd = base.vd(i);

		// using Breadth-first search (for some depth)
		distance[i] = IntersectionSearch(&base, vd, ray, res, vd->index);

		if(distance[i] == NOT_SET)
			distance[i] = 0;
	}*/

	// Create vectors
	height = Vector<Vec>(numVertices);
	shift = Vector<Vec>(numVertices);

	// In order to ignore borders
	this->detailed->flagBorderVertices();

	/*for(int i = 0; i < numVertices; i++)
	{
		if(this->detailed->vd(i)->flag != VF_BORDER)
		{
			height[i] = (base.n(i)->vec() * -distance[i]);
			shift[i] = (detailed->v(i)->vec() + height[i]) - base.v(i)->vec();
		}
	}*/

	// hide by default
	this->isVisible = false;
}

float SmoothStep::IntersectionSearch(Mesh * mesh, VertexDetail * vd, Ray & ray, HitResult & res, int flag, int depth)
{
	if(depth > 2 || vd->isBorderFlag())
		return NOT_SET;

	float distance = NOT_SET;

	for(Vector<Face*>::iterator faces = vd->ifaces.begin(); faces != vd->ifaces.end(); faces++)
	{
		Face * f = *faces;

		if(f->flag != flag)
		{
			f->intersectionTest(ray, res);

			if(res.hit && res.distance < distance)
			{
				distance = res.distance;
				break;
			}

			// mark as visited face
			f->flag = flag;
		}
	}

	if(distance == NOT_SET)
	{
		for(Vector<Face*>::iterator faces = vd->ifaces.begin(); faces != vd->ifaces.end(); faces++)
		{
			Face * f = *faces;

			for(int i = 0; i < 3; i++)
			{
				distance = IntersectionSearch(mesh, mesh->vd(f->VIndex(i)), ray, res, flag, ++depth);
				
				if(distance != NOT_SET)
					break;
			}
		}
	}

	return distance;
}

Mesh * SmoothStep::detailedMesh()
{
	return this->detailed;
}

Mesh * SmoothStep::baseMesh()
{
	return &this->base;
}

void SmoothStep::draw()
{
	if(this->isVisible)
	{
		int numVertices = detailed->numberOfVertices();

		Vector<Vec> height_starts, height_ends;
		Vector<Vec> shift_starts, shift_ends;

		for(int i = 0; i < numVertices; i++)
		{
			height_starts.push_back(detailed->v(i)->vec());
			height_ends.push_back(detailed->v(i)->vec() + height[i]);

			shift_starts.push_back(base.v(i)->vec());
			shift_ends.push_back(base.v(i)->vec() + shift[i]);
		}

		SimpleDraw::IdentifyArrows(shift_starts, shift_ends, 2.0f);
		SimpleDraw::IdentifyArrows(height_starts, height_ends, 2.0f, 0.5f, 0.1f, 0.1f);
	}
}
