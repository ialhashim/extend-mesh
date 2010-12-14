#include "GridMesh.h"
#include "Reconstructor.h"
#include "SimpleDraw.h"

#include "ExtendMeshHeaders.h"

Reconstructor::Reconstructor(GridMesh * srcGridMesh)
{
	this->gm = srcGridMesh;
	this->patch = &gm->tri_patch;
	this->numPatches = gm->tri_patch.size();

	this->isDone = false;

	printf("\nSampling..");

	// Sample area
	this->Sample();

	// Triangulate samples
	this->Triangulate();

	// Combine
	this->Combine();

	printf("Done (num samples = %d).\n", samples.size());

	this->isDone = true;
}

void Reconstructor::Sample()
{
	int resolution = 2;

        Vector<Vector<double> > weights = GridSquare::uniformSample( resolution );

	// Collect all borders involved
	Vector<SimpleSquare> border;

        for(int i = 0; i < (int)patch->size(); i++)
	{
		MeshPatch * pPatch = &patch->at(i);

		Vector<SimpleSquare> * currBorder = &(pPatch->border);
		border.insert(border.begin(), currBorder->begin(), currBorder->end());

		pPatch->distanceField = DistanceField(resolution * 2, pPatch->start_x, pPatch->start_y, 
			pPatch->unrolledPatch, gm->totalWidth, gm->totalHeight);

		//patches.push_back(pPatch->readyToDrawTris());
	}

	// Recycling
	int patchId = 0, count = 0;
	double sumBlendVal = 0, blendVal = 0, blendSum = 0, blend_delta = 0; 
	MeshPatch * curr_patch;
	Face * face;
	Vec pos, n, faceNormal, emptyVec;
	SampleHit h;

	Vector<Vec> pointsOnSquare;
	foreach(const Vector<double> w, weights) pointsOnSquare.push_back( PointOnUnitSquare(w) );

	// For each square on border
	foreach(const SimpleSquare s, border)
	{
		// For each sample on square
                for(int wi = 0; wi < (int)weights.size(); wi++)
		{
			Vector<double> w = weights[wi];

			sumBlendVal = 0.0f;

			// Check intersection with patches (find sample hits)
			HashMap<int, SampleHit> sampleHits;
			HashMap<int, double> blendVals;
			count = 0;

			Vec p = pointsOnSquare[wi];

			p.x += s.v;
			p.y += s.u;
			p.z -= 0.5f;

			Ray ray(p, Vec(0,0,1));

			//seeds.push_back(SampleSeed(ray.origin, ray.direction, seeds.size()));

                        for(int i = 0; i < (int)patch->size(); i++)
			{
				patchId = i;
				curr_patch = &patch->at(patchId);

				HitResult hitRes;

                                curr_patch->parameterOctree.testIntersectRayBoth(ray, hitRes);

				// Record sample hits
				if(hitRes.hit)
				{
					blendVal = curr_patch->distanceField.blendAt(s.u, s.v, w);
					blendVals[patchId] = blendVal;
					sumBlendVal += blendVal;

					sampleHits[patchId] = SampleHit(hitRes.u, hitRes.v, hitRes.index, blendVal);

					count++;
				}
			}

			// Add sample (based on blend weights)
			if(count)
			{
				// Make sure blend weights add up to one:
				blendSum = 0.0;	blend_delta = 0.0;
				for(HashMap<int, SampleHit>::iterator it = sampleHits.begin(); it != sampleHits.end(); it++)
					blendSum += it->second.blendWeight;

				// Correction factor
				blend_delta = (1.0 - blendSum) / count;

				// Clear old vector values
				pos = n = emptyVec;

				for(HashMap<int, SampleHit>::iterator it = sampleHits.begin(); it != sampleHits.end(); it++)
				{
					patchId = it->first;
					h = it->second;
					curr_patch = &patch->at(patchId);

					face = curr_patch->fpdMesh.f(h.findex);
					faceNormal = *curr_patch->fpdMesh.fn(h.findex);
					
					// Ignore blending of only one hits
					if(sampleHits.size() == 1)
					{
						h.blendWeight = 1.0;
						blend_delta = 0.0;
					}

					pos += ( face->getBary(h.u, h.v) * (h.blendWeight + blend_delta) );
					n += faceNormal;
				}

				n /= count;

				samples.push_back(SamplePoint(pos, n.unit(), samples.size()));
			}
		}
	}
}

void Reconstructor::Triangulate()
{
	
}

void Reconstructor::Combine()
{

}

Vec Reconstructor::PointOnUnitSquare(const Vector<double>& w)
{
	Vec p ,corner[4];

	corner[1].x = 1;
	corner[2].x = 1; corner[2].y = 1;
	corner[3].y = 1;

	for(int i = 0; i < 4; i++)
	{
		p.x += corner[i].y * w[i];
		p.y += corner[i].x * w[i];
		p.z += corner[i].z * w[i];
	}
	
	return p;
}

void Reconstructor::draw()
{
	if(!isDone)
		return;

	Vector<Vec> samplesPos, seedPos;

	glClear(GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_LIGHTING);
	glDisable(GL_LIGHTING);

	Vec dir = mainWindow->ui.viewer->camera()->viewDirection();

	glPointSize(8.0f);

	glColor3f(0.46f, 1.0f, 0.03f);

	glBegin(GL_POINTS);
	for(Vector<SamplePoint>::iterator s = samples.begin(); s != samples.end(); s++)
	{
		if(s->n * dir < 0) // hack
		{
			glNormal3fv(s->n);
			glVertex3fv(s->pos);
		}
	}
	glEnd();

	glColor3f(0.0,0.0,0);
	glPointSize(10.0f);

	glDisable(GL_LIGHTING);
	glBegin(GL_POINTS);
	for(Vector<SamplePoint>::iterator s = samples.begin(); s != samples.end(); s++)
	{
		if(s->n * dir < 0)
		{
			glVertex3fv(s->pos);
		}
	}
	glEnd();
	glEnable(GL_LIGHTING);
	
	// Debug sample seeds
	for(Vector<SampleSeed>::iterator s = seeds.begin(); s != seeds.end(); s++)
		seedPos.push_back(s->pos);
	SimpleDraw::IdentifyPoints(seedPos, 0, 1, 0, 8);

	/*
	// Draw parameter meshes
	for(int i = 0; i < numPatches; i++)
	{
		SimpleDraw::DrawTriangles(patches[i]);
	}

	this->patch = &gmesh.back().tri_patch; // stupid memory managment

	for(int i = 0; i < patch->size(); i++)
	{
		MeshPatch * curr_patch = &patch->at(i);

		curr_patch->paramterOctree.draw(0,1,0);
	}*/
}
