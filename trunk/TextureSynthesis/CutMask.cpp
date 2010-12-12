#include "CutMask.h"

CutMask::CutMask(MatrixXf & overlap, BlockType type)
{
	for(int y = 0; y < BlockSize; y++)
	{
		for(int x = 0; x < BlockSize; x++)
		{
			points[nodes.size()] = Point(x,y);
			nodes[Point(x,y)] = nodes.size();
		}
	}

	protectCore(overlap, type);

	costMatrix = overlap;

	for(int y = 0; y < BlockSize; y++)
	{
		for(int x = 0; x < BlockSize; x++)
		{
			Point p1 = Point(x    , y    );
			Point p2 = Point(x + 1, y    );
			Point p3 = Point(x    , y + 1);

			if(point(p2))	g.AddEdge(nodes[p1], nodes[p2], cost(p1,p2));
			if(point(p3))	g.AddEdge(nodes[p1], nodes[p3], cost(p1,p3));
		}
	}

	// Add start and end nodes
	int start = nodes.size();
	int end = start + 1;

	BlockType originalType = type;
	
	if(originalType == V_BOTHSIDES)
	{
		// convert it to N_SHAPED
		for(int i = BandSize - 1; i < BlockSize - BandSize; i++)
		{
			g.SetEdgeWeight(nodes[Point(i,0)], nodes[Point(i+1,0)], 0);
			g.SetEdgeWeight(nodes[Point(i+1,0)], nodes[Point(i,0)], 0);
		}

		type = N_SHAPED;
	}

	// connect start and end to graph, first is start node case
	switch(type)
	{
	case VERTICAL:
		for(int i = 0; i < BandSize; i++)
			g.AddEdge(start, nodes[Point(i, 0)], 0);
		break;

	case L_SHAPED:
		//for(int i = 0; i < BandSize; i++)
		//	g.AddEdge(start, nodes[Point(BlockSize - 1, i)], 0);
		g.AddEdge(start, nodes[Point(BlockSize - 1, BandSize - 1)], 0);
		break;

	case N_SHAPED:
		for(int i = 0; i < BandSize; i++)
			g.AddEdge(start, nodes[Point((BlockSize - 1) - i, BlockSize - 1)], 0);
		break;

        case NONE_BLOCK:
        case V_BOTHSIDES:
        case HORIZONTAL:
                break;
	}

	// end node connections
	for(int i = 0; i < BandSize; i++)
		g.AddEdge(end, nodes[Point(i, BlockSize - 1)], 0);

	// Find the shortest path
	std::list<int> path = g.DijkstraShortestPath(start, end);

	// Remove start and end points
	path.pop_front();
	path.pop_back();

	// turn into boolean mask
	mask = MatrixXf::Ones(BlockSize, BlockSize);

	for(std::list<int>::iterator i = path.begin(); i != path.end(); i++)
	{
		Point p = points[*i];
		mask(p.y, p.x) = 0;
	}

	fillMask(mask, type);

	// Special case: fix V_BOTHSIDES mask
	if(originalType == V_BOTHSIDES)
	{
		for(int i = BandSize; i < BlockSize - BandSize; i++)
		{
			mask(0, i) = 1.0f;
		}
	}
}

float CutMask::cost(Point p1, Point p2)
{
	return costMatrix(p1.y, p1.x) + costMatrix(p2.y, p2.x);
}

bool CutMask::point(Point p)
{
	return (nodes.find(p) != nodes.end());
}

void CutMask::protectCore(MatrixXf & overlap, BlockType type)
{
	switch(type)
	{
	case VERTICAL:
                overlap.block(0, BandSize, BlockSize, BlockSize - BandSize) = MatrixXf::Constant(BlockSize, BlockSize - BandSize, FLT_MAX);
		break;

	case V_BOTHSIDES:
		{
			int width = BlockSize - (2 * BandSize);
			overlap.block(0, BandSize, BlockSize, width) = MatrixXf::Constant(BlockSize, width, FLT_MAX);
		}break;

	case L_SHAPED:
		{
			int size = BlockSize - BandSize;
			overlap.block(BandSize, BandSize, size, size) = MatrixXf::Constant(size, size, FLT_MAX);
		}break;

	case N_SHAPED:
		{
			int width = BlockSize - (2 * BandSize);
			int height = BlockSize - BandSize;
			overlap.block(BandSize, BandSize, height, width) = MatrixXf::Constant(height, width, FLT_MAX);
		}break;

        case NONE_BLOCK:
        case HORIZONTAL:
                break;
	}
}

void CutMask::fillMask(MatrixXf & m, BlockType type)
{
	switch(type)
	{
	case VERTICAL:
		for(int i = 0; i < BlockSize; i++)
			fillMask(0, i, m);
		break;

	case V_BOTHSIDES:
		for(int i = 0; i < BlockSize; i++)
		{
			fillMask(0, i, m);
			fillMask(BlockSize - 1, i, m);
		}
		break;

	case L_SHAPED:
	case N_SHAPED:
		for(int i = 0; i < BlockSize; i++)
		{
			fillMask(i, 0, m);
			fillMask(0, i, m);

			if(type == L_SHAPED && i < BandSize)
				fillMask(BlockSize - 1, i, m);
			else if(type == N_SHAPED)
				fillMask(BlockSize - 1, i, m);
		}
		break;

        case NONE_BLOCK:
        case HORIZONTAL:
                break;
	}
}

void CutMask::fillMask(int x, int y, MatrixXf & m)
{
	if(x >= 0 && x < BlockSize && y >= 0 && y < BlockSize && m(y,x) == 1)
	{
		m(y,x) = 0; 

		fillMask(x + 1, y,     m);       
		fillMask(x - 1, y,     m);       
		fillMask(x,     y + 1, m);
		fillMask(x,     y - 1, m);              
	}   
}

MatrixXf CutMask::getMask()
{
	return mask;
}
