#include "Synthesizer.h"

int BlockSize = 15;
int BandSize = 5;

int synthesisWidth = 0;

int cur_x = 0;
int cur_y = 0;

namespace Synth
{

Synthesizer::Synthesizer(int BottomPadding)
{
	this->bottomPadding = BottomPadding;
}

void Synthesizer::init(const MatrixXf & source, int synthWidth, int blockSize, int bandSize)
{
	isDone = false;

	cur_x = 0;
	cur_y = 0;

	// Enforce block and band bounds
	blockSize = Max(4, Min(blockSize, source.cols() - 1));
	bandSize = Max(3, Min(bandSize, blockSize / 2));

	// Set global settings
	BlockSize = blockSize;
	BandSize = bandSize;
	synthesisWidth = synthWidth;

	// Prepare matrices
	this->src = source;
	this->target = MatrixXf::Constant(source.rows(), synthWidth, EMPTY_PIXEL);
	this->debug = MatrixXf::Zero(source.rows(), source.cols());

        this->targetAsPos = Vector<Vector<Point> >(source.rows(), Vector<Point>(synthWidth, Point(-1,-1)));
	
	// Prepare cut variables
	int src_width = source.cols();
	patchSize = BlockSize - BandSize;
	cut = ceil(src_width / 2.0f) - 1;
	jump = (floor((synthesisWidth - src_width) / (float)patchSize)) * patchSize;

	print("Copying two target pieces..");

	// Analyize input, and copy our two pieces to target
	for(int y = 0; y < source.rows(); y++)
	{
		for(int x = 0; x < source.cols(); x++)
		{
			positions[source(y,x)].push_back(Point(x,y));

			if(x >= cut)
			{
				target(y, x + jump) = src(y, x);
				targetAsPos[y][x + jump] = Point(x,y);
			}
			else
			{
				target(y, x) = src(y, x);
				targetAsPos[y][x] = Point(x,y);
			}
		}
	}
}

void Synthesizer::run()
{
	synthesizeAll();
}

void Synthesizer::synthesizeAll()
{
	print("Starting synthesis.. 0%");

	int cols = ( synthesisWidth - src.cols())	/ patchSize;
	int rows = ( src.rows() )					/ patchSize;

	int start_x = cut - 1;

	cur_x = start_x;
	cur_y = 0;

	//QString::arg();
	int count = 0;
	float total = (float)(cols * rows) + rows + cols;
	float percent;

	// Top row
	for(int x = 0; x <= cols; x++)
	{
		synthesizeNext();
		cur_x += patchSize;

		percent = count++ / total;
		print(QString("Synthesizing.. %1").arg((int)(100 * percent)) + QString("%"));
	}

	// Remaining rows
	cur_x = start_x;
	cur_y += BlockSize - 1;

	for(int y = 1; y < rows; y++)
	{
		for(int x = 0; x <= cols; x++)
		{
			synthesizeNext();
			cur_x += patchSize;

			percent = count++ / total;
			print(QString("Synthesizing.. %1").arg((int)(100 * percent)) + QString("%"));
		}

		cur_x = start_x;
		cur_y += patchSize;
	}

	isDone = true;
}


void Synthesizer::synthesizeNext()
{
	// Bounds checks
	if(cur_x + patchSize > target.cols() || cur_y + patchSize > target.rows())	return;

	if(BandSize > BlockSize / 2) 
		BandSize = (BlockSize / 2) - 1;

	BlockType foundType = getBlockType();

	ReducedSet set = getMatchingBlocks(foundType);

	if(set.size())
	{
		Block best_block = getBestBlock( set );

                best_block.paste(cur_x, cur_y, &target, &targetAsPos);
	}
}

Block Synthesizer::getBestBlock(ReducedSet & set)
{
	Block candidate;
	BlockType type = set.begin()->second.type;

	float Si, Smin = FLT_MAX;

	WeightMatrix weight(type);
	MatrixXf target_block, difference;

	// Prepare target block
	if(type == VERTICAL || type == V_BOTHSIDES)
	{
		target_block = target.block(cur_y, cur_x - (BandSize - 1), BlockSize, BlockSize);
	}
	else if(type == L_SHAPED || type == N_SHAPED)
	{
		target_block = target.block(cur_y - (BandSize - 1), cur_x - (BandSize - 1), BlockSize, BlockSize);
	}

	int bx = 0;
	int by = 0;

	// Check all blocks in our reduced set
	for (ReducedSet::iterator i = set.begin(); i != set.end(); ++i) {
		bx = i->second.p.x;
		by = i->second.p.y;

		difference = src.block(by, bx, BlockSize, BlockSize) - target_block;
	
		Si = (weight.m.array() * difference.array().square()).sum();

		/* Location matters?
		int half_src = src.cols() / 2;
		float x_penalty = 1 + abs(bx - half_src) + abs(Bi.x() - cur_x);
		float y_penalty = 1 + abs(by - cur_y);
	
		float locationPenalty = (float)(x_penalty * y_penalty);
		*/

		Si = Si;// * locationPenalty;

		if(Si < Smin){
			Smin = Si;
			candidate = i->second;

			if(Smin == 0) break;
		}
	}

	return candidate;
}

ReducedSet Synthesizer::getMatchingBlocks(BlockType type)
{
	ReducedSet set;

        CheckList list;

	switch(type)
	{
		case VERTICAL:
			for(int j = 0; j < BlockSize; j++)
				addBlocks(Point(cur_x, cur_y + j), BandSize - 1, j, VERTICAL, set, list);
			break;

		case V_BOTHSIDES:
			{
				int side = (BlockSize - 2 * BandSize) + 1;

				for(int j = 0; j < BlockSize; j++)
				{
					addBlocks(Point(cur_x, cur_y + j), BandSize - 1, j, V_BOTHSIDES, set, list);
					list.clear();
					addBlocks(Point(cur_x + side, cur_y + j), BlockSize - BandSize, j, V_BOTHSIDES, set, list);
				}
			}
			break;

		case L_SHAPED:
			for(int j = 0; j < (BlockSize - BandSize) + 1; j++)
			{
				addBlocks(Point(cur_x, cur_y + j), BandSize - 1, (BandSize - 1) + j, L_SHAPED, set, list); // Y-Direction
				addBlocks(Point(cur_x + j, cur_y), (BandSize - 1) + j, BandSize - 1, L_SHAPED, set, list); // X-Direction
			}
			break;

		case N_SHAPED:
			{
				int side = (BlockSize - 2 * BandSize) + 1;

				// Y-Direction
				for(int j = 0; j < (BlockSize - BandSize) + 1; j++)
				{
					addBlocks(Point(cur_x, cur_y + j), BandSize - 1, (BandSize - 1) + j, N_SHAPED, set, list);
					addBlocks(Point(cur_x + side, cur_y + j), BlockSize - BandSize, (BandSize - 1) + j, N_SHAPED, set, list);
				}

				// X-Direction
				for(int j = 0; j < side; j++)
					addBlocks(Point(cur_x, cur_y + j), BandSize - 1, (BandSize - 1) + j, N_SHAPED, set, list);
			}
			break;

                case NONE_BLOCK:
                case HORIZONTAL:
                        break;
	}

        list.size();

	return set;
}

void Synthesizer::addBlocks(Point at, int deltaX, int deltaY, 
							BlockType type, 
							ReducedSet & set,
							CheckList & list)
{
	float color = target(at.y, at.x);

	Point curr_point, relative;

	//if(!list[color])
	{
		// Check all possible blocks matching current color
                for(int i = 0; i < (int)positions[color].size(); i++){
			curr_point = positions[color].at(i);
			relative = Point(curr_point.x - deltaX, curr_point.y - deltaY);

			if(isValidPatch(relative))
			{
				if( set.find(relative) == set.end() )
				{
					set[relative] = Block(relative, &src, type);
					//list[color] = true;
				}
			}
		}
	}

	// If nothing is there, check everything at that pixel
        if((int)set.size() < 1)
	{
                for(StdMap<float, Vector<Point> >::iterator it = positions.begin(); it != positions.end(); it++)
		{
			color = it->first;

                        for(int i = 0; i < (int)positions[color].size(); i++)
			{
				curr_point = positions[color].at(i);
				relative = Point(curr_point.x - deltaX, curr_point.y - deltaY);

				if(isValidPatch(relative))
				{
					if( set.find(relative) == set.end() )
					{
						set[relative] = Block(relative, &src, type);
						//list[color] = true;
					}
				}
			}
		}
	}

        list.size();
}

BlockType Synthesizer::getBlockType()
{
	if(cur_y < BandSize)
	{
                Point p = pixel(cur_x + patchSize, cur_y);
                if( empty(p) )
			return VERTICAL;
		else
			return V_BOTHSIDES;
	}
	else if(cur_y + patchSize < src.rows())
	{
                Point p = pixel(cur_x + patchSize, cur_y + patchSize);
                if( empty(p) )
			return L_SHAPED;
		else
			return N_SHAPED;
	}

	return NONE_BLOCK;
}

void Synthesizer::crop()
{
	int widthCrop = target.cols();
	int heightCrop = this->target.rows() - bottomPadding;
 	
	while(target(0, --widthCrop) == EMPTY_PIXEL && widthCrop >= 0); 

	// Convert index to width
	widthCrop += 1;

	// As pixel values
	MatrixXf temp = target.block(0, 0, heightCrop, widthCrop); 
	target = temp;

	// As positions
        Vector<Vector<Point> > tempAsPos = Vector<Vector<Point> >(heightCrop, Vector<Point>(widthCrop, Point(-1,-1)));
	for(int x = 0; x < widthCrop; x++)
	{
		for(int y = 0; y < heightCrop; y++)
		{
			tempAsPos[y][x] = Point(targetAsPos[y][x].x, targetAsPos[y][x].y % heightCrop);
		}
	}
	targetAsPos = tempAsPos;
}

bool Synthesizer::isValidPatch(Point & p)
{
	return p.x >= 0 && p.x < (src.cols() - BlockSize) 
		   && p.y >= 0 && p.y < (src.rows() - BlockSize);
}

MatrixXf Synthesizer::result()
{
	return target;
}

Vector<Vector<Point> > Synthesizer::resultAsPos()
{
	return targetAsPos;
}

}
