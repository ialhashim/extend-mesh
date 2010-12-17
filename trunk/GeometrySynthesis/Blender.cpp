#include "GridMesh.h"
#include "Blender.h"

#include "ExtendMeshHeaders.h"

Blender::Blender( GridMesh * src_gm )
{
	this->gm = src_gm;
	tri_patch = &gm->tri_patch;

	isDone = false;

	Mesh * pM = &M;

	Seam::FindSeams(gm, pM, comulativeNumV, seams);

	spline = user_curve->getSpline()->ToUniformBezierSpline();

	// Find seam times
	FindSeamTimes();

	isDone = true;

	// Sample the blending area
	Sample();

	Reconstruct();

	Export();
}

void Blender::FindSeamTimes()
{
	for(int i = 0; i < (int)seams.size(); i++)
	{
		float start = FLT_MAX;
		float end = FLT_MIN;

		// Start time
		foreach(const int vi, seams[i].boundryA)
		{
			start = Min( start, spline.GetTimeClosestPoint(M.vec(vi)) );
		}

		// End time
		foreach(const int vi, seams[i].boundryB)
		{
			end = Max( end, spline.GetTimeClosestPoint(M.vec(vi)) );
		}

		seamTime.push_back(SeamTime(start, end));
	}
}

void Blender::Sample()
{
	int resolution = 25;

	for(int i = 0; i < (int)seams.size(); i++)
	{
		Vector<Plane> csPlane;

		Seam * s = &seams[i];
		SeamTime * time = &seamTime[i];

		float timeDuration = time->end - time->start;
		float timeStep = timeDuration / resolution;
		float t = time->start;

		for(int v = 0; v <= resolution; t += timeStep, v++)
		{
			Vec pointOnCurve = spline.GetValue(t);
			Vec normalPlane = spline.GetDerivativeGlobal(t).unit();

			csPlane.push_back(Plane(normalPlane, pointOnCurve));

			//testPlanes.push_back(csPlane.back());
		}

		// Collect curves to sample
		curve_samples.push_back(Vector<CurveSample>());
		blended_curves.push_back(Vector<ExtendedPolygon>());
		Vector<ClosedPolygon> poly;

		Vec upVec, bVec; 
		csPlane[0].getTangents(upVec,bVec);

		// Find cross section samples per patch
		for(int v = 0; v < (int)csPlane.size(); v++)
		{
			bool hasClosedPolyA = false;

			// Per patch
			for(int pi = s->patchIdA; pi <= s->patchIdB; pi++)
			{
				MeshPatch * pPatch = &tri_patch->at(pi);

				Rotation q(csPlane[Max(0,v-1)].n, csPlane[v].n);
				upVec = q.rotate(upVec); bVec = q.rotate(bVec);

				// Closed polygon structure
				poly.push_back(ClosedPolygon(csPlane[v], upVec, bVec));

				StdList<Face> * faceList = pPatch->fpdMesh.facesList();

				// For all Faces in full patch
				for(StdList<Face>::iterator f = faceList->begin(); f != faceList->end(); f++)
				{
					Face * face = &(*f);

					Vec p1, p2;	
					int isContour = csPlane[v].ContourFacet(face, p1, p2);

					if(isContour > 0)
					{
						poly.back().insertLine(p1,p2);
					}
				}

				// First or second patch
				if(pi == s->patchIdA)	
				{
					// Create weighted polygon
					if(poly.back().isClosed())
						hasClosedPolyA = true;
				}
				else if(hasClosedPolyA && poly.back().isClosed())
				{
					QColor c1(255,0,0);
					QColor c2(0,255,0);

					curve_samples.back().push_back(
						CurveSample( ExtendedPolygon(poly[poly.size() - 2], c1, 0, csPlane[v].center), 
						ExtendedPolygon(poly.back(), c2, 0, csPlane[v].center))	);
				}
			}
		}

		// Assign weights
		float w = 1.0f;
		int numSides = 0;
		Vector<CurveSample> * curves = &curve_samples.back();

		for(int ci = 0; ci < (int)curves->size(); ci++)
		{
			w = (float)ci / (curves->size() - 1);

			// its pretty, thats why
			CurveSample * curveSample = &curves->at(ci);

			curveSample->A.weight = 1.0f - w;
			curveSample->B.weight = w;

			numSides = Max(numSides, Max((int)curveSample->A.allPoints.size(), (int)curveSample->B.allPoints.size()));
		}

		numSides *= 1;

		// blended curves
		for(int ci = 0; ci < (int)curves->size(); ci++)
		{
			CurveSample * curveSample = &curves->at(ci);

			blended_curves.back().push_back(ExtendedPolygon(curveSample->A, curveSample->B, numSides, QColor(0,0,255)));
		}
	}
}

void Blender::Reconstruct()
{
	mainWindow->ui.viewer->makeCurrent();

	for(int si = 0; si < (int)seams.size(); si++)
	{
		int lengthCount = blended_curves[si].size(); if(lengthCount < 1) continue;
		int widthCount = blended_curves[si].front().closedPoints.size();

		reconPatch.push_back(Mesh(lengthCount * widthCount));
		Mesh * currPatch = &reconPatch.back();

		int vIndex = 0, fIndex = 0;

		for(int v = 0; v < lengthCount; v++)
		{
			for(int u = 0; u < widthCount; u++)
				currPatch->addVertex( blended_curves[si][v].closedPoints[u], vIndex++ );
		}

		for(int v = 0; v < lengthCount - 1; v++)
		{
			for(int u = 0; u < widthCount; u++)
			{ 
				int offset = (widthCount * (v));

				int v0 = u + offset;
				int v1 = ((u + 1) % widthCount) + offset;
				int v2 = v1 + widthCount;
				int v3 = v0 + widthCount;

				// new faces indices
				int f1 = fIndex;
				int f2 = fIndex + 1;

				// Add the two faces
				currPatch->addFace(v2, v1, v0, f1);
				currPatch->addFace(v3, v2, v0, f2);

				fIndex += 2;
			}
		}

		currPatch->clearColors();
		currPatch->computeNormals();
		currPatch->createVBO();

		currPatch->isVisible = true;
		currPatch->isReady = true;
	}

}

void Blender::Export()
{
	Mesh exportMesh;

	int n = seams.size();

	int vIndex = 0, pIdA, pIdB;

	Plane A, B;

	StdSet<int> patchIds;
	StdMap<int, Plane> fronts;
	StdMap<int, Plane> backs;

	for(int si = 0; si < n; si++)
	{
		int lengthCount = blended_curves[si].size(); if(lengthCount < 1) continue;
		int widthCount = blended_curves[si].front().closedPoints.size();

		for(int v = 0; v < lengthCount; v++)
		{
			for(int u = 0; u < widthCount; u++)
			{
				exportMesh.addVertex( blended_curves[si][v].closedPoints[u], vIndex++ );
			}
		}

		patchIds.insert(pIdA = seams[si].patchIdA);
		patchIds.insert(pIdB = seams[si].patchIdB);

		A = blended_curves[si].back().plane;
		B = blended_curves[si].front().plane.inverseDir();

		backs[pIdA] = B;
		fronts[pIdB] = A;

		if(si == 0)	fronts[pIdA] = B;
		if(si == n - 1)	backs[pIdB] = A;
	}

	foreach(int pid, patchIds)
	{
		Mesh * mesh = &tri_patch->at(pid).fpdMesh;

		for(int vi = 0; vi < mesh->numberOfVertices(); vi++)
		{
			Vec p = *mesh->v(vi);

			if(fronts[pid].IsFront(p) && backs[pid].IsFront(p))
			{
				exportMesh.addVertex(p, vIndex++);
			}
		}

		// For all Faces in full patch
		StdList<Face> * faceList = mesh->facesList();

		for(StdList<Face>::iterator f = faceList->begin(); f != faceList->end(); f++)
		{
			Vec p = f->center();

			if(fronts[pid].IsFront(p) && backs[pid].IsFront(p))
			{
				exportMesh.addVertex(p, vIndex++);
			}
		}
	}

	exportMesh.saveToFile("ExportedBlendMesh.obj");
}

void Blender::draw()
{
	if(!isDone)
		return;

	/*for(int i = 0; i < seams.size(); i++)
	{
	Seam * s = &seams[i];
	}*/

	for(int si = 0; si < (int)seams.size(); si++)
	{
		for(int c = 0; c < (int)curve_samples[si].size(); c++)
		{
			//curve_samples[si][c].A.drawColored();
			//curve_samples[si][c].B.drawColored();

			//blended_curves[si][c].drawColored();
		}
	}

	for(int si = 0; si < (int)reconPatch.size(); si++)
	{
		reconPatch[si].draw();
	}
}
