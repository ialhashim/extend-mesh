#include "Grid.h"

#include "SimpleDraw.h"

#include "ExtendMeshHeaders.h"

Grid::Grid(Vector<Vec> & src_spine, double radius, double Length, int sizeOfGrid, 
		   SmoothStairway * Stair, StdList<Face*> & MeshFaces, int rotateLeft, int rotateRight)
{
	this->isReady = false;

	this->stair = Stair;
	this->meshFaces = &MeshFaces;

	if (src_spine.size() < 2 || sizeOfGrid < 3)	return;

	this->gridSize = sizeOfGrid;

	// Aspect ratio
	double ratio = 1.0;
	Length = Length * ratio;

	// Compute dimensions and properties
	this->r = radius;
	this->length = Length;
	this->width = 2.0 * M_PI * r;

	double gridSquareSize = width / gridSize;

	widthCount = width / gridSquareSize;
	lengthCount = length / gridSquareSize;

	multiLevelSquares.push_back(Vector<GridSquare>());
	Vector<GridSquare> * squares = &multiLevelSquares[0];

	squares->reserve(widthCount * lengthCount);

	// Number of Sides
	this->numSides = widthCount;
	//double theta = (2.0 * M_PI) / numSides;

	// Height Segments
	this->numStrips = lengthCount;

	// Relax spine in case of very close points (this is a hack)
	for(int r = 0; r < 30; r++){
		Vector<Vec> relaxedSpine = src_spine;
		for(int i = 1; i < (int)src_spine.size() - 1; i++)
			relaxedSpine[i] = (src_spine[i-1] + src_spine[i+1]) * 0.5;
		src_spine = relaxedSpine;
	}

	// Create a spine with uniform distances using a arc length parameterized bezier spline
	spinePoints = Spline::GetUniformPath( lengthCount, src_spine );

	segmentLength = (spinePoints[1] - spinePoints[0]).norm();

	// First tangent needed for reference vectors
	tangent = Spline::GetUniformTangent( lengthCount, src_spine );

	// Relax tangents ?
	for(int r = 0; r < 20; r++){
		Vector<Vec> relaxedTangent = tangent;
		for(int i = 1; i < (int)relaxedTangent.size() - 1; i++)
			relaxedTangent[i] = ((tangent[i-1] + tangent[i+1]) * 0.5).unit();
		relaxedTangent[0] = relaxedTangent[1];
		relaxedTangent.back() = relaxedTangent[relaxedTangent.size() - 2];
		tangent = relaxedTangent;
	}

	// Local frames
	localFrames = LocalFrame::alongTangent(tangent);

	// add one for last cross section
	tangent.push_back( tangent.back() );
	localFrames.push_back( localFrames.back() );

	// Bound planes
	startPlane = Plane(tangent[1], spinePoints[1]);
	endPlane = Plane(-*(tangent.rbegin() + 1), *(spinePoints.rbegin() + 1));

	// Reserve space
	int expectedNumVer = lengthCount * numSides * 4;
	vertex.reserve( expectedNumVer );
	vertexInfo.reserve(expectedNumVer );

	// vertex / face current indices
	int vIndex = 0;
	int fIndex = 0;

	// Create cylindrical structure as cross sections
	sections.push_back(CrossSection(radius, widthCount, spinePoints[0], tangent[0])); // first

	for(int v = 1; v < lengthCount; v++)
		sections.push_back(sections[v - 1].shiftedCopy(spinePoints[v], tangent[v])); // remaining

	sections.push_back(sections.back().shiftedCopy(spinePoints.back() + tangent.back() * segmentLength, tangent.back()));	// last

	// Last part
	spinePoints.push_back(sections.back().getCenter());
	tangent.push_back(tangent.back());

	// Add vertices to mesh structure
	for(int v = 0; v <= lengthCount; v++)
	{
		for(int u = 0; u < widthCount; u++)
			addVertex( sections[v].pointAt(u,0), vIndex++ );
	}

	// square vertex indices
	int v0, v1, v2, v3;

	int sIndex = 0;

	// Create grid faces
	for(int v = 0; v < lengthCount; v++)
	{
		for(int u = 0; u < widthCount; u++)
		{
			int offset = (widthCount * (v));

			v0 = u + offset;
			v1 = ((u + 1) % widthCount) + offset;
			v2 = v1 + widthCount;
			v3 = v0 + widthCount;

			// new faces indices
			int f1 = fIndex;
			int f2 = fIndex + 1;

			// Add the two faces
			addFace(v0, v1, v2, f1);
			addFace(v0, v2, v3, f2);

			// Add to grid structure
			squares->push_back(GridSquare(v0, v1, v2, v3, f1, f2, u, v, sIndex));

			faceToSquare.push_back(sIndex);
			faceToSquare.push_back(sIndex);

			// Add to squares map
			indexSquareMap[sIndex] = squares->back();
			squaresMap[Point(u, v)] = squares->back();
			squaresIndexMap[Point(u, v)] = sIndex++;

			fIndex += 2;
		}
	}

	// Compute rest of Mesh structure
	//computeNormals();
	//computeBounds();
	//clearColors();
	//createVBO();

	this->isVisible = true;

	// grid visibility specific
	this->isDrawReconstructedPoints = true;
	this->isDrawGridLines = true;
	this->isDrawSpine = true;
	this->isDrawWireframe = false;
	this->isDrawSmooth = false;

	// Rotation experiment
	this->rotateLeft = rotateLeft;
	this->rotateRight = rotateRight;

	// Debug
	activeGridSquare = NULL;
	activeGridPoint = NULL;

	printf(".grid constructed (w = %d, l = %d).", widthCount, lengthCount);
}

void Grid::FitCrossSections(Mesh * m, bool useEntireMesh)
{
	polygon.clear();
	shapes.clear();

	Vector<Plane> csPlane;
	Vector<Vec> pointOnPlane;

	// Rotation experiment
	Rotation q1(localFrames.front().n, (M_PI / 180.0) * rotateLeft);
	Rotation q2(localFrames.back().n, (M_PI / 180.0) * rotateRight);

	for(int v = 0; v < lengthCount + 1; v++)
	{
		Vec normalPlane = tangent[v];
		pointOnPlane.push_back(spinePoints[v]);

		csPlane.push_back(Plane(normalPlane, pointOnPlane.back()));

		// Rotation experiment
		float t = ((float)v / (lengthCount));

		localFrames[v].up = Rotation::slerp(q1,q2,t).rotate(localFrames[v].up);

		polygon.push_back(ClosedPolygon(csPlane[v], localFrames[v].up, localFrames[v].b));
	}

	// Experiment : multi step fitting
	StdList<Face*> faces = *meshFaces;
	if(useEntireMesh)
		faces = m->facesListPointers();

	for(int v = 0; v < lengthCount + 1; v++)
	{
		for(StdList<Face*>::iterator f = faces.begin(); f != faces.end(); f++)
		{
			Vec p1, p2;	

			Face * face = (*f);

			int isContour = csPlane[v].ContourFacet(face, p1, p2);

			if(isContour > 0)
			{
				//this->testLines.push_back(Line(p1, p2));
				polygon[v].insertLine(p1,p2);
			}
		}

		polygon[v].close();

		shapes.push_back(polygon[v].getEqualDistancePoints(numSides, spinePoints[v]));
	}

	SmoothCrossSecitons(2);

	for(int v = 0; v < (int)polygon.size(); v++)
	{
		if((int)shapes[v].size() == numSides)
		{
			for(int u = 0; u < numSides; u++)
			{
				int index = u + (v * (numSides));
				vertex[index] = shapes[v][u];
			}
		}
	}

	// Recompute cross-sections after fitting
	for(int v = 0; v < (int)this->lengthCount + 1; v++)
		sections[v] = CrossSection(v, this);
}

void Grid::SmoothCrossSecitons(int numIterations)
{
	int yCount = shapes.size() - 1;
	int xCount = shapes[0].size();

	Vector<Vector<Vec> > new_shapes = shapes;

	for(int it = 0; it < numIterations; it++)
	{
		// cross-section smoothing
		for(int v = 1; v < yCount; v++)
		{
			for(int u = 0; u < xCount; u++)
			{
				new_shapes[v][u] = (shapes[v-1][u] + shapes[v+1][u]) / 2.0;
			}
		}

		// Borders
		for(int u = 0; u < xCount; u++)
		{
			int prev = (u + xCount-1) % xCount;
			int next = (u + 1) % xCount;

			new_shapes[0][u] = (shapes[0][prev] + shapes[0][next]) / 2.0;
			new_shapes[yCount][u] = (shapes[yCount][prev] + shapes[yCount][next]) / 2.0;
		}

		shapes = new_shapes;
	}
}

void Grid::FitCylinder()
{
	Face * face = NULL;

	HitResult res;
	Ray ray;

	for(int i = 0; i < this->numberOfVertices(); i++)
	{
		// Shoot ray from Grid vertex along its normal
		ray.origin = vertex[i];
		ray.direction = vNormal[i];

		// Check contact with mesh that we need to fit
		for(StdList<Face*>::iterator f = meshFaces->begin(); f != meshFaces->end(); f++)
		{
			face = *f;
			face->intersectionTest(ray, res, true);

			if(res.hit && face->normal() * ray.direction > 0)
			{
				vertex[i] += ray.direction * res.distance;
				break;
			}
		}
	}
}

void Grid::FitNothing()
{

}

void Grid::Gridify(Vector<int> & selectedMeshFaces)
{
	// Preprocessing...
	computeNormals();
        computeBounds();

	for(int v = 0; v < this->lengthCount + 1; v++)	
		sections[v] = CrossSection(v, this);
	// end preprocess.

	CreateTimer(timer);
	printf("Building Octrees..");

	Mesh * base = stair->mostBaseMesh();
	Mesh * detailed = stair->mostDetailedMesh();

	// Octree initialization
        grid_octree = Octree(this->facesListPointers(), 20);
	detailed_octree = Octree(selectedMeshFaces, detailed, 20);

	grid_octree.build();
	detailed_octree.build();

	printf("Done (%d ms).", (int)timer.elapsed());

	this->selectedFaces = selectedMeshFaces;

	// Convert faces indices to vertex indices
	Vector<int> activePoints = SET_TO_VECTOR( base->getVerticesFromFaces(selectedFaces) );

	// Record heights for each level
	this->max_height = Vector<double>(stair->numberOfSteps(), DBL_MIN);
	this->min_height = Vector<double>(stair->numberOfSteps(), DBL_MAX);

	Vector<GridSquare> * squares = &multiLevelSquares[0];
	Vector<int> modifiedSquares;

	int N = activePoints.size();

	// Timing
	CreateTimer(projectionTimer);
	printf(".(Number of Points = %d).", N);

	stats["gridifiy"] = Stats("Projecting points (Gridify)");

	// For each vertex of the base mesh
	for(int p = 0; p < N; p++)
	{
		int i = activePoints[p]; // point index

		Vec detailedPoint = *detailed->v(i);
		Vec basePoint = *base->v(i);

		Ray testRay(basePoint, *base->n(i), i); // ray towards triangles

		HitResult res;
		GridSquare * square = NULL;

		double minDist = DBL_MAX;
		Vec pointOnSquare;
		Vector<double> w(4, 0);

		// Filter points outside our work area
		if(startPlane.IsFront(detailedPoint) && endPlane.IsFront(detailedPoint))
		{
			IndexSet indexSet;

			Face * closeFace = grid_octree.findClosestTri(testRay, indexSet, this, res);

			if(closeFace)
			{
				square = &squares->at(faceToSquare[closeFace->index]);

				pointOnSquare = testRay.origin + (res.distance * testRay.direction);
			}

			// Brute force for outliers
			if(!square || (pointOnSquare - basePoint).norm() > segmentLength * 2)
			{
				double closestFaceDist = DBL_MAX;
				Face * closestFace = NULL;

				// Try all squares
				for(StdList<Face>::iterator f = face.begin(); f != face.end(); f++)
				{
					f->intersectionTest(testRay, res, true);

					if(res.hit && abs(res.distance) < minDist)
					{
						minDist = abs(res.distance);

						square = &squares->at(faceToSquare[f->index]);
						pointOnSquare = testRay.origin + (res.distance * testRay.direction);
					}

					// For last resort
					if((f->center() - basePoint).norm() < closestFaceDist)
					{
						closestFaceDist = (f->center() - basePoint).norm();
						closestFace = &(*f);
					}
				}

				// Last resort, force to closest square 
				if(!square && closestFace)
				{
					pointOnSquare = basePoint = closestFace->center();
					square = &squares->at(faceToSquare[closestFace->index]);
				}
			}
		}

		// The grid insertion operations
		if(square)
		{
			// Insert point into grid square
			GridPoint gp = ProjectOnGrid(detailedPoint, pointOnSquare, square, i, w);
			square->insertPoint(gp);

			// Add to map for easy access
			pointSquareMap[i] = (*square);
			pointSquareIndexMap[i] = Point(square->u, square->v);
			pointVecMap[i] = ParameterCoord(w, square->u, square->v);
			basePoints[i] = basePoint;

			// Save normals of original mesh
			originalMeshNormals[i] = *stair->mostDetailedMesh()->n(i);

			modifiedSquares.push_back(square->id);

			// World Records
			float h = square->points.back().h;

			max_height[0] = Max(h, max_height[0]);
			min_height[0] = Min(h, min_height[0]);
		}
	}

	stats["gridifiy"].end();

	stats["numPoints"] = Stats("Num Points in region", (double)pointSquareMap.size());

	// Copy modified squares (so bad.. pointers are better)
	for(int i = 0; i < (int)modifiedSquares.size(); i++)
	{
		GridSquare s = squares->at(modifiedSquares[i]);
		indexSquareMap[modifiedSquares[i]] = s;
		squaresMap[Point(s.u, s.v)] = s;
	}

	printf("Point projections done (%d ms).", (int)projectionTimer.elapsed());

	CreateTimer(squaresTimer);
	printf("Computing square values..");

	this->computeSquareValues();

	printf("done (%d ms).", (int)squaresTimer.elapsed());

	this->isReady = true;
}

void Grid::computeSquareValues()
{
	int numberOfLevels = stair->numberOfSteps();

	// Prepare multi-level (empty) squares
	for(int l = 0; l < numberOfLevels - 1; l++)
		multiLevelSquares.push_back(Vector<GridSquare>(multiLevelSquares[0]));

	Vector<GridSquare> * squares = &multiLevelSquares[0];

	Mesh * mesh = stair->mostDetailedMesh();
	//Mesh * baseMesh = stair->mostBaseMesh();

	stats["computeSquares"] = Stats("Compute approximate image");

	// Get multilevel height for each square on the grid
	for(int s = 0; s < (int)squares->size(); s++)
	{
		GridSquare * square = &squares->at(s);
		Vec squareNormal = ((fNormal[square->face1] + fNormal[square->face1]) / 2.0).unit();

		//Vector<GridPoint> * points = &square->points;

		// Average of points (if any) <- this might be a bad choice...
		/*if(points->size() > 0)
		{
		// Multi-level
		for(int l = 0; l < numberOfLevels; l++)
		{
		// for each level compute the height
		double sum = 0;

		for(int pIndex = 0; pIndex < points->size(); pIndex++)
		{
		GridPoint * p = &points->at(pIndex);

		Vec detailedPoint = *stair->getStepDetailed(l)->v(p->corrPoint);

		double height = ((squareNormal * SIGN(p->h)) * (detailedPoint - basePoints[p->corrPoint])) * SIGN(p->h);

		if(l > 0)
		multiLevelSquares[l][s].getPoint(pIndex)->h = height;

		sum += height;
		}

		multiLevelSquares[l][s].height = sum / points->size();
		}
		}
		else*/
		{
			// Else, Shoot ray to base mesh, and find it on detailed surface to get height
			Vec pointOnSquare = PointFromSquare(GridPoint::MidPoint().w, square);

			Ray ray(pointOnSquare, squareNormal);
			HitResult hitRes, closestHit;

			IndexSet tris;

			Face * closestFace = detailed_octree.findClosestTri(ray, tris, mesh, hitRes);

			if(closestFace)
			{
				// Multi-level
				for(int l = 0; l < numberOfLevels; l++)
				{
					stair->getStepDetailed(l)->f(closestFace->index)->intersectionTest(ray, hitRes, true);

					if(hitRes.hit)
						closestHit = hitRes;
					else
					{
						double minDist = DBL_MAX;

						// closest face has changed
						foreach(int findex, selectedFaces)
						{
							stair->getStepDetailed(l)->f(findex)->intersectionTest(ray, hitRes, true);

							if(hitRes.hit && abs(hitRes.distance) < abs(minDist))
							{
								closestFace = mesh->f(findex);
								minDist = abs(hitRes.distance);
								closestHit = hitRes;
							}
						}
					}

					double height = closestHit.distance;

					if(height > max_height[l] * 5)	height = max_height[l];
					if(height < min_height[l] * 5)	height = min_height[l];

					multiLevelSquares[l][s].height = height;

					max_height[l] = Max(max_height[l], height);
					min_height[l] = Min(min_height[l], height);
				}
			}
			else
			{
				multiLevelSquares[0][s].height = min_height[0];
			}
		}
	}

	stats["computeSquares"].end();

	// Subtract large scale pattern
	/*for(int s = 0; s < squares->size(); s++)
	{
	GridSquare * square = &squares->at(s);
	Vector<GridPoint> * points = &square->points;

	double level1_difference = -((multiLevelSquares[1][s].height - min_height[1]));

	for(int i = 0; i < points->size(); i++)
	{
	GridPoint * p = &points->at(i);
	GridPoint * p_smoothed = multiLevelSquares[1][s].getPoint(i);

	//p->h = p->h + level1_difference;
	}

	//multiLevelSquares[0][s].height += level1_difference;
	}*/
}

Vector2Df Grid::largeScalePattern()
{
	Vector2Df result = Vector2Df(widthCount, Vector<float>(lengthCount, 0.0));

	if(multiLevelSquares.size() > 1)
	{
		Vector<GridSquare> * level = &multiLevelSquares[1];

		for(int v = 0; v < lengthCount; v++)
		{
			for(int u = 0; u < widthCount; u++)
				result[u][v] = level->at(squaresIndexMap[Point(u,v)]).height - min_height[1];
		}
	}

	return result;
}

Vector2Df Grid::sectionsPattern()
{
	Vector2Df result = Vector2Df(widthCount, Vector<float>(lengthCount + 1));

	double max_value = DBL_MIN;

	// Find maximum height across all cross-sections
	for(int v = 0; v < lengthCount + 1; v++)
	{
		for(int u = 0; u < widthCount; u++)
		{
			result[u][v] = (sections[v].pointAt(u) - sections[v].getCenter()).norm();

			max_value = Max(MaxElement(result[u]), max_value);
		}
	}

	return result;
}

GridSquare * Grid::getSquare(int u, int v)
{
	Point p(u,v);

	if(squaresMap.find(p) != squaresMap.end())
		return &squaresMap[p];
	else
		return NULL;
}

Vec Grid::gridPoint(GridSquare * s, int index)
{
	return vertex[s->p[index]].vec();
}

Vec Grid::gridPointNormal(int point_index, const Vec& U, const Vec& V, const Vec& N)
{
	GridSquare * square = getSquare(pointSquareIndexMap[point_index].x, pointSquareIndexMap[point_index].y);

	Vec myU = (vertex[square->p[1]] - vertex[square->p[0]]).unit();
	Vec myV = (vertex[square->p[3]] - vertex[square->p[0]]).unit();
	Vec myN = ((fNormal[square->face1] + fNormal[square->face2]) / 2.0).unit();

	Vec pointNormal = originalMeshNormals[point_index];

	double tu = pointNormal * myU;
	double tv = pointNormal * myV;
	double tn = pointNormal * myN;

	return (tu * U) + (tv * V) + (tn * N);
}

Vector<GridSquare> * Grid::getSquares(int level)
{
	return &multiLevelSquares[level];
}

SquareDimension Grid::getSquareDimensions(int u, int v)
{
	GridSquare * s = getSquare(u, v);

	double U = (*this->v(s->p[0]) - *this->v(s->p[1])).norm();
	double V = (*this->v(s->p[0]) - *this->v(s->p[3])).norm();

	return SquareDimension(U, V);
}

double Grid::getHeightRange(int level)
{
	if(max_height.size())
		return max_height[level] - min_height[level];
	else
		return 0;
}

Mesh * Grid::getBaseMesh()
{
	return stair->mostBaseMesh();
}

Mesh * Grid::getDetailedMesh()
{
	return stair->mostDetailedMesh();
}

Vector<int> * Grid::getSelectedFaces()
{
	return &selectedFaces;
}

StdSet<int> Grid::getGridifiedPoints()
{
	StdSet<int> pointsIds;

	for(HashMap<int, Vec>::iterator it = basePoints.begin(); it != basePoints.end(); it++)
	{
		pointsIds.insert(it->first);
	}

	return pointsIds;
}

Plane Grid::getMidPlane()
{
	int mid = sections.size() / 2;

	return Plane(sections[mid].getNormal(), sections[mid].getCenter());
}

void Grid::drawAsGrid()
{
	// Debug

	// Draw cross-sections
	//for(int i = 0; i < sections.size(); i++)
	//	sections[i].draw(3.0);
	/*for(int i = 0; i < (int)polygon.size(); i++)
	{
		localFrames[i].draw(spinePoints[i]);
		polygon[i].draw();
	}*/
	//return;

	// Octrees
	/*if(isReady)
	{
		grid_octree.draw(1,0,0);
		//base_octree.draw(1,1,1);
		detailed_octree.draw(0,0,1);
	}*/

	SimpleDraw::IdentifyConnectedPoints(testPoints);

	if(isDrawSpine)
	{
		SimpleDraw::IdentifyConnectedPoints(spinePoints, 1, 0.2f, 1);
	}

	if(isDrawGridLines)
	{
		glDisable(GL_LIGHTING);

		glLineWidth(2.0);

		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBegin(GL_LINES);
		for(int i = 0; i < this->lengthCount; i++)
		{
			for(int s = 0; s < this->widthCount - 1; s++)
			{
				glColor4f(0,0,1.0, 0.5);  // blue
				glVertex3fv( ver(  s + (numSides * (i))        ));
				glVertex3fv( ver(  s + (numSides * (i+1))      ));
				glVertex3fv( ver(  s + (numSides * (i)) + 1    ));
				glVertex3fv( ver(  s + (numSides * (i+1)) + 1  ));

				glColor4f(1.0,0,0, 0.5);  // red
				glVertex3fv( ver(  s + (numSides * (i))        ));
				glVertex3fv( ver(  s + (numSides * (i)) + 1    ));	
			}

			glVertex3fv( ver(  (numSides * (i))					));
			glVertex3fv( ver(  (numSides * (i)) + numSides-1	));
		}

		// Last line
		int i = this->lengthCount;
		for(int s = 0; s < this->widthCount - 1; s++)
		{
			glVertex3fv( ver(  s + (numSides * (i))        ));
			glVertex3fv( ver(  s + (numSides * (i)) + 1    ));	
		}
		glVertex3fv( ver(  (numSides * (i))					));
		glVertex3fv( ver(  (numSides * (i)) + numSides-1	));
		glEnd();


		glDisable(GL_BLEND);
	}

	if(isDrawReconstructedPoints && isReady)
	{
		Vector<GridSquare> * squares = &multiLevelSquares[0];

		double heightScale = max_height[0] - min_height[0];

		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND); 

		glEnable (GL_POINT_SMOOTH);

		glPointSize(6.0);

		// draw reconstructed points
		glBegin(GL_POINTS);
		for(int s = 0; s < (int)squares->size(); s++)
		{
			GridSquare * square = &squares->at(s);

			for(Vector<GridPoint>::iterator p = square->points.begin(); p != square->points.end(); p++)
			{
				if(activeGridPoint != &(*p))
				{
					Vec point = Reconstruct(p->w, square, p->h, p->shiftAngle, p->shiftMagnitude);

					double value = (p->h - this->min_height[0]) / heightScale;

					value = Max(0.0, Min(1.0, value));

					glColor3f(value, value , value);
					glVertex3fv(point);
				}
			}
		}
		glEnd();
	}

	// Debug
	if(activeGridSquare)
	{
		GridSquare * square = activeGridSquare;

		highlightSquare(square->u, square->v);

		if(activeGridPoint)
		{
			GridPoint * p = activeGridPoint;

			Vec point = Reconstruct(p->w, square, p->h, p->shiftAngle, p->shiftMagnitude);

			SimpleDraw::IdentifyPoint(point);

			// draw original too
			SimpleDraw::IdentifyPoint2(*stair->mostDetailedMesh()->v(p->corrPoint));
		}
	}

	glEnable(GL_LIGHTING);
}

void Grid::drawPointNames()
{
	Vector<GridSquare> * squares = &multiLevelSquares[0];

	for(Vector<GridSquare>::iterator square = squares->begin(); square != squares->end(); square++)
	{
		for(Vector<GridPoint>::iterator p = square->points.begin(); p != square->points.end(); p++)
		{
			Vec point = Reconstruct(p->w, &(*square), p->h, p->shiftAngle, p->shiftMagnitude);

			glPushName(p->corrPoint);
			glBegin(GL_POINTS);
			glVertex3fv(point);
			glEnd();
			glPopName();
		}
	}
}

void Grid::selectPoint(int index)
{
	if(index < 0)
	{
		activeGridSquare = NULL;
		activeGridPoint = NULL;
		return;
	}

	activeGridSquare = getSquare(pointSquareIndexMap[index].x, pointSquareIndexMap[index].y);

	for(Vector<GridPoint>::iterator p = activeGridSquare->points.begin(); p != activeGridSquare->points.end(); p++)
	{
		if(p->corrPoint == index)
		{
			activeGridPoint = &(*p);

			printf("\nSelected point (%d) :\n", p->corrPoint);
			printf("Height: %f \n", p->h);
			printf("Weights: ( %f ) ( %f ) ( %f ) ( %f ) \n", p->w[0], p->w[1], p->w[2], p->w[3]);
			printf("Angle (%f) - Magnitude (%f)\n", p->shiftAngle, p->shiftMagnitude);
			break;
		}
	}
}

void Grid::highlightSquare(int u, int v)
{
	// Avoid indexing problems
	u = u % this->widthCount;
	v = v % this->lengthCount;

	GridSquare * square = getSquare(u, v);

	// Draw highlighted square
	glDisable(GL_LIGHTING);

	glColor3f(0,0.3f,0.9f);

	glBegin(GL_QUADS);
	glVertex3fv( vertex[square->p[0]].vec() );
	glVertex3fv( vertex[square->p[1]].vec() );
	glVertex3fv( vertex[square->p[2]].vec() );
	glVertex3fv( vertex[square->p[3]].vec() );
	glEnd();

	glEnable(GL_LIGHTING);
}

GridSquare * Grid::getSquareOf( int vindex )
{
	return &this->pointSquareMap[vindex];
}

// Mean value coordinates - based on "Mean Value Coordinates" by Michael S. Floater
void Grid::MVC(const Vec& p, const Vec q[], Vector<double> & w)
{
	int n = w.size();
	int j, prev, next;

	// Border test
	for(j = 0; j < n; j++)
	{
		next = (j + 1) % n;

		if(Vertex::isBetween(p, q[j], q[next], Epsilon))
		{
			double len = (q[j] - q[next]).norm();

			// clear weights and interpolate between two points
			w = Vector<double>(n, 0.0);		

			double fromj = (p - q[j]).norm();

			w[next] = fromj / len;
			w[j] = 1 - w[next];

			return;
		}
	}

	double weightSum = 0.0;

	// Inside polygon
	for(j = 0; j < n; j++)
	{
		prev = (j + n-1) % n;
		next = (j + 1) % n;

		double len = (p - q[j]).norm();

		double tanAlpha = Vertex::halfAlphaTangent(q[prev], p, q[j]);
		double tanBeta = Vertex::halfAlphaTangent(q[j], p, q[next]);

		w[j] = (tanAlpha + tanBeta) / len;

		weightSum += w[j];
	}

	// Normalize weights
	for(j = 0; j < n; j++)
		w[j] /= weightSum;
}

Vec Grid::ParameterCoord( const Vector<double> w, int u, int v )
{
	Vec p;

	Vec corner[4];

	corner[1].x = 1;
	corner[2].x = 1; corner[2].y = 1;
	corner[3].y = 1;

	// point Relative to square
	for(int i = 0; i < 4; i++)
	{
		p.x += corner[i].y * w[i];
		p.y += corner[i].x * w[i];
	}

	// relative to grid
	double gridX = (double) v / lengthCount;
	double gridY = (double) u / widthCount;

	double squareSizeX = 1.0 / lengthCount;
	double squareSizeY = 1.0 / widthCount;

	// final position
	p.x = gridX + (p.x * squareSizeX);
	p.y = gridY + (p.y * squareSizeY);

	return p;
}

GridPoint Grid::ProjectOnGrid(const Vec& detailedPoint, const Vec& pointOnSquare, 
							  GridSquare * square, int corrPoint, Vector<double> & w)
{
	Vec q[] = {vertex[square->p[0]], vertex[square->p[1]], vertex[square->p[2]], vertex[square->p[3]]};

	MVC(pointOnSquare, q, w);

	Vec squareNormal = ((fNormal[square->face1] + fNormal[square->face2]) / 2.0).unit();

	// Find height from grid in normal direction
	double height = (squareNormal * (detailedPoint - pointOnSquare));

	// Find shift factor
	Vec heightPoint = pointOnSquare + (squareNormal * height); // p'
	Vec shiftVector = detailedPoint - heightPoint;

	shiftVector.projectOnPlane(squareNormal);

	double shiftMagnitude = shiftVector.norm();

	// Forward direction (used for local frame)
	double shiftAngle = Vertex::singed_angle(q[3] - q[0], shiftVector, squareNormal);

	return GridPoint (&w[0], height, shiftAngle, shiftMagnitude, corrPoint);
}

Vec Grid::PointFromSquare(const double w[], GridSquare * square)
{
	Vec p;

	for(int i = 0; i < 4; i++)
	{
		p.x += vertex[square->p[i]].x * w[i];
		p.y += vertex[square->p[i]].y * w[i];
		p.z += vertex[square->p[i]].z * w[i];
	}

	return p;
}

Vec Grid::Reconstruct(const double w[], GridSquare * square, double height, double angle, double shift)
{
	Vec squareNormal = ((fNormal[square->face1] + fNormal[square->face2]) / 2.0).unit();
	Vec pointOnSquare = PointFromSquare(w, square);

	Vec point = pointOnSquare + (height * squareNormal);

	double shiftAngle = angle;
	double shiftMagnitude = shift;

	Vec ref = (v(square->p[3])->vec() - v(square->p[0])->vec()).unit();
	Vec rotatedRef = Rotation(squareNormal, shiftAngle).rotate(ref);

	point += rotatedRef * shiftMagnitude;

	return point;
}
