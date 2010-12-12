#pragma once

#include <algorithm>

#include "SimpleDraw.h"
#include "Umbrella.h"

#include "Plane.h"
#include "Line.h"

#include "kdtree.h"
#include "Graph.h"

#include "PolygonArea.h"

class ClosedPolygon
{
public:
	ClosedPolygon(){ closedLength = 0; }

	ClosedPolygon(const Plane& Plane, const Vec& VecUp, const Vec& VecB);

	// debug
	Vector<Vec> testPoints;
	Vector<Face *> testFaces;

	void insertPoint(const Vec &);
	int insertIndexedPoint(const Vec &);
	void insertLine(const Vec &, const Vec &);

	void close();
	bool isClosed();
	bool isClosed(int & i);

	Vector<Vec> getEqualDistancePoints(int numSides, const Vec& center);
	void walk(double distance, double startTime, int index, double * destTime, int * destIndex);

	void computeLengths();
	double closedPathLen();

	void reverseClosedOrder();
	void simplify();

	void draw();

public: // should be protected..
	Vector<double> closedEdgeLen;
	double closedLength;
	double minEdgeLength;

	Plane plane;
	Vec vecUp, vecB;

	int lastVertexIndex;
	int lastEdgeIndex;
	KDTree points;

	Graph<int, double> g;

	HashMap<int, Vec> allPoints;
	Vector<Vec> closedPoints;
	Vector<Vec> unvisitedPoints;

	Vector<Line> lines;
	Vector<Line> tempLines;
	Vector<Line> allLines;
};

class ExtendedPolygon : public ClosedPolygon{
public:
	ExtendedPolygon(const ClosedPolygon& poly, const QColor& color, double weight, const Vec& Center);
	ExtendedPolygon(ExtendedPolygon& polyA, ExtendedPolygon& polyB, int numSides, const QColor& color);

	QColor color;
	double weight;
	Vec center;

	void drawColored();
};
