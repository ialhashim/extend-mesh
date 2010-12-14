#include "GridMesh.h"

#include "SimpleDraw.h"
#include "HashTable.h"
#include "Matrixf.h"
using namespace Matrixf;

// Patch blending methods
#include "Stitcher.h"
#include "Reconstructor.h"
#include "Blender.h"

#include "ExtendMeshHeaders.h"

#include <sstream>
using namespace std;

GridMesh::GridMesh(Grid * src_grid, const Rect & cropArea,
				   Spline * curve, Vector2DPoint & synthOutput, 
				   bool isSynthesizeCS,  bool isBlendCrossSections, int bandSize, bool isChangeProfile, 
				   bool isFillSeams, bool isBlendSeams, bool isSampleSeams)
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
	extensionSize = synthOutput[0].size() - cropArea.width();

	totalHeight = grid->widthCount;
	totalWidth = cropArea.width() + extensionSize - 1;

        square = Vector<Vector<GridSquare> >(totalHeight, Vector<GridSquare>(totalWidth));

	// Split and Jump locations
	splitIndex = start + (ceil(cropArea.width() * 0.5f) - 1);
	jump = extensionSize + splitIndex;

	// Cross-sections at split point 
	CrossSection c1(splitIndex, grid);
	CrossSection c2(splitIndex + 1, grid);
	CrossSection c3(splitIndex + 2, grid);

	originalEndCenter = c1.getCenter();

	// Align extension curve to original shape
	tangentAtSplit = grid->spinePoints[splitIndex + 1] - grid->spinePoints[splitIndex];
	extensionCurve->Align(tangentAtSplit);
	extensionCurve->Translate(grid->spinePoints[splitIndex]);

	// Find path and tangent from extension curve
	float segmentLength = grid->segmentLength;

	extensionPath = extensionCurve->GetUniformPath( segmentLength );

	// Find tangents
	extensionTanget = extensionCurve->GetUniformTangent( segmentLength );

	extensionTanget.push_back(extensionTanget.back()); // last tangent

	for(int it = 0; it < 4; it++){
		Vector<Vec> smooth = extensionTanget;

                for(int i = 3; i < (int)extensionTanget.size() - 3; i++)
			smooth[i] = ((  extensionTanget[i-3] + extensionTanget[i-2] + extensionTanget[i-1] 
						  + extensionTanget[i+3] + extensionTanget[i+2] + extensionTanget[i+1] ) / 6.0f).unit();
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

	// Start the extension process
	for(int v = start; v < totalWidth + start + 1; v++)
	{
		if(v < splitIndex)
		{
			// Real start part
			current = CrossSection(vi++, grid, Color4(255,50,50)); // red
		}
		else if(e > extensionSize)
		{
			// Real end part
			current = CrossSection(vi++, grid, Color4(255,50,255)); // purple

			current.rotateAround(originalEndCenter, endRotation);
			current.translateBy(shiftedCenter);
		}
		else
		{
			// Synthetic parts
			current = CrossSection(0, crossSection.back(), crossSection.back(), Color4(50,255,50)); // green-ish

			// hack?
                        if(e+1 >= (int)extensionPath.size())
			{
				Vec delta = extensionPath.back() - extensionPath[extensionPath.size() - 2];

				extensionPath.push_back(extensionPath.back() + delta);
				extensionTanget.push_back(extensionTanget.back());
				localFrames.push_back(localFrames.back());
			}

			current.align( extensionTanget[e] );
			current.translate( extensionPath[e] );

			if(e == extensionSize)
			{
				modifiedEndCenter = extensionPath[e+1];
				shiftedCenter = modifiedEndCenter - originalEndCenter;

				// Find rotation needed to align last cross-sections
				endRotation = LocalFrame::rotation(localFrames[splitIndex], localFrames[e]);

				// Hack
				CrossSection nextCS(vi+1, grid, Color4(255,50,255));
				nextCS.rotateAround(originalEndCenter, endRotation);
				nextCS.translateBy(shiftedCenter);
				Rotation adjRot(nextCS.getNormal(), localFrames[e+1].n);

				endRotation = adjRot * endRotation;
			}
			
			e++;
		}

		// Add it to GridMesh
		crossSection.push_back(current);

		// Add special cross-sections
		if (v == splitIndex - 1 || v == splitIndex + extensionSize)
		{
			special_cs.push_back(current);
		}
	}

	// Deal with cross-sections shapes
	if(isSynthesizeCS)
	{
                MatrixXf csMatrix = fromVector2Df(grid->sectionsPattern());

		if(isBlendCrossSections)
		{
                        extendedSections = tileBlendFromMap(csMatrix, bandSize, grid->item_int["tileCount"]);
		}
		else
		{
			// Correspond cross sections with synthesized patches
                        extendedSections = synthFromMap(csMatrix, synthOutput);
		}
	}
	else
	{
		// Interpolate CrossSection shapes
		extendedSections = resizeAsImage(grid->sectionsPattern(), crossSection.size() + 1, true);
	}

	if(isCustomProfile)
	{
		// Experiment: add randomness
		float range = extendedSections.array().maxCoeff() - extendedSections.array().minCoeff();

		int rows = extendedSections.rows();
		int cols = extendedSections.cols();

		MatrixXf pattern = Matrixf::pattern(cols, rows, "smooth-random", 0.2f);
		//MatrixXf pattern = Matrixf::pattern(cols, rows, "wave", 12);

		pattern.array() *= 0.75;
		
		extendedSections.array() += (pattern.array() * range);

		/*for(int v = 0; v <= totalWidth; v++)
		{
			lastProfileScale = (float) (v * 0.9f) / (totalWidth) ;

			crossSection[v].scaleBy(lastProfileScale);
			crossSection[v].translate(crossSection[v].getCenter() * (1.0f + (lastProfileScale / 2.0f)));
		}*/
	}
	else
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
			c.push_back( crossSection[v].pointAt(u, 0) );
			c.push_back( crossSection[v].pointAt(u, 1) );
			c.push_back( crossSection[v+1].pointAt(u, 1) );
			c.push_back( crossSection[v+1].pointAt(u, 0) );

			square[u][v].setCorners(cIndex, cIndex + 1, cIndex + 2, cIndex + 3);
			square[u][v].setUV(u,v);

			cIndex += 4;
		}
	}

	printf("\nStart index = %d \t End index = %d \n", start, end);
	printf("Split Point = %d \t Extension Size = %d \n", splitIndex, extensionSize);
	printf("Num cross sections = %d \n\n", crossSection.size());

	isReady = false;

	this->stitcher = NULL;
	this->recon = NULL;
	this->blender = NULL;

	// Default render options
	isDrawWithNormals = true;
	isDrawCrossSections = false;
	isDrawTriangulated = false;
	isDrawColoredPatches = false;

	// Synthesize geometry!
	Synthesize(synthOutput);

	// Triangulate
	Triangulate();

	// Used for blending
	if(isSampleSeams || isBlendSeams)	
		CreateFullPatches();

	// Stitch patches together
	BlendPatches();
}

void GridMesh::Synthesize(Vector<Vector<Point> > & synthOutput)
{
	stats["reconstruction"] = Stats("Reconstruction");

	this->synth = synthOutput;

	this->synthPatch = synth;
	this->synthPatchCopy = synth;

	int synth_u, synth_v;

	GridSquare * src_square;

	// Copy square info from the synthesis process output
	for(int v = 0; v < totalWidth; v++)
	{
		for(int u = 0; u < totalHeight; u++)
		{
			synth_v = synth[u][v].x;
			synth_u = synth[u][v].y;

			src_square = grid->getSquare(synth_u, synth_v + start);

			if(src_square)
			{
				square[u][v].points = src_square->points;
				square[u][v].pointIndices = src_square->pointIndices;

				square[u][v].id = src_square->id;
				square[u][v].u = src_square->u;
				square[u][v].v = src_square->v;
			}
		}
	}

        // Reconstruction of elements
        recon_points = Vector<Vector<HashMap<int, Vec> > >(totalHeight, Vector<HashMap<int, Vec> >(totalWidth));

	// For each square, reconstruct all points
	for(int v = 0; v < totalWidth; v++)
	{
		for(int u = 0; u < totalHeight; u++)
		{
			for(Vector<GridPoint>::iterator p = square[u][v].points.begin(); p != square[u][v].points.end(); p++)
			{
                                Vec point = Reconstruct(p->w, &square[u][v], p->h, p->shiftAngle, p->shiftMagnitude);

				// Used for triangulation
				recon_points[u][v][p->corrPoint] = point;
			}
		}
	}

	isReady = true;
}


void GridMesh::Triangulate()
{
	this->isReady = false;

	getAllPatches();

	Mesh * mesh = grid->getBaseMesh();

	// Copying triangulation from mesh
	CreateTimer(timer);

	// Get triangles that are in current patch
	Vector<int> faces = *grid->getSelectedFaces();

        int v0, v1, v2;
	Face * f;

	// For each patch
        for(int w = 0; w < (int)tri_patch.size(); w++)
	{
                IntSet patchPoints;	// store points in this patch

		// Get the points indices from patch's squares
		for(Vector<SimpleSquare>::iterator it = tri_patch[w].patch.begin(); it != tri_patch[w].patch.end(); it++)
		{
			GridSquare * currSquare = &square[it->u][it->v];

			for(Vector<int>::iterator q = currSquare->pointIndices.begin(); q != currSquare->pointIndices.end(); q++)
			{
				patchPoints.insert(*q);

				tri_patch[w].insertPoint(*q, recon_points[it->u][it->v][*q]);
			}

			it->parentPatch = w;
		}

		// If all points of a face is within this patch, include these points
                for(int findex = 0; findex < (int)faces.size(); findex++)
		{
			f = mesh->f(faces[findex]);

			v0 = f->vIndex[0];
			v1 = f->vIndex[1];
			v2 = f->vIndex[2];

			bool v0_ok = patchPoints.has(v0);
			bool v1_ok = patchPoints.has(v1);
			bool v2_ok = patchPoints.has(v2);

			bool face_ok = v0_ok && v1_ok && v2_ok;

			if(face_ok)
				tri_patch[w].insertFace(v0, v1, v2);
			else
			{
				if(v0_ok) tri_patch[w].insertNonTriangulated(v0);
				if(v1_ok) tri_patch[w].insertNonTriangulated(v1);
				if(v2_ok) tri_patch[w].insertNonTriangulated(v2);
			}
		}
	}

        printf(".Triangulated patches done. (%d ms)\n", (int)timer.elapsed());
        CreateTimer(patchTimer);

	// Create patches as triangulated meshes
	#pragma omp parallel for
        for(int w = 0; w < (int)tri_patch.size(); w++)
		tri_patch[w].makeMesh();

        printf(".Finalizing patches. (%d ms)\n", (int)patchTimer.elapsed());

	isDrawTriangulated = true;

	stats["reconstruction"].end();
}

void GridMesh::BlendPatches()
{
	stats["stitching"] = Stats("Stitching");

	// Simple Stitching:
	stitcher = new Stitcher(this, isFillSeams);

	// Reconstruction (interpolation):
	if(isSampleSeams)
	{
		CreateTimer(timer);

		recon = new Reconstructor(this);

                printf("\n.Reconstructor. (%d ms)\n", (int)timer.elapsed());
	}

	// Using cross-sections blending
	if(isBlendSeams)
	{
		blender = new Blender(this);
	}

	this->isReady = true;

	stats["stitching"].end();
}

void GridMesh::CreateFullPatches()
{
	printf("\nfp b/d Mesh(es)..");
	CreateTimer(timer);

	Vector<SimpleSquare> fullPatchStarts;

	// Find vertices & faces of the grid's full patch
	StdSet<int> fullPatchVerts = grid->getGridifiedPoints();
	StdSet<Face*> fullPatchFaces = grid->getBaseMesh()->getFacesFromVertices(fullPatchVerts, true);
	int n = fullPatchVerts.size();

        for(int w = 0; w < (int)tri_patch.size(); w++)
	{
		SimpleSquare s = tri_patch[w].patch.front();

		fullPatchStarts.push_back(SimpleSquare(-1, -1, s.u - s.src_u, s.v - s.src_v));

		//empty full patch meshes
		tri_patch[w].fpbMesh = Mesh(n);
		tri_patch[w].fpdMesh = Mesh(n);
	}

	// Create full patch meshes
        for(int i = 0; i < (int)tri_patch.size(); i++)
	{
		SimpleSquare start = fullPatchStarts[i];

		// Add vertices for full patch mesh
		VertexMap vertsMap;
		StdSet<int>::iterator fpvi = fullPatchVerts.begin();
		int vIndex = 0;

		Vector<Vec> parameterPoint;

		for(int vi = 0; vi < n; vi++)
		{
			int corr = *(fpvi++);

			GridSquare * squareInGrid = grid->getSquareOf(corr);
			GridSquare * squareInGridMesh = getSquareFromStart(squareInGrid->u, start.src_u, squareInGrid->v, start.src_v);

			if(squareInGridMesh != NULL)
			{
				GridPoint * p = squareInGrid->getPointCorr(corr);

				Vec point = PointFromSquare(p->w, squareInGridMesh);

				tri_patch[i].fpbMesh.addVertex(point, vIndex);
				tri_patch[i].fpdMesh.addVertex(Reconstruct(p->w, squareInGridMesh, p->h, p->shiftAngle, p->shiftMagnitude), vIndex);

				if(isSampleSeams)
				{
					// Parameter points
					Vec uvPos = grid->pointVecMap[p->corrPoint];
					uvPos.x = (start.src_v) + (uvPos.x * grid->lengthCount);
					uvPos.y = (start.src_u) + (uvPos.y * grid->widthCount);
					parameterPoint.push_back(uvPos);
				}

				vertsMap[corr] = vIndex++;
			}
		}

		// for parameter mesh creation
		Vector<Triangle> ptris;

		// Add faces for full patch mesh
		int fi = 0;
		for(StdSet<Face*>::iterator f = fullPatchFaces.begin(); f != fullPatchFaces.end(); f++)
		{
			if(MAP_HAS(vertsMap, (*f)->vIndex[0]) && MAP_HAS(vertsMap, (*f)->vIndex[1]) && 
				MAP_HAS(vertsMap, (*f)->vIndex[2]))
			{
				int vi1 = vertsMap[(*f)->vIndex[0]];
				int vi2 = vertsMap[(*f)->vIndex[1]];
				int vi3 = vertsMap[(*f)->vIndex[2]];

				tri_patch[i].fpbMesh.addFace(vi1, vi2, vi3, fi);
				tri_patch[i].fpdMesh.addFace(vi1, vi2, vi3, fi);

				if(isSampleSeams)
				{
					// Parameter mesh
					ptris.push_back(Triangle(parameterPoint[vi1],parameterPoint[vi2],parameterPoint[vi3], fi));
				}

				fi++;
			}
		}

		if(isSampleSeams)
		{
			// Parameter domain code:

			// Adjust looped parameter triangles
			float threshold = grid->widthCount * 0.5f; // half the height
			
			Vector<Triangle> tempTris;

			for(Vector<Triangle>::iterator t = ptris.begin(); t != ptris.end(); t++)
			{
				Triangle other;
				t->unloop(threshold, other);

				// Add both to tempTris
				tempTris.push_back(*t);
				if(other.flag == MODIFIED_TRI)	tempTris.push_back(other);
			}

                        tri_patch[i].parameterTriangles = tempTris;

			// Build octree of paramter domain
                        tri_patch[i].parameterOctree.initBuild(tri_patch[i].parameterTriPointers(), 30 );
		}
	
		// Compute normals for detailed
		tri_patch[i].fpdMesh.computeNormals();
	}

        printf(".Done (%d ms).\n", (int)timer.elapsed());
}

void GridMesh::getAllPatches()
{
	CreateTimer(timer);

	Vector<SimpleSquare> curr_patch = getNextPatch();

	int maxNumVerts = grid->getSelectedFaces()->size() * 3;

	// while we are finding new patches
	while(curr_patch.size())
	{
		// create a triangular patch structure
		tri_patch.push_back(MeshPatch(maxNumVerts, tri_patch.size())) ; // generous estimation...

		// insert the patch
		tri_patch.back().patch = curr_patch;

		// find the next one
		curr_patch = getNextPatch();
	}

	if(isSampleSeams)
	{
                CreateTimer(borderTimer);

                #pragma omp parallel for
                for(int i = 0; i < (int)tri_patch.size(); i++)
		{
			// Unroll the patch
			tri_patch[i].unrolledPatch = unroll(tri_patch[i].patch, tri_patch[i].start_x, tri_patch[i].start_y);
			tri_patch[i].border = findPatchBorders(tri_patch[i].unrolledPatch, 2);
		}
                printf(".Border (%d).", (int)borderTimer.elapsed());
	}

        printf(".Patches found (%d) (%d ms).", tri_patch.size(), (int)timer.elapsed());
}

Vector<SimpleSquare> GridMesh::getNextPatch()
{
	Vector<SimpleSquare> result;

	for(int v = 0; v < totalWidth; v++){
		for(int u = 0; u < totalHeight; u++){
			if(synthPatch[u][v].x >= 0 && synthPatch[u][v].y >= 0)
			{
				findAllSquaresInPatch(result, u, v);
				return result;
			}
		}
	}

	return result;
}

void GridMesh::findAllSquaresInPatch(Vector<SimpleSquare> & squaresInPatch, int first_u, int first_v)
{
	Stack<Point> stack;

	stack.push(Point(first_u, first_v));

	while(!stack.empty())
	{
		// Get next square
		Point p = stack.top();	stack.pop();
		int u = p.x , v = p.y;

		if(synthPatch[u][v].x >= 0 && synthPatch[u][v].y >= 0 && u < totalHeight)
		{
			// Copy src coordinates
			int src_x = synthPatch[u][v].x;
			int src_y = synthPatch[u][v].y;	

			// add it
			squaresInPatch.push_back(SimpleSquare(u, v, src_y, src_x));

			// mark as visited
			synthPatch[u][v].x = -1;
			synthPatch[u][v].y = -1;

			// Visit right, bottom, left, top (add to stack)
			if(v + 1 < totalWidth  && (src_x + 1) == synthPatch[u  ][v+1].x && src_y == synthPatch[u  ][v+1].y)	stack.push( Point (u, v + 1 ));
			if(u + 1 < totalHeight && (src_y + 1) == synthPatch[u+1][v  ].y && src_x == synthPatch[u+1][v  ].x)	stack.push( Point (u + 1, v	));
			if(v-1 >= 0 && (src_x - 1) == synthPatch[u  ][v-1].x && src_y == synthPatch[u  ][v-1].y)			stack.push( Point (u, v - 1 ));		
			if(u-1 >= 0 && (src_y - 1) == synthPatch[u-1][v  ].y && src_x == synthPatch[u-1][v  ].x)			stack.push( Point (u - 1, v	));
		}
	}
}

Vector<SimpleSquare> GridMesh::findPatchBorders( const Vector<Vector<SimpleSquare> > & inputPatch , int thickness )
{
	Vector<SimpleSquare> border;

	// Convert to 2D grid
        Vector<Vector<SimpleSquare> > patch = inputPatch; // local copy
        Vector<Vector<SimpleSquare> > currPatch = patch;

	int width = patch[0].size();
	int height = patch.size();

	int i0, i1, j0, j1;

	bool isActualPatchHeight = (height == grid->widthCount);

	for(int t = 0; t < thickness; t++)
	{
		currPatch = patch;

		int i = 0, j = 0;
		bool isBorder = false;

		for(i = 0; i < height; i++){
			for(j = 0; j < width; j++){

				isBorder = false;

				if(!currPatch[i][j].isEmpty())
				{
					if(j == 0 || j == width - 1 || width < 3 || height < 3)	// thin parts and border cases
						isBorder = true;
					else
					{
						i0 = i - 1; i1 = i + 1;
						j0 = Max(0, j - 1); j1 = Min(width - 1, j + 1);

						if(isActualPatchHeight){
							i0 = (i + grid->widthCount - 1) % grid->widthCount;
							i1 = (i + 1) % grid->widthCount;
						}
						else
							if (i0 == -1 || i1 == height) isBorder = true;
						
						if(!isBorder)
						{
							if(currPatch[i0][j0].isEmpty()	||	currPatch[i0][j ].isEmpty()	|| currPatch[i0][j1].isEmpty()
							|| currPatch[i1][j0].isEmpty()	||	currPatch[i1][j ].isEmpty()	|| currPatch[i1][j1].isEmpty()
							|| currPatch[i ][j0].isEmpty()	||	currPatch[i ][j1].isEmpty())
								isBorder = true;
						}
					}

					if(isBorder)
					{
						border.push_back(currPatch[i][j]);

						// Unset
						patch[i][j].u = -1;
						patch[i][j].v = -1;
					}
				}
			}
		}
	}

	return border;
}

Vector<Vector<SimpleSquare> > GridMesh::unroll( const Vector<SimpleSquare> & patch, int & start_x, int & start_y )
{
	start_y = INT_MAX; start_x = INT_MAX;
	int end_y = INT_MIN, end_x = INT_MIN;

	for(Vector<SimpleSquare>::const_iterator s = patch.begin(); s!= patch.end(); s++)
	{
		start_y = Min(s->u, start_y);
		start_x = Min(s->v, start_x);

		end_y = Max(s->u, end_y);
		end_x = Max(s->v, end_x);
	}

	int width = (end_x - start_x) + 1;
	int height = (end_y - start_y) + 1;

	// Create empty field
        Vector<Vector<SimpleSquare> > result(height, Vector<SimpleSquare>(width));

	// unroll list into field
	for(Vector<SimpleSquare>::const_iterator s = patch.begin(); s!= patch.end(); s++)
		result[s->u - start_y][s->v - start_x] = *s;

	return result;
}

MeshPatch * GridMesh::getFirstPatch()
{
	return &tri_patch.front();
}

MeshPatch * GridMesh::getLastPatch()
{
	return &tri_patch.back();
}

MeshPatch * GridMesh::getPatch(int patchId)
{
	return &tri_patch[patchId];
}

void GridMesh::findCutPoints()
{
	double minDist;

	MeshPatch * lastPatch = &tri_patch.back(), *firstPatch = &tri_patch.front();

	Mesh * lastPart = &lastPatch->mesh, *firstPart = &firstPatch->mesh;

	Vector<int> borderFirst = firstPatch->getBorder();

	// Find first cut points, by finding closest point to first cross-section
	int closestIndexFirst = 0;
	minDist = DBL_MAX;
	Vec firstPoint = crossSection.front().getCenter();

	foreach(int vi, borderFirst)
	{
		double dist = (firstPoint - firstPart->ver(vi)).norm();

		if(dist < minDist){
			minDist = dist;
			closestIndexFirst = vi;
		}
	}

	pointIdsFirst = firstPart->getBoundry(closestIndexFirst, true);

	foreach(int vi, pointIdsFirst)
	{
		int corrIndex = firstPatch->usedIndexToCorr[vi];
		firstCut.insert(corrIndex);
	}

	// Find last cut points, same as above but for last part
	Vector<int> borderLast = lastPart->getBorderVertices();

	int closestIndexLast = 0;
	minDist = DBL_MAX;
	Vec lastPoint = crossSection.back().getCenter();

	foreach(int vi, borderLast)
	{
		double dist = (lastPoint - lastPart->ver(vi)).norm();

		if(dist < minDist){
			minDist = dist;
			closestIndexLast = vi;
		}
	}

	pointIdsLast = lastPart->getBoundry(closestIndexLast, true);

	foreach(int vi, pointIdsLast)
	{
		int corrIndex = lastPatch->usedIndexToCorr[vi];
		lastCut.insert(corrIndex);
	}
}

Transformation GridMesh::endTransform()
{
	// Get mesh and patches
	Mesh * mesh = grid->getDetailedMesh();
	MeshPatch * lastPatch = &tri_patch.back();
	Mesh * lastPart = &lastPatch->mesh;

	// This shouldn't happen, but added to avoid crashing on other test cases
	if(lastPart->numberOfVertices() < 3)
		return Transformation(1, Rotation(), Vec());

	// Find cut points
	findCutPoints();

	Vector<Vec> partPoints, meshPoints;

	foreach(int vi, pointIdsLast)
	{
		int vIndex = vi;
		int corrIndex = lastPatch->usedIndexToCorr[vIndex];

		meshPoints.push_back(*mesh->v(corrIndex));
		partPoints.push_back(*lastPart->v(vIndex));

		//mesh->testVertex.push_back(meshPoints.back());
		//mesh->testVertex.push_back(partPoints.back());
	}

	Transformation tr = Transform3D::findFrom(meshPoints, partPoints);

	if(isCustomProfile)
	{
		tr.S = lastProfileScale + 1;
	}

	return tr;
}

GridSquare * GridMesh::getSquareFromStart(int src_u, int start_u, int src_v, int start_v)
{
	int u = ((src_u + start_u) + totalHeight) % totalHeight;
	int v = (src_v + start_v);

	if(v < totalWidth)
		return &square[u][v];
	else
		return NULL;
}

Vec GridMesh::PointFromSquare(const double w[], GridSquare * square)
{
	Vec p;

	if(w && square)
	{
		for(int i = 0; i < 4; i++)
		{
			p.x += c[square->p[i]].x * w[i];
			p.y += c[square->p[i]].y * w[i];
			p.z += c[square->p[i]].z * w[i];
		}
	}

	return p;
}

Vec GridMesh::Reconstruct(const double w[], GridSquare * square, float height, float angle, float shift)
{
	Vec v1 = (c[square->p[1]] - c[square->p[0]]).unit();
	Vec v2 = (c[square->p[3]] - c[square->p[0]]).unit();

	Vec squareNormal = (v1 ^ v2).unit();
	//Vec squareNormal = ((fNormal[square->face1] + fNormal[square->face2]) / 2.0f).unit();

	Vec pointOnSquare = PointFromSquare(w, square);

	Vec point = pointOnSquare + (height * squareNormal);

	float shiftAngle = angle;
	float shiftMagnitude = shift;

	Vec ref = v2;
	Vec rotatedRef = Rotation(squareNormal, shiftAngle).rotate(ref);

	point += rotatedRef * shiftMagnitude;

	return point;
}

Rotation GridMesh::lastRotation()
{
	return Rotation(crossSection[splitIndex].getNormal(), crossSection[jump - 1].getNormal());
}

Vec GridMesh::SquareNormal( GridSquare * square )
{
	Vec v1 = (c[square->p[1]] - c[square->p[0]]).unit();
	Vec v2 = (c[square->p[3]] - c[square->p[0]]).unit();

	return (v1 ^ v2).unit();
}

Grid * GridMesh::GetGrid()
{
	return grid;
}

void GridMesh::outputGridMesh()
{
	Mesh m;

        for(int i = 0; i < (int)tri_patch.size(); i++)
		m.mergeWith(tri_patch[i].mesh);

	m.saveToFile("output_gridMesh.obj");

	grid->saveToFile("output_grid.obj");
	grid->getBaseMesh()->saveToFile("output_base.obj");
	grid->getDetailedMesh()->saveToFile("output_detailed.obj");
}

void GridMesh::drawTriangulated()
{
	// draw regular mesh
	this->triangluatedExtension.draw();

	// Draw cloud of non-triangulated points
	//currPatch->drawPointCloud();
}

void GridMesh::showHideTriangulated()
{
	triangluatedExtension.isDrawSmooth = !triangluatedExtension.isDrawSmooth;
}

void GridMesh::ToggleWireframe()
{
	triangluatedExtension.isDrawWireframe = !triangluatedExtension.isDrawWireframe;
}

void GridMesh::ToggleShowColoredPatches()
{
	isDrawColoredPatches = !isDrawColoredPatches;
}

void GridMesh::draw()
{
	if(!isReady)
		return;

	// Debug
	//SimpleDraw::IdentifyLine( grid->spinePoints[splitPrev],  grid->spinePoints[splitPrev] + extendDirection);
	//SimpleDraw::IdentifyConnectedPoints( grid->spinePoints, 0,1,0 );
	//SimpleDraw::IdentifyConnectedPoints(testPoints, 1,1,0 );

	/*glColor4f(1,1,1, 0.5f);
	glBegin(GL_QUADS);
	for(int v = 0; v < totalWidth; v++){
		for(int u = 0; u < totalHeight; u++){
			glVertex3fv(c[square[u][v].p[0]]);
			glVertex3fv(c[square[u][v].p[1]]);
			glVertex3fv(c[square[u][v].p[2]]);
			glVertex3fv(c[square[u][v].p[3]]);			
		}
	}
	glEnd();*/

	if(isDrawColoredPatches)
	{
		// Draw each patch in a different color
                for(int w = 0; w < (int)tri_patch.size(); w++)
		{
			Color4 color = tri_patch[w].falseColor;
			Vector<SimpleSquare> * patch = &tri_patch[w].patch;

			// DRAW SQUARES
                        Vector<Vector<Vec> > patchSquares(patch->size(), Vector<Vec>(4));
			int sc = 0;
			for(Vector<SimpleSquare>::iterator it = patch->begin(); it != patch->end(); it++){
				GridSquare * s = &square[it->u][it->v];
				patchSquares[sc][0] = c[s->p[0]];	patchSquares[sc][1] = c[s->p[1]];
				patchSquares[sc][2] = c[s->p[2]];	patchSquares[sc][3] = c[s->p[3]];
				sc++;
			}
			SimpleDraw::DrawSquares(patchSquares, true, color.r(), color.g(), color.b(), 0.25f);

			// DRAW BORDER
			/*Vector<SimpleSquare> border = tri_patch[w].border;
			Vector<Vector<Vec>> borderSquare; sc = 0;
			for(Vector<SimpleSquare>::iterator it = border.begin(); it != border.end(); it++){
				GridSquare * s = &square[it->u][it->v];
				borderSquare.push_back(Vector<Vec>(4));
				borderSquare[sc][0] = c[s->p[0]];	borderSquare[sc][1] = c[s->p[1]];
				borderSquare[sc][2] = c[s->p[2]];	borderSquare[sc][3] = c[s->p[3]];
				sc++;
			}*/
			//glClear(GL_DEPTH_BUFFER_BIT);
			//SimpleDraw::DrawSquares(borderSquare, true, color.r(), color.g(), color.b(), 1.0f);
			
			glColor3f(color.r(), color.g(), color.b());

			//tri_patch[w].fpbMesh.drawSimpleWireframe(false);
			tri_patch[w].fpdMesh.drawSimpleWireframe(false);

			/*// PARAMTER TRIANGLES VISUALIZATION
			for(int w = 0; w < tri_patch.size(); w++){
				foreach(Triangle t, tri_patch[w].parameterTriangle){
					Color4 color = tri_patch[w].falseColor;
					SimpleDraw::DrawTriangle(t.p1, t.p2, t.p3, color.r(), color.g(), color.b());
				}
			}*/
		}
	}

	if(isDrawCrossSections)
	{
                for(int i = 0; i < (int)crossSection.size(); i++)
			crossSection[i].draw(2.0f);

                for(int i = 0; i < (int)special_cs.size(); i++)
			special_cs[i].draw(6.0f);

                int numFrames = Min((int)localFrames.size(), (int)extensionPath.size());
		for(int i = 0; i < numFrames; i++)
			localFrames[i].draw(extensionPath[i]);

		// Special points
		glDisable(GL_LIGHTING);
		glPointSize(14);
		glBegin(GL_POINTS);
		glColor3f(1,0,0);
		glVertex3fv(originalEndCenter);

		glColor3f(0,0,1);
		glVertex3fv(modifiedEndCenter);
		glEnd();
	}

	if(isReady)
	{
		glDisable(GL_LIGHTING);

		glEnable(GL_POINT_SMOOTH);
		glDisable(GL_BLEND);

		//glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		//glEnable(GL_BLEND);

		glPointSize(8.0f);

		if(isDrawWithNormals)
		{
			glEnable(GL_LIGHTING);
			glColor3f(1,1,1);
		}

		if(isDrawTriangulated)
		{
			drawTriangulated();
		}
		else
		{
			// Draw reconstructed points
			glBegin(GL_POINTS);
                        for(int i = 0; i < (int)points.size(); i++)
			{
				if(isDrawWithNormals)
				{
					if(pointNormal.size())
						glNormal3fv(pointNormal[i]);
				}
				else
				{
					// Just use the height color
					glColor3f(pointValue[i], pointValue[i], pointValue[i]);
				}

				glVertex3fv(points[i]);
			}
			glEnd();
		}
	}

	if(recon != NULL)
		recon->draw();

	if(blender != NULL && blender->isDone == true)
		blender->draw();

}

GridMesh::~GridMesh()
{
	/*if(this->stitcher != NULL) delete stitcher;
	if(this->recon != NULL) delete recon;
	if(this->blender != NULL) delete blender;*/
}
