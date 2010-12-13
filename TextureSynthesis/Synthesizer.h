#pragma once

#include <QThread>
#include <QElapsedTimer>

#include "Globals.h"
#include "Block.h"
#include "WeightMatrix.h"

namespace Synth
{
	typedef StdMap<Point, Block> ReducedSet;

	class Synthesizer : public QThread
	{
		Q_OBJECT

		private:
			MatrixXf src;
			MatrixXf target;
			Vector<Vector<Point> > targetAsPos;

			MatrixXf debug;

			StdMap<float, Vector<Point> > positions;

			int cut;
			int jump;
			int patchSize;
			int bottomPadding;

			int cur_x;
			int cur_y;

		protected:
			void run();

		public:
			Synthesizer(int BottomPadding = 0);

			void init(const MatrixXf & source, int synthWidth, int blockSize, int bandSize);
			void synthesizeNext();

			void synthesizeAll();

			// Block operations
			BlockType getBlockType();
			ReducedSet getMatchingBlocks(BlockType);
			Block getBestBlock(ReducedSet & set);
			void addBlocks(Point at, int deltaX, int deltaY, BlockType type, ReducedSet & set, CheckList & list);

			// Pixel helper functions
			Point pixel(int x, int y) { return Point(x,y) ;}
			inline bool empty(Point & p)	{return target(p.y, p.x) == EMPTY_PIXEL;}
			bool isValidPatch(Point & p);

			bool isDone;

			void crop();

			MatrixXf result();
			Vector<Vector<Point> > resultAsPos();

		signals:
			void print(QString);
			void view(MatrixXf m);
	};
}
