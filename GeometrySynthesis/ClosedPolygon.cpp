#include "ExtendMeshHeaders.h"
#include "ClosedPolygon.h"

ClosedPolygon::ClosedPolygon(const Plane& Plane, const Vec& VecUp, const Vec& VecB)
{
	closedLength = 0;
	minEdgeLength = DBL_MAX;

	this->plane = Plane;
	this->vecUp = VecUp.unit();
	this->vecB = VecB.unit();

	this->points.create(3);

	this->lastVertexIndex = 0;
	this->lastEdgeIndex = 0;
}

void ClosedPolygon::insertPoint(const Vec & point)
{
	closedPoints.push_back(point);
}

int ClosedPolygon::insertIndexedPoint(const Vec &p)
{
	int vIndex = -1;

	kdres * findP = points.nearest3f(p.x, p.y, p.z);

	if(findP)
	{
		double * pos = findP->riter->item->pos;

		Vec closestPoint(pos[0], pos[1], pos[2]);

		double dist = (closestPoint - p).norm();

		if(dist < Epsilon)
		{
			vIndex = findP->riter->item->index;
			kd_res_free(findP);
			return vIndex;
		}
	}

	vIndex = lastVertexIndex;
	allPoints[vIndex] = p;

	points.insert3f(p.x, p.y, p.z, lastVertexIndex++);

	return vIndex;
}

void ClosedPolygon::insertLine(const Vec &p1, const Vec &p2)
{
	int vIndex1 = insertIndexedPoint(p1);
	int vIndex2 = insertIndexedPoint(p2);

	g.AddEdge(vIndex1, vIndex2, 1, lastEdgeIndex++);

	Line line (p1, p2);
	allLines.push_back(line);
}

bool ClosedPolygon::isClosed()
{
	int i = g.getNodeLargestConnected();
	if(i == -1) return false;
	return g.isCircular(i);
}

bool ClosedPolygon::isClosed(int& i)
{
	i = g.getNodeLargestConnected();

	if(i == -1) return false;

	return g.isCircular(i);
}

void ClosedPolygon::close()
{
	if(!allPoints.size())
		return;

	int i = 0;

	// for non circular cross-sections, force closed circle (won't work with multi-non connected)
	if(!isClosed(i))
	{
		Vector<int> leaves = g.GetLeaves();

		// connect ends
		if(leaves.size() > 1)
			g.AddEdge(leaves.front(), leaves.back(), 1, lastEdgeIndex++);
	}

	if(i >= 0)
	{
		int j = g.GetRandomNeighbour(i);

		g.SetEdgeWeight(i, j, DBL_MAX);

		StdList<int> points = g.DijkstraShortestPath(i,j);

		for(StdList<int>::iterator it = points.begin(); it != points.end(); it++)
			closedPoints.push_back(allPoints[*it]);
	}
}

void ClosedPolygon::reverseClosedOrder()
{
	std::reverse(closedPoints.begin(), closedPoints.end());
	std::reverse(closedEdgeLen.begin(), closedEdgeLen.end());
}

void ClosedPolygon::simplify()
{
	// smooth ?
	/*Vector<Vec> tempPoints = closedPoints;
	for(int it = 0; it < 2; it++)
	{
	Vector<Vec> subDivided;

	// Subdivide at midpoint
	subDivided.push_back(tempPoints[0]);
	for(int i = 1; i < tempPoints.size(); i++){
	subDivided.push_back(Vertex::MidVec(tempPoints[i-1], tempPoints[i]));
	subDivided.push_back(tempPoints[i]);
	}

	// Smooth result
	for(int smoothIt = 0; smoothIt < 2; smoothIt++){
	Vector<Vec> smooth = subDivided;
	int N = smooth.size();

	for(int i = 1; i < N + 1; i++){
	int p = (i-1) % N;
	int q = (i) % N;
	int r = (i+1) % N;

	smooth[q] = (smooth[p] + smooth[r]) / 2.0;
	}

	tempPoints = smooth;
	}
	}

	closedPoints = tempPoints;*/
}

void ClosedPolygon::computeLengths()
{
	int N = closedPoints.size();

	closedLength = 0;

	for(int i = 0; i < N; i++)
	{
		double dist = (closedPoints[(i+1) % N] - closedPoints[i]).norm();

		minEdgeLength = Min(minEdgeLength, dist);

		closedEdgeLen.push_back(dist);
		closedLength += dist;
	}
}

Vector<Vec> ClosedPolygon::getEqualDistancePoints(int numSides, const Vec& center)
{
        Vector<Vec> result;

	// for complex shapes
	HashMap<int, Line> lineDists;

	int N = closedPoints.size();

	if(N < 1)	return result; // empty polygon

        for(int i = 0; i < N; i++)
                lines.push_back(Line(closedPoints[i], closedPoints[(i+1) % N], i));

	this->computeLengths();

	// Distance to walk on polygon
	double segmentLength = this->closedLength / numSides;

	// Locate start point using vecUp
        Vec startPoint;
        int startIndex = 0;
        Line referenceLine(center, center + vecB); // long line

	// Hack, vector Up should already be projected on the plane?
        //this->plane.projectLine(referenceLine);

        // Debug:
        testLines1.push_back(referenceLine.colored(Color4(255,255,255)));

        //int lastIndex = 0;

        Plane halfPlane(vecUp, center);

        //testPlanes1.push_back(halfPlane);

        double minDist = DBL_MAX;

        // Test intersection with all lines and remember minimum one
	for(int i = 0; i < N; i++)
	{
                Vec pointIntersect;

                int res = halfPlane.LineIntersect(lines[i], pointIntersect);

                if(res == INTERSECT || res == ENDPOINT_INTERSECT)
                {
                    Vec toIntsect = pointIntersect - center;

                    if(toIntsect.norm() < minDist && toIntsect * vecB > 0)
                    {
                        minDist = toIntsect.norm();

                        lineDists[i] = Line(pointIntersect, pointIntersect);
                        startPoint = pointIntersect;
                        startIndex = i;
                    }
                }
	}

	double t = lines[startIndex].timeAt(startPoint);

	int index = startIndex;

        testPoints1.push_back(startPoint);

	// Compute equal-dist points on polygon
	for(int s = 0; s < numSides; s++)
	{
		// Add new point
		result.push_back(lines[index].pointAt(t));

		walk(segmentLength, t, index, &t, &index);
	}

	// if polygon is opposite direction then reverse 
        if( signedArea(result, plane.n, center) < 0 )
	{
		std::reverse(result.begin(), result.end());
                std::rotate(result.begin(), result.begin()+result.size()-1 , result.end());
	}



/*	// Fix first node position
        double minDist = DBL_MAX;
	Vector<Vec>::iterator makeFirst = result.begin();
	for(Vector<Vec>::iterator it = result.begin(); it != result.end(); it++){
		double dist = (*it - startPoint).norm();

		if(dist < minDist){
			minDist = dist;
			makeFirst = it;
		}
	}

	// And make that point the first point
	std::rotate(result.begin(), makeFirst, result.end());
*/
	return closedPoints = result;
}

void ClosedPolygon::walk(double distance, double startTime, int index, double * destTime, int * destIndex)
{
	double remain = lines[index].lengthsAt(startTime).second;

	// Case 1: the point is on the starting line
	if(remain > distance)
	{
		double startLength = startTime * lines[index].length;
		*destTime = (startLength + distance) / lines[index].length;
		*destIndex = index;
		return;
	}

	double walked = remain;

	// Case 2: keep walking next lines
	while(walked < distance)
	{
                index = (index + 1) % lines.size();		// step to next line
		walked += lines[index].length;
	}

	// Step back to the start of this line
	walked -= lines[index].length;

	double remainDistance = distance - walked;
	double endTime = remainDistance / lines[index].length;

	*destTime = endTime;
	*destIndex = index;
}

double ClosedPolygon::closedPathLen()
{
	double len = 0;

	for(int i = 0; i < (int)lines.size(); i++)
		len += lines[i].length;

	return len;
}

void ClosedPolygon::draw()
{
	// Draw the closed polygon
	Vector<Vec> loop = this->closedPoints;

	//if(loop.size())
	{
		//loop.push_back(loop.front());
		//SimpleDraw::IdentifyConnectedPoints(loop);
	}

	// first point
	SimpleDraw::IdentifyPoint(closedPoints.front(), 1,0,0.5, 20);

	SimpleDraw::IdentifyPoints(unvisitedPoints);
	SimpleDraw::IdentifyPoints(testPoints);
	SimpleDraw::IdentifyLines(tempLines, 10.0);

	/*for(StdMap<int, Vec>::iterator it = allPoints.begin(); it!= allPoints.end() ; it++)
	{
	SimpleDraw::IdentifyPoint(it->second);
	}*/

	//foreach(Face * face, testFaces)
	//	SimpleDraw::DrawTriangle(face->vec(0), face->vec(1), face->vec(2));

	//PairVec edge = GetClosedEdge(maxEdge.index);
	//SimpleDraw::IdentifyLine(edge.first, edge.second);
}

ExtendedPolygon::ExtendedPolygon(const ClosedPolygon& Poly, const QColor& Color, double Weight, const Vec& Center)
{
	this->color = Color;
	this->weight = Weight;
	this->center = Center;

	this->vecB = Poly.vecB;
	this->vecUp = Poly.vecUp;
	this->plane = Poly.plane;

	this->closedLength = 0;
	this->lastVertexIndex = 0;
	this->lastEdgeIndex = 0;

	points.create(3);

	for(int l = 0; l < (int)Poly.allLines.size(); l++)
		this->insertLine(Poly.allLines[l].a, Poly.allLines[l].b);

	this->close();
}

ExtendedPolygon::ExtendedPolygon( ExtendedPolygon& polyA, ExtendedPolygon& polyB, int numSides, const QColor& Color )
{
	this->color = Color;
	this->vecB = polyA.vecB;
	this->vecUp = polyA.vecUp;
	this->center = polyA.center;
	this->plane = polyA.plane;
	this->weight = polyA.weight;

	this->closedLength = 0;
	this->lastVertexIndex = 0;
	this->lastEdgeIndex = 0;

	points.create(3);

	// Create from a blend of two closed polygons, depending on their weight
	Vector<Vec> equiCountA = polyA.getEqualDistancePoints(numSides, center);
	Vector<Vec> equiCountB = polyB.getEqualDistancePoints(numSides, center);

	//polyA.tempLines.push_back(Line(equiCountA[0], equiCountA[1]));
	//polyB.tempLines.push_back(Line(equiCountB[0], equiCountB[1]));

	for(int i = 0; i < numSides - 1; i++)
	{
		Vec p1 = (equiCountA[i] * polyA.weight) + (equiCountB[i] * polyB.weight);
		Vec p2 = (equiCountA[i+1] * polyA.weight) + (equiCountB[i+1] * polyB.weight);

		this->insertLine(p1, p2);
	}

	this->close();
}

void ExtendedPolygon::drawColored()
{
	SimpleDraw::IdentifyPoints(closedPoints, color.redF(), color.greenF(), color.blueF(), 6);

	glColor3f(color.redF(), color.greenF(), color.blueF());

	SimpleDraw::DrawArrowDirected(center, vecUp, 0.25f);
	SimpleDraw::DrawArrowDirected(center, vecB, 0.1f);

	SimpleDraw::IdentifyLines(tempLines, 1.0);
}
