#pragma once

#include "Mesh.h"

class GridMesh;

class Seam
{

private:

public:
	Mesh *M;
	Vector<int> boundryA, boundryB;

	int index;
	int closestA, closestB;
	int patchIdA, patchIdB;

        Seam(Mesh * m, Vector<int> bA, Vector<int> bB, int closeA, int closeB, int Index, int patch1_index, int patch2_index) :
          M(m), boundryA(bA), boundryB(bB), index(Index), closestA(closeA), closestB(closeB), patchIdA(patch1_index), patchIdB(patch2_index) {}

	static void FindSeams(GridMesh * gm, Mesh* & M, Vector<int> & comulativeNumV, Vector<Seam> & seams);

	void LineupSeams(Vector<int>::iterator & bdryA, Vector<int>::iterator & bdryB);

	void draw();
};

struct SeamTime
{
	float start, end;
	SeamTime(float Start, float End): start(Start), end(End){}
};
