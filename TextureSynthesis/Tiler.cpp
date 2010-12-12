#include "Tiler.h"

Tiler::Tiler(const MatrixXf & src, int bandSize, int tileCount)
{
	src_img = src;

	band_size = Min(bandSize, src.cols() - 1);
	tile_count = Max(1, tileCount); // at least one

	is_done = false;
}

MatrixXi Tiler::tileAsPos()
{
	MatrixXf square_diff, weighted_square_diff;
	MatrixXf weight = LeftWeight(src_img.rows(), band_size);

	MatrixXf L = src_img;
	MatrixXi Li = NumberedMatrix(src_img.rows(), src_img.cols());

	// Final result for tiles as positions
	Vector<MatrixXi> texture;
	texture.reserve(tile_count);

	// last tile result
	MatrixXi result;

	for(int i = 0; i < tile_count; i++)
	{
		int l_rows = L.rows(), l_cols = L.cols();
		int bandStart = l_cols - band_size;

		square_diff = (fromRight(L, band_size) - fromLeft(src_img, band_size)).array().square();

		//weighted_square_diff = square_diff;
		weighted_square_diff = square_diff.array() * weight.array();
		//float total_cost = (weighted_square_diff).sum();

		MatrixXi mask = cut_mask(weighted_square_diff);

		// Identity = all zeros
		result = MatrixXi::Zero(l_rows, l_cols + (src_img.cols() - band_size));

		// Copy positions to result
		for(int y = 0; y < result.rows(); y++){
			int mask_x = 0;

			for(int x = 0; x < result.cols(); x++){
				if(x < bandStart)
				{
					result.coeffRef(y,x) = Li.coeffRef(y,x);
				}
				else
				{
					if(mask_x < mask.cols() && !mask(y, mask_x++))
						result.coeffRef(y,x) = Li.coeffRef(y, x);
					else
						result.coeffRef(y,x) = Li.coeffRef(y, (x - (l_cols -band_size)) % l_cols);
				}
			}
		}

		// Split into two parts, Texture part:
		texture.push_back(result.topLeftCorner(l_rows, l_cols - band_size));

		// New input part
		Li = result.topRightCorner(l_rows, l_cols);
		L = FromPositionMatrix(src_img, Li);
	}

	// Last part
	texture.push_back(result.topRightCorner( src_img.rows(), src_img.cols()));

	int rows = src_img.rows();
	int cols = texture[0].cols();

	// compute actual target column count
	int target_cols = 0;
	for(size_t i = 0; i < texture.size(); i++) target_cols += texture[i].cols();

	// Create target pos matrix
	MatrixXi target = MatrixXi::Ones(rows, target_cols);

	for(size_t i = 0; i < texture.size(); i++)
		target.block(0, i * cols, rows, texture[i].cols()) = texture[i];

	return target;
}

MatrixXi Tiler::stitchAsPos(const MatrixXi & left_img, const MatrixXi & right_img, const MatrixXi & mask)
{
	MatrixXi invMask = InverseMask(mask);

	MatrixXi leftImg = left_img;
	MatrixXi rightImg = right_img;

	leftImg.topRightCorner(leftImg.rows(), mask.cols()) =	leftImg.topRightCorner( leftImg.rows(), invMask.cols()).array() * invMask.array();
	rightImg.block(0,0, rightImg.rows(), mask.cols()) =	rightImg.topLeftCorner(rightImg.rows(), mask.cols()).array() * mask.array();

	MatrixXi result = MatrixXi::Zero(left_img.rows(), left_img.cols() + (right_img.cols() - mask.cols()));

	result.block(0,0,leftImg.rows(), leftImg.cols()) = leftImg;
	result.block(0,leftImg.cols() - mask.cols(), leftImg.rows(), rightImg.cols()) += rightImg;

	return result;
}

MatrixXi Tiler::cut_mask(const MatrixXf & costMatrix)
{
	Graph<int, float> g;

	StdMap<Point, int> node;
	StdMap<int, Point> point;

	int rows = costMatrix.rows();
	int cols = costMatrix.cols();

	// Register nodes
	for(int y = 0; y < rows; y++){
		for(int x = 0; x < cols; x++){
			point[node.size()] = Point(x,y);
			node[Point(x,y)] = node.size();
		}
	}

	// Connect with costs
	for(int y = 0; y < rows; y++){
		for(int x = 0; x < cols; x++){
			Point p1 = Point( x     , y     );
			Point p2 = Point( x + 1 , y     );
			Point p3 = Point( x     , y + 1 );

			if(node.find(p2) != node.end()){
				float cost = costMatrix(p1.y, p1.x) + costMatrix(p2.y, p2.x);
				g.AddEdge(node[p1], node[p2], cost);
			}

			if(node.find(p3) != node.end()){
				float cost = costMatrix(p1.y, p1.x) + costMatrix(p3.y, p3.x);
				g.AddEdge(node[p1], node[p3], cost);
			}
		}
	}

	int start = node.size();
	int end = start + 1;

	// Connect start and end nodes to graph
	for(int i = 0; i < cols; i++)
	{
		g.AddEdge(start, node[Point(i, 0)], 0);
		g.AddEdge(end, node[Point(i, rows - 1)], 0);
	}

	// Get shortest path
	std::list<int> path = g.DijkstraShortestPath(start, end);

	// Create full mask and set path nodes to zero
	MatrixXi mask = MatrixXi::Ones(rows, cols);
	for(std::list<int>::iterator i = path.begin(); i != path.end(); i++)
	{
		Point p = point[*i];
		mask.coeffRef(p.y, p.x) = 0;
	}

	// fill in the rest of the mask, row by row
	for(int i = 0; i < rows; i++)
	{
		fillMask(0, i, mask); // vertical (left)
	}

	/*for(int i = 0; i < cols; i++)
	{
		fillMask(i, 0, mask); // horiz (top)
		fillMask(i, rows - 1, mask); // horiz (bottom)
	}*/

	return mask;
}

void Tiler::fillMask(int x, int y, MatrixXi & m)
{
	if(x >= 0 && x < m.cols() && y >= 0 && y < m.rows() && m(y,x) == 1)
	{
		m.coeffRef(y,x) = 0;

		fillMask(x + 1, y,     m);
		fillMask(x - 1, y,     m);
		fillMask(x,     y + 1, m);
		fillMask(x,     y - 1, m);
	}
}

MatrixXf Tiler::getTarget()
{
	return target_img;
}

MatrixXf Tiler::getSource()
{
	return src_img;
}

bool Tiler::isDone()
{
	return is_done;
}
