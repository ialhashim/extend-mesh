#pragma once

#define STAGE_NOT_COMPLETE 0
#define STAGE_COMPLETE 1

#include "MeshPatch.h"
#include "Seam.h"

class GridMesh;

class Stitcher
{
private:
	GridMesh * gm;
	Vector<MeshPatch> * tri_patch;
	Mesh * M; // working mesh

	Vector<Seam> seams;
	Vector<Face*> addedFaces;

	Vector<int> comulativeNumV;

	void fillSmallHole(Mesh * M, int borderVertex);

public:
	Stitcher(GridMesh * srcGridMesh, bool isFillSeams);

	//void FindSeams();

	int StageOne(Mesh * mesh, Vector<int> & boundry);

	void TreatSeam(Seam & seam);

	int ZipSeam(Seam & seam); // stage 2?

	int FairSeams(int numIterations = 1);

private:
	struct VirtualTriangle{
		int v1,v2,v3;

		VirtualTriangle(){v1 = v2 = v3 = -1;}

		inline bool has(int w1, int w2, int w3)	{
			if(v1 == w1 || v1 == w2 || v1 == w3 || 
				v2 == w1 || v2 == w2 || v2 == w3 || 
				v3 == w1 || v3 == w2 || v3 == w3) 
				return true;
			else
				return false;
		}
	};
};
