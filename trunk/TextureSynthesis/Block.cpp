#include "Block.h"

#include <QImage>
#include <QPainter>

namespace Synth
{
	Block::Block()
	{
		this->p = Point(-1, -1);
		this->src = NULL;
		this->type = NONE_BLOCK;
	}

	Block::Block(Point point, MatrixXf * source, BlockType newType)
	{
		this->p = point;
		this->src = source;
		this->type = newType;
	}

	void Block::paste(int u, int v, MatrixXf * target, Vector<Vector<Point> > * targetAsPos)
	{
		setUV(u, v);

                float final;

		MatrixXf mask = getCutMask(u, v, target);

		for(int y = 0; y < BlockSize; y++)
		{
			for(int x = 0; x < BlockSize; x++)
			{
				if(mask(y,x))
				{

                                        final = (*src)(p.y + y, p.x + x);

					(*target)(y + v, x + u) = final;
					(*targetAsPos)[y + v][x + u] = Point(p.x + x, p.y + y);
				}
			}
		}
	}

	void Block::setUV(int & x, int & y)
	{
		if(type == L_SHAPED || type == N_SHAPED)
		{
			x -= (BandSize - 1);
			y -= (BandSize - 1);
		}
		else
		{
			x -= (BandSize - 1);
		}
	}

	MatrixXf Block::getCutMask(int u, int v, MatrixXf * target)
	{
		MatrixXf difference = src->block(p.y, p.x, BlockSize, BlockSize) - target->block(v, u, BlockSize, BlockSize);
		MatrixXf overlap = difference.array().square();

		MatrixXf mask = MatrixXf::Ones(BlockSize, BlockSize);

		// Find best cut
		if(overlap.sum() != 0)
		{
			CutMask cut = CutMask(overlap, type);
			mask = cut.getMask();

			/*
			QImage * img = new QImage(BlockSize, BlockSize, QImage::Format_RGB32);
			img->fill(0);
			for(int x = 0; x < BlockSize; x++){
				for(int y = 0; y < BlockSize; y++){
					if(mask(y,x))
						img->setPixel(x,y, qRgb(0,0,0));
					else
						img->setPixel(x,y, qRgb(255,255,255));
				}
			}
			char fileName [125];
			sprintf(fileName, "mask_%d_%d.png", (int)type, xxxxx);
			img->save(fileName);
			*/
		}

		return mask;
	}

}
