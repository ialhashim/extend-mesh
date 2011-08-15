#include "GridMeshStretch.h"

#include "Matrixf.h"
using namespace Matrixf;

GridMeshStretch::GridMeshStretch( Grid * src_grid, const Rect & cropArea, Spline * curve, 
								 std::vector<std::vector<Point> > & synthOutput, bool isSynthesizeCS, 
								 bool isBlendCrossSections, int bandSize, bool isChangeProfile, 
								 bool isFillSeams, bool isBlendSeams, bool isSampleSeams )
{
	this->grid = src_grid;
	this->extensionCurve = curve;

	this->start = cropArea.l;
	this->end = cropArea.r;

	this->isFillSeams = isFillSeams;
	this->isCustomProfile = isChangeProfile;
	this->isBlendSeams = isBlendSeams;
	this->isSampleSeams = isSampleSeams;

	// Create empty grid squares
	extensionSize = 0;

	totalHeight = grid->widthCount;
	totalWidth = grid->lengthCount;

	square = Vector<Vector<GridSquare> >(totalHeight, Vector<GridSquare>(totalWidth));

	// Split and Jump locations
	splitIndex = 0;
	jump = extensionSize + splitIndex;

	// Cross-sections at split point 
	CrossSection c1(splitIndex, grid);
	CrossSection c2(splitIndex + 1, grid);
	CrossSection c3(splitIndex + 2, grid);

	originalEndCenter = c1.getCenter();

	// Align extension curve to original shape
	tangentAtSplit = (grid->spinePoints[splitIndex + 1] - grid->spinePoints[splitIndex]).unit();
	extensionCurve->Align(tangentAtSplit);
	extensionCurve->Translate(grid->spinePoints[splitIndex]);

	int numSegments = totalWidth;

	// Find path and tangent from extension curve
	extensionPath = extensionCurve->GetUniformPath( numSegments, extensionCurve->GetPoints() );

	// Find tangents
	extensionTanget = extensionCurve->GetUniformTangent( numSegments, extensionCurve->GetPoints() );

	extensionPath.push_back( extensionPath.back() + (extensionPath.back()-extensionPath[extensionPath.size() - 2]) );
	extensionTanget.push_back( extensionTanget.back() );	// last tangent

	for(int it = 0; it < 4; it++)
	{
		Vector<Vec> smooth = extensionTanget;

		for(int i = 3; i < (int)extensionTanget.size() - 3; i++)
		{
			smooth[i] = ((  extensionTanget[i-3] + extensionTanget[i-2] + 
				extensionTanget[i-1] + extensionTanget[i+3] + 
				extensionTanget[i+2] + extensionTanget[i+1] ) / 6.0f).unit();
		}

		extensionTanget = smooth;
	}

	// Check tangents direction
	if(extensionTanget.front() * c1.getNormal() < 0)
	{
		for(int i = 0; i < (int)extensionPath.size(); i++)
			extensionTanget[i] = -extensionTanget[i];
	}

	// Local frames
	localFrames = LocalFrame::alongTangent( extensionTanget,  c1.getUp() );

	Vec shiftedCenter;
	Rotation endRotation;

	int e = 0;
	int vi = start;
	CrossSection current;

	// Assign stretched cross-sections
	for(int v = 0; v < totalWidth + 1; v++)
	{
		current = CrossSection(v, grid, Color4(255,255,255)); // white

		current.align( extensionTanget[v] );
		current.translate( extensionPath[v] );

		// Add it to GridMesh
		crossSection.push_back(current);
	}

	// Cross-section shape
	extendedSections = resizeAsImage(grid->sectionsPattern(), crossSection.size() + 1, true);
	lastProfileScale = 1.0f;

	// Apply cross-section pattern
	for(int v = 0; v < (int)crossSection.size(); v++)
		crossSection[v].setLengths(column(v, extendedSections));

	// add the squares coordinates
	int cIndex = 0;

	for(int v = 0; v < totalWidth; v++)
	{
		for(int u = 0; u < totalHeight; u++)
		{
			testPoints.push_back(crossSection[v].getCenter());

			c.push_back( crossSection[v].pointAt(u, 0) );
			c.push_back( crossSection[v].pointAt(u, 1) );
			c.push_back( crossSection[v+1].pointAt(u, 1) );
			c.push_back( crossSection[v+1].pointAt(u, 0) );

			square[u][v].setCorners(cIndex, cIndex + 1, cIndex + 2, cIndex + 3);
			square[u][v].setUV(u,v);

			cIndex += 4;
		}
	}

	isReady = false;

	// Default render options
	isDrawWithNormals = false;
	isDrawCrossSections = false;
	isDrawTriangulated = true;
	isDrawColoredPatches = false;

	// Synthesize geometry!
	Synthesize(synthOutput);

	Triangulate();

	this->triangluatedExtension = tri_patch[0].mesh;
	this->triangluatedExtension.clearColors();
	this->triangluatedExtension.computeNormals();
	this->triangluatedExtension.rebuildVBO();

	this->triangluatedExtension.isReady = true;
	this->triangluatedExtension.isVisible = true;

	recon = NULL;
	blender = NULL;

	isReady = true;
}
