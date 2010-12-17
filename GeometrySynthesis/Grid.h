#ifndef Grid_H
#define Grid_H

#include "Mesh.h"
#include "Plane.h"
#include "Spline.h"

#include "SmoothStairway.h"
#include "GridSquare.h"

#include "Point.h"
#include "CrossSection.h"
#include "ClosedPolygon.h"
#include "Octree.h"
#include "LocalFrame.h"

class Grid : public Mesh
{
private:
	int numSides;
	int numStrips;
	int gridSize;

	double r;
	double width;
	double length;

	SmoothStairway * stair;
        StdList<BaseTriangle*> meshFaces;
	Vector<int> selectedFaces;
	HashMap<int, Vec> originalMeshNormals;

	// Square data
	Vector< Vector<GridSquare> > multiLevelSquares;
        StdMap<Point, GridSquare> squaresMap;
        StdMap<Point, int> squaresIndexMap;
	HashMap<int, GridSquare> indexSquareMap;

	// Points data
	HashMap<int, Point> pointSquareIndexMap;
	HashMap<int, GridSquare> pointSquareMap;
	HashMap<int, Vec> basePoints;
	Vector<int> faceToSquare;

	void computeSquareValues();

	Octree grid_octree;
	Octree base_octree;
	Octree detailed_octree;

	Plane startPlane;
	Plane endPlane;

public:
	Grid(Vector<Vec> & src_spine, double radius, double Length, int sizeOfGrid,
                SmoothStairway * Stair, const StdList<Face*>& MeshFaces, int rotateLeft, int rotateRight);
        Grid(){widthCount = lengthCount = -1; segmentLength = 0.0f;}

	// PROPERTIES
	int widthCount;
	int lengthCount;

	Vector<double> max_height;
	Vector<double> min_height;

	double segmentLength;
	
	//Spline spine;
	Vector<Vec> spinePoints, tangent;
	Vector<LocalFrame> localFrames;
	Vector<CrossSection> sections;

	// GRID RELATED PATTERNS
	Vector2Df sectionsPattern();
	Vector2Df largeScalePattern();

	// DEBUG
	GridSquare * activeGridSquare;
	GridPoint * activeGridPoint;
	Vector<Vec> testPoints;

	// ACCESSORS
	Vector<GridSquare> * getSquares(int level);
        GridSquare * getSquare(int u, int v);
	SquareDimension getSquareDimensions(int u, int v);
	Vec gridPoint(GridSquare * s, int index);
	double getHeightRange(int level);
	GridSquare * getSquareOf(int vindex);

	Vec gridPointNormal(int point_index, const Vec& U, const Vec& V, const Vec& N);
	Vec gridSquareNormal(int point_index);

	Mesh * getBaseMesh();
	Mesh * getDetailedMesh();
	Vector<int> * getSelectedFaces();
	StdSet<int> getGridifiedPoints();
	Plane getMidPlane();

	// MODIFIERS
	void FitNothing();
	void FitCylinder();
        void FitCrossSections();

	// Rotation experiment
	int rotateLeft, rotateRight;

	Vector<ClosedPolygon> polygon;
	Vector<Vector<Vec> > shapes;
	void SmoothCrossSecitons(int numIterations);

	// Most important method !
	void Gridify(Vector<int> & selectedMeshFaces);
	
	// MEAN VALUE COORDINATES
	void MVC(const Vec& p, const Vec q[], Vector<double> & w);
	Vec PointFromSquare(const double w[], GridSquare * square);
	Vec Reconstruct(const double w[], GridSquare * square, double height, double angle, double shift);
	Vec ParameterCoord(const Vector<double> w, int u, int v);

	// PROJECT POINT ON GRID
	GridPoint ProjectOnGrid(const Vec& detailedPoint, const Vec& pointOnSquare, 
		GridSquare * square, int corrPoint, Vector<double>& w);

	HashMap<int, Vec> pointVecMap;

	// VISUALIZATION
	void drawAsGrid();
	void highlightSquare(int u, int v);
	void selectPoint(int index);

	// SELECTION
	void drawPointNames();

	// VISUALIZATION FLAGS
	bool isDrawReconstructedPoints;
	bool isDrawGridLines;
	bool isDrawSpine;
};

#endif // Grid_H
