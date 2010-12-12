#include "GridMesh.h"
#include "Seam.h"

void Seam::FindSeams(GridMesh * gm, Mesh* & M, Vector<int> & comulativeNumV, Vector<Seam> & seams)
{
	Vector<MeshPatch> * tri_patch = &gm->tri_patch;

	Vector<int> closestBoundryA, closestBoundryB;
	Vector<int> patchIdsA, patchIdsB;

	// Everything will be merged into mesh M, i.e. first patch
	*M = Mesh(tri_patch->at(0).mesh);

	Mesh *A, *B;
	Vector<int> borderA, borderB;
	float minDist, dist;
	Vertex vA, vB;
	int closestA = 0, closestB = 0;
	borderA = tri_patch->at(0).getBorder();

	comulativeNumV.push_back(0); // no offset for first mesh A

        for(int p = 0; p < (int)tri_patch->size() - 1; p++)
	{
		MeshPatch * patch1 = &tri_patch->at(p);
		MeshPatch * patch2 = &tri_patch->at(p+1);

		patchIdsA.push_back(patch1->id);
		patchIdsB.push_back(patch2->id);

		A = &patch1->mesh;
		B = &patch2->mesh;

		borderB = patch2->getBorder();

		// Check for bad vertices
		foreach(int vi, borderB)
		{
			if(B->vd(vi)->isMissingBorder())
			{
				// Middle face should be good with high probability
				int startFace = B->numberOfFaces() * 0.5;

                                Vector<int> subFaces = SET_TO_VECTOR(B->getManifoldFaces(startFace));
                                Mesh * cleaned = B->CloneSubMesh(subFaces);

				bool isLastPart = false;

                                if(p == (int)tri_patch->size() - 2)
					isLastPart = true;

				patch2->replaceMesh(cleaned, isLastPart);

				B = &patch2->mesh;

				borderB = B->getBorderVertices();

				break;
			}
		}

		if(A->numberOfVertices() == 0 || B->numberOfVertices() == 0)
			continue;

		closestA = 0;
		closestB = 0;

		// find closest borders (compare one with others, then compare again the other side)
		minDist = FLT_MAX;
		vA = *A->v(borderA[closestA]);

		foreach(int i, borderB)
		{	
			if((dist = (vA - *B->v(i)).squaredNorm()) < minDist){
				minDist = dist;
				closestB = i;
			}
		}

		minDist = FLT_MAX;
		vB = *B->v(closestB);

		foreach(int i, borderA)
		{
			if((dist = (vB - *A->v(i)).squaredNorm()) < minDist){
				minDist = dist;
				closestA = i;
			}
		}
		vA = *A->v(closestA);

		closestBoundryA.push_back(closestA + comulativeNumV.back());

		comulativeNumV.push_back(comulativeNumV.back() + A->numberOfVertices());

		closestBoundryB.push_back(closestB + comulativeNumV.back());

		M->mergeWith(*B);

		borderA = borderB;
	}

	// Refresh mesh 'M' for processing
	M->reassignFaces();
	M->getUmbrellas();

	Vector<int> bdryA, bdryB;

        for(int i = 0; i < (int)closestBoundryA.size(); i++)
	{
		int closestA = closestBoundryA[i];
		int closestB = closestBoundryB[i];

		bdryA = LIST_TO_VECTOR(M->getBoundry(closestA));
		bdryB = LIST_TO_VECTOR(M->getBoundry(closestB));

		seams.push_back(Seam(M, bdryA, bdryB, closestA, closestB, seams.size(), patchIdsA[i], patchIdsB[i]));
	}
}

void Seam::LineupSeams(Vector<int>::iterator & bdryA, Vector<int>::iterator & bdryB)
{
	//std::reverse(boundryA.begin(), boundryA.end()); // does this make any difference ?

	bdryA = boundryA.begin();
	bdryB = boundryB.begin();

	//debug
	//foreach(int vi, boundryA) M->testVertex.push_back(M->vertex[vi]);
	//foreach(int vi, boundryB) M->testVertex.push_back(M->vertex[vi]);

	// unify boundary directions
	Vector<Vec> pointsA, pointsB;
	for(Vector<int>::iterator it = bdryA; it != boundryA.end(); it++) pointsA.push_back(M->vec(*it));
	for(Vector<int>::iterator it = bdryB; it != boundryB.end(); it++) pointsB.push_back(M->vec(*it));

	Vec seamNormalA,seamNormalB;
        findNormal3D(pointsA, seamNormalA);
        findNormal3D(pointsB, seamNormalB);

	// Reverse direction of boundary 'B'
	if(seamNormalA * seamNormalB < 0)
	{
		std::reverse(boundryB.begin(), boundryB.end());
		bdryB = boundryB.begin();
	}

	// Align start points
	Vector<int>::iterator startB = bdryB;
	int startA = *bdryA;
	float minDist = FLT_MAX;
	for(Vector<int>::iterator it = boundryB.begin(); it != boundryB.end(); it++)
	{
		float dist = (*M->v(*it) - *M->v(startA)).norm();

		if(dist < minDist)
		{
			minDist = dist;
			startB = it;
		}
	}
	std::rotate(boundryB.begin(), startB, boundryB.end());
	bdryB = boundryB.begin();
}

void Seam::draw()
{
	Vector<Vec> seam1, seam2;

	foreach(int vi, boundryA)	seam1.push_back( M->vec(vi) );
	foreach(int vi, boundryB)	seam2.push_back( M->vec(vi) );

	glDisable(GL_LIGHTING);

	SimpleDraw::IdentifyConnectedPoints(seam1, 1,0,0);
	SimpleDraw::IdentifyConnectedPoints(seam2, 0,1,0);
}
