#pragma once

#include "Globals.h"
#include "Utility/Graph.h"

class CutMask
{
	MatrixXf costMatrix;
	float cost(Point p1, Point p2);

	Graph<int, float> g;
	StdMap<Point, int> nodes;
	StdMap<int, Point> points;

	MatrixXf mask;

	bool point(Point p);
	void protectCore(MatrixXf & overlap, BlockType type);

	void fillMask(MatrixXf & m, BlockType type);
	void fillMask(int x, int y, MatrixXf & m);

public:
	CutMask(MatrixXf & overlap, BlockType type);
	
	MatrixXf getMask();
};
