#pragma once

#include "Globals.h"
#include "WeightMatrix.h"
#include "CutMask.h"

namespace Synth
{
	class Block
	{
	private:
		
		MatrixXf * src;

	public:
		BlockType type;

		Block();
		Block(Point point, MatrixXf * source, BlockType newType);

		void paste(int u, int v, MatrixXf * target, Vector<Vector<Point> > * targetAsPos);
		
		void setUV(int & x, int & y);
		MatrixXf getCutMask(int u, int v, MatrixXf * target);

		Point p; // faster??
		inline int y() {return p.y;}
		inline int x() {return p.x;}
	};
}
