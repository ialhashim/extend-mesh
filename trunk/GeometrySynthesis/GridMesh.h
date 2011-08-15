#pragma once

#include "TextureSynthesis/Globals.h"

#include "Transform.h"
#include "Grid.h"
#include "CrossSection.h"
#include "MeshPatch.h"

// Methods for blending the patches together
class Stitcher;
class Reconstructor;
class Blender;

class GridMesh
{
protected:
	Vector<Vector<GridSquare> > square;

	Vector<Vec> c;							// grid square points
	Vector<CrossSection> crossSection;		// cross sections of input and extension
	Vector<LocalFrame> localFrames;

	// Testing profile effect
	bool isCustomProfile;
	float lastProfileScale;

	bool isFillSeams;
	bool isBlendSeams;
	bool isSampleSeams;

	Vector<Vec> points;						// reconstructed synthesized points 
	Vector<float> pointValue;				// their values
	Vector<Vec> pointNormal;				// estimated normal

	int totalHeight;
	int totalWidth;

	int start;
	int end;
	int croppedWidth;
	int extensionSize;
	int splitPoint;
	int jump;
	int splitPrev, splitIndex, splitNext;

	Vec PointFromSquare(const double w[], GridSquare * square);
	Vec Reconstruct(const double w[], GridSquare * square, float height, float angle, float shift);
	Vec SquareNormal( GridSquare * square );

	Grid * grid;

	Vector<Vector<Point> > synth;
	Vector<Vector<Point> > synthPatch;
	Vector<Vector<Point> > synthPatchCopy;

	// PATCHES & Blending Ops.
	Vector<MeshPatch> tri_patch;

	// Experiments on stitching
	Stitcher * stitcher;
	Reconstructor * recon;
	Blender * blender;

	// Patch extraction routines, from synthesized image
	void getAllPatches();
	Vector<SimpleSquare> getNextPatch();
	void findAllSquaresInPatch(Vector<SimpleSquare> & ids, int u, int v);
	void CreateFullPatches();

	// Border finding routines
	Vector<Vector<SimpleSquare> > unroll(const Vector<SimpleSquare> & patch, int & start_x, int & start_y);
	Vector<SimpleSquare> findPatchBorders( const Vector<Vector<SimpleSquare> > & patch, int thickness = 1);

	Vector<Vector<HashMap<int, Vec> > > recon_points; // reconstructed points

	// Extension properties
	Vector<Vec> extensionPath;
	Vector<Vec> extensionTanget;
	Spline * extensionCurve;

	Vector<Vec> extendedSpine;
	Vector<Vec> tangent;

	// Triangulation
	MeshPatch findPatches();

public:
	GridMesh(Grid * src_grid, const Rect & cropArea,
		Spline * curve, std::vector<std::vector<Point> > & synthOutput,
		bool isSynthesizeCS,  bool isBlendCrossSections, int bandSize, bool isChangeProfile, 
		bool isFillSeams, bool isBlendSeams, bool isSampleSeams);
	GridMesh(){};
	~GridMesh();

	void Synthesize(std::vector<std::vector<Point> > & synthOutput);
	void Triangulate();
	void BlendPatches();

	// End transformation & cut points
	Transformation endTransform();
	void findCutPoints();
	StdSet<int> firstCut, lastCut;
	StdList<int> pointIdsFirst, pointIdsLast;

	MeshPatch * getFirstPatch();
	MeshPatch * getLastPatch();
	MeshPatch * getPatch(int patchId);

	Grid * GetGrid();

	// Attachemnt info
	Vec originalEndCenter;
	Vec modifiedEndCenter;
	Rotation lastRotation();
	Vec tangentAtSplit;

	// Grid based patterns
	MatrixXf extendedSections;

	// VISUALIZATION
	Mesh triangluatedExtension;
	void draw();
	void drawTriangulated();

	bool isReady;
	bool isDrawWithNormals;
	bool isDrawCrossSections;
	bool isDrawTriangulated;
	bool isDrawColoredPatches;

	void showHideTriangulated();
	void ToggleWireframe();
	void ToggleShowColoredPatches();

	// Testing stitching
	void outputGridMesh();

	GridSquare * getSquareFromStart(int src_u, int start_u, int src_v, int start_v);

	// debug
	Vector<Vec> testPoints;
	Vector<CrossSection> special_cs;
	Vec n1;
	Vec n2;

	friend class Seam;
	friend class Stitcher;
	friend class Reconstructor;
	friend class Blender;
};
