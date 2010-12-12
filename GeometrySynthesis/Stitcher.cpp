#include "GridMesh.h"
#include "Stitcher.h"

#include "PolygonArea.h"

#include <QMap> // why not..

double theta = M_PI / 18.0; // 10 deg

Stitcher::Stitcher(GridMesh * srcGridMesh, bool isFillSeams)
{
	this->gm = srcGridMesh;
	tri_patch = &gm->tri_patch;
	this->M = &gm->triangluatedExtension;
	
	if(tri_patch->size() < 2)	return;

	int startTime = clock();

	// Find seams between patches
	printf("\nFinding Seams..");
	Seam::FindSeams(gm, M, comulativeNumV, seams);

	printf("Adjusting boundry..");
	for(Vector<Seam>::iterator seam = seams.begin(); seam != seams.end(); seam++)
	{
		// STAGE (1) : Make hole boundry shape better on both sides
		while(StageOne(seam->M, seam->boundryA) != STAGE_COMPLETE);
		while(StageOne(seam->M, seam->boundryB) != STAGE_COMPLETE);
	}

	// Experiment: more seam treatments
	for(Vector<Seam>::iterator seam = seams.begin(); seam != seams.end(); seam++)
		TreatSeam(*seam);

	M->computeNormals();

	if(isFillSeams)
	{
		printf("Zipping boundaries..\n");
		for(Vector<Seam>::iterator seam = seams.begin(); seam != seams.end(); seam++)
		{
			// STAGE (2) : Zip seam
			ZipSeam(*seam);

			printf(".");
		}
	}

	M->clearColors();

	// Refresh mesh 'M' for drawing
	M->computeNormals();
	M->rebuildVBO();
	
	M->isReady = true;
	M->isVisible = true;
	M->isTransparent = false;
	//M->isDrawWireframe = true;
	//M->isShowVertexNormals = true;
	//M->isShowFaceNormals = true;
	//M->isFlatShade = true;

        printf("\n\nBlending done (%d ms).\n", (int)clock() - startTime);
}

int Stitcher::StageOne(Mesh * mesh, Vector<int> & boundry)
{
	double phi = (M_PI) + (M_PI / 2.25f);	// 260 degrees (or 100 between 2 edges)
	double omega = M_PI / 4.0;				// 45 degrees

	int N = boundry.size();

	bool changesMade = false;

	VirtualTriangle lastTriangle;

	StdSet<int> notBorder;

	for(int v = 1; v <= N; v++)
	{
		int v1 = boundry[(v - 1) % N];
		int v2 = boundry[(v    ) % N];
		int v3 = boundry[(v + 1) % N];

		// Internal angle (angle between adjacent edges)
		double alpha = mesh->angleAroundVertex(v2);
		
		Face * f1 = mesh->getBoundryFace(v2,v1), *f2 = mesh->getBoundryFace(v2,v3);

		// Get the dihedral
		double beta = Vertex::angleBetweenTwo(f1->normal(), f2->normal());
	
		if(alpha >= phi && beta < omega)
		{
			// Avoid self intersect by avoiding consecutive triangle addition
			if(!lastTriangle.has(v1,v2,v3))
			{
				mesh->addFace(v1, v2, v3, mesh->numberOfFaces(), true);

				//Debug:
				//testLines1.push_back(Line(mesh->ver(v1), mesh->ver(v2)));
				//testLines1.push_back(Line(mesh->ver(v2), mesh->ver(v3)));
				//testLines1.push_back(Line(mesh->ver(v3), mesh->ver(v1)));

				changesMade = true;

				// Remember last triangulation
				lastTriangle.v1 = v1;
				lastTriangle.v2 = v2;
				lastTriangle.v3 = v3;

				notBorder.insert(v2);
			}
		}
	}

	// Update boundry
	if(changesMade)
	{
		Vector<int> modifiedBoundry;

                for(int v = 0; v < (int)boundry.size(); v++)
		{
			if(notBorder.find( boundry[v] ) == notBorder.end())
				modifiedBoundry.push_back(boundry[v]);
		}

		boundry = modifiedBoundry;
	}

	if(changesMade)
		return STAGE_NOT_COMPLETE;
	else
		return STAGE_COMPLETE;
}

void Stitcher::TreatSeam(Seam & seam)
{
        int N, j, vj;

	Mesh * M = seam.M;

	double phi = M_PI / 3.0;
	
	// Experiment: smooth boundaries (bad results..)
	QMap<int, Vertex> newPos;

	N = seam.boundryA.size();
	for (j = 0; j < N; j++)		
	{
		vj = seam.boundryA[j];

		if(M->angleAroundVertex(vj) < phi)
			newPos[vj] = Smoother::LaplacianSmoothVertex(M, vj);
	}
	
	N = seam.boundryB.size();
	for (j = 0; j < N; j++)	
	{
		vj = seam.boundryB[j];

		if(M->angleAroundVertex(vj) < phi)
			newPos[vj] = Smoother::LaplacianSmoothVertex(M, vj);
	}

	foreach (int vi, newPos.keys())		
		M->ver(vi) = (newPos[vi] * 0.3f) + (M->ver(vi) * 0.7f);
}

int Stitcher::ZipSeam(Seam & seam)
{
	Mesh * M = seam.M;
	Vector<int>::iterator bdryA, bdryB;

	// Important Step !
	seam.LineupSeams(bdryA, bdryB);

	int i, i2, j, j2;
	Vec pi, pi2, pj, pj2;
	double len1, len2;
	int fIndex = M->numberOfFaces();

        bool a_done = false, b_done = false;

        int starti = *bdryA;

	// DEBUG:
	Vector<Vec> red, blue;
        //StdSet<Face*> green, yellow;
	//foreach(int vi, seam.boundryA)	blue.push_back(*M->v(vi));
	//foreach(int vi, seam.boundryB)	red.push_back(*M->v(vi));
	M->redPoints.push_back(red);
	M->bluePoints.push_back(blue);

	while (!a_done && !b_done)
	{
		bool advanceA = false;

		if(bdryA + 1 == seam.boundryA.end()) a_done = true;
		if(bdryB + 1 == seam.boundryB.end()) b_done = true;

		if(!a_done && !b_done)
		{
			i = *bdryA;
			i2 = *(bdryA+1);	// i + 1

			j = *bdryB;
			j2 = *(bdryB+1);	// j + 1

			pi = M->vec(i); pi2 = M->vec(i2);	pj = M->vec(j); pj2 = M->vec(j2);

			len1 = (pi - pi2).norm() + (pj - pi2).norm();
			len2 = (pi - pj2).norm() + (pj - pj2).norm();

			double minAngleA = Vertex::minAngle(pi, pi2, pj);
			double minAngleB = Vertex::minAngle(pi, pj2, pj);

			// Length criterion
			if(len1 < len2)
				advanceA = true;

			if(advanceA)
			{
				if(minAngleA < theta && minAngleB > minAngleA && minAngleB > theta)		advanceA = false;
			}
			else
			{
				if(minAngleB < theta && minAngleA > minAngleB && minAngleA > theta)		advanceA = true;
			}

			// Avoid almost and self-intersects
			if(advanceA)
			{
				// Create plane from future face, test if other points are projected on it
				Plane p(pi, pi2, pj);
				if(p.IsInTri(pj2, pi, pi2, pj))	{advanceA = false;}
			}
			else
			{
				Plane p(pi, pj2, pj);
				if(p.IsInTri(pi2, pi, pj2, pj))	{advanceA = true;}
			}

			if(advanceA)
			{
				M->addFace(i2, j, i, fIndex++, true);
				bdryA++;
			}
			else
			{
				M->addFace(j2, j, i, fIndex++, true);
				bdryB++;

				//green.insert(&M->face.back());
			}

			addedFaces.push_back(&(M->facesList()->back()));

			// Debug:
			//testPoints2.push_back(addedFaces.back()->center());
		}
	}

	//M->greenFaces.push_back(green);

	// Fill last hole
	fillSmallHole(M, starti);

	// Smooth vertices with large angles > 150
	FairSeams();

	return 1;
}

void Stitcher::fillSmallHole(Mesh * M, int borderVertex)
{
	int fIndex = M->numberOfFaces();

	StdList<int> boundry = M->getBoundry(borderVertex, true);

	StdList<int>::iterator i = boundry.begin();
	StdList<int>::iterator j = i;	j++;

	StdList<int>::reverse_iterator k = boundry.rbegin();
	StdList<int>::reverse_iterator l = k; 	l++;

	double len1, len2;

	while(*i != *k){
		bool advanceA = false;

		len1 = (M->vec(*i) - M->vec(*j)).norm() + (M->vec(*k) - M->vec(*j)).norm();
		len2 = (M->vec(*i) - M->vec(*l)).norm() + (M->vec(*k) - M->vec(*l)).norm();
			
		if(len1 < len2)	advanceA = true;

		double minAngleA = Vertex::minAngle(M->vec(*i), M->vec(*j), M->vec(*k));
		double minAngleB = Vertex::minAngle(M->vec(*i), M->vec(*l), M->vec(*k));
		
		if(advanceA)
		{
			if(minAngleA < theta && minAngleB > minAngleA)
				advanceA = false;
		}
		else
		{
			if(minAngleB < theta && minAngleA > minAngleB)
				advanceA = true;
		}

		// Avoid almost and self-intersects
		if(advanceA){
			Plane p(M->vec(*i), M->vec(*j), M->vec(*k));
			if(p.IsInTri(M->vec(*l), M->vec(*i), M->vec(*j), M->vec(*k)))
				advanceA = false;
		}else
		{
			Plane p(M->vec(*i), M->vec(*l), M->vec(*k));
			if(p.IsInTri(M->vec(*j), M->vec(*i), M->vec(*l), M->vec(*k)))
				advanceA = true;
		}

		if(advanceA){
			M->addFace(*j, *k, *i, fIndex++, true);

			i++;
			j++;
		} else {
			M->addFace(*l, *k, *i, fIndex++, true);

			k++;
			l++;
		}

		addedFaces.push_back(&(M->facesList()->back()));

		if (j == boundry.end() || l == boundry.rend() || *i == *l || *k == *j || *j == *l)	break;
	}

	// Last triangle: 
	Vector<int> lastTri = LIST_TO_VECTOR(M->getBoundry(*i, true));

	if(lastTri.size() > 2)
		M->addFace(lastTri[0], lastTri[1], lastTri[2], fIndex++, true);
}

int Stitcher::FairSeams(int numIterations)
{
	Mesh * M = seams[0].M;

	// Deal with 'cap' triangles:
	double lamda_max = (2.0 / 3.0) * M_PI;	// 150°
	double lamda_min = M_PI / 18.0;			// 10 deg

	for(int si = 0; si < numIterations; si++)
	{
		StdSet<int> badVertices;

		// Find bad vertices
		foreach (Face * f, addedFaces)
		{
			VertexAngle va = f->largestAngle();

			if(M->vd(va.index)->ifaces.size() == 3)
			{
				M->v(va.index)->set(Smoother::LaplacianSmoothVertex(M, va.index));
				//testPoints2.push_back(*M->v(va.index));
			}

			if(va.angle > lamda_max || va.angle < lamda_min)
			{
				//M->testVertex.push_back(*M->v(va.index));
				badVertices.insert(va.index);
			}
		}

		// Smooth these nasty vertices
		foreach(int vi, badVertices)
		{
			if(!M->vd(vi)->hasAnyNeighbour(badVertices))
			{
				M->v(vi)->set(Smoother::LaplacianSmoothVertex(M, vi));
			}
		}
	}

	// The smooth everything approach...
	/*Mesh * M = seams[0].M;

	Vector<int> fairGroup;
	StdSet<Face *> facesList;
	StdSet<int> sharpPointsA, sharpPointsB;

	foreach(Seam seam, seams){
		foreach(int vi, seam.boundryA)	sharpPointsA.insert(vi);
		foreach(int vi, seam.boundryB)	sharpPointsB.insert(vi);
	}

	// Smooth one side 
	for(int i = 0; i < 2; i++){
		foreach(int vi, sharpPointsA)	M->vec(vi] = Smoother::LaplacianSmoothVertex(M, vi);
		foreach(int vi, sharpPointsB)	M->vec(vi] = Smoother::LaplacianSmoothVertex(M, vi);
	}

	//M->setColor(SET_TO_VECTOR(sharpPointsA),100,100,200);
	*/

	return 1;
}
