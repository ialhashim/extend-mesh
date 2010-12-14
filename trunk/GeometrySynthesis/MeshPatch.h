#pragma once

#include "Mesh.h"
#include "Octree.h"
#include "SimpleSquare.h"
#include "Triangle.h"
#include "DistanceField.h"

class MeshPatch
{

public:
	int id;

	MeshPatch(int numVerts, int Id);	/* Constructor */

	VertexMap vertex;
	VertexMap corrPoint;

	HashMap<int, Vec> cloud;
	StdSet<int> nonTriangulated;

	StdSet<int> usedPoints;

	Vector<SimpleTriangle> triangles;
	VertexMap usedPointsMap;
	VertexMap usedIndexToCorr;

        int convertToCorr(int index);
        StdSet<int> convertToCorr(const StdList<int>& indices);
        Vector<int> getBorder();

	// Grid Related data
	Vector<SimpleSquare> patch;
	Vector<SimpleSquare> border;

	Vector< Vector<SimpleSquare> > unrolledPatch;
	int start_x, start_y;

	DistanceField distanceField;

	Mesh mesh;
	Color4 falseColor;

	// Full patch structures (for overlapping)
	Mesh fpbMesh; /* full patch base mesh (fpb-mesh) */
	Mesh fpdMesh; /* detailed */

	// For sampling
        Vector<Triangle> parameterTriangles;
        OctreeTriangles parameterOctree;
        StdList<Triangle*> parameterTriPointers();

        // Visulaize paramter triangles
        void drawparameterTris();
	Vector<Vector<Vec> > readyToDrawTris();

	void insertPoint(int corrPointIndex, const Vec& pos);
	void insertFace(int cp1, int cp2, int cp3);

	void insertNonTriangulated(int cp);

	void makeMesh();
	void replaceMesh(Mesh * from, bool isReplaceCorr);

        void drawPointCloud();
};
