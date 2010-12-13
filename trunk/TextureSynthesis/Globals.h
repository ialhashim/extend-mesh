#pragma once

#include <stdio.h>
#include <time.h>
#include <vector>
#include <map>
#include "float.h"

#define Vector std::vector
#define StdMap std::map
typedef std::map<float, bool> CheckList;

// Eigen library
#include <Eigen/Core>
using namespace Eigen;

// Qt Timer
#define Timer QElapsedTimer
#define CreateTimer(timer)  QElapsedTimer timer; timer.start()

extern int BlockSize;
extern int BandSize;
extern int synthesisWidth;
extern int cur_x;
extern int cur_y;

enum BlockType {NONE_BLOCK, VERTICAL, V_BOTHSIDES, HORIZONTAL, L_SHAPED, N_SHAPED };

#define EMPTY_PIXEL (FLT_MIN)

#define BLUE (-2)
#define GREEN (-3)
#define RED (-4)
#define YELLOW (-5)

// ========================================================================
// Basic 2D point
// ========================================================================
#include "Point.h"

// ========================================================================
// Useful macros
// ========================================================================
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Between(x, MIN, MAX) ((x >= MIN && x <= MAX) ? (true) : (false))

static inline void printMatrixXf(MatrixXf & mat, bool nonZero = false)
{
	printf("\nMatrix (rows = %d , cols = %d)\n", mat.rows(), mat.cols());
	for(int y = 0; y < mat.rows(); y++){
		if(nonZero)
		{
			for(int x = 0; x < mat.cols(); x++)
			{
				if(mat(y,x) >= 1.0f)
					printf("%d ", 1);
				else
					printf("%d ", 0);
			}
		}
		else
		{
			for(int x = 0; x < mat.cols(); x++)
				printf("%1.1f ", mat(y,x));
		}

		printf(" #\n");
	}
	printf("\n");
}

static inline void printMatrixXi(MatrixXi & mat)
{
	printf("\nMatrix (rows = %d , cols = %d)\n", mat.rows(), mat.cols());
	for(int y = 0; y < mat.rows(); y++){

		for(int x = 0; x < mat.cols(); x++)
			printf("%d ", mat(y,x));

		printf(" #\n");
	}
	printf("\n");
}

static inline MatrixXf fromLeft(const MatrixXf & mat, int cols)
{
	return mat.topLeftCorner(mat.rows(), cols);
}

inline MatrixXf fromRight(const MatrixXf & mat, int cols)
{
	return mat.topRightCorner(mat.rows(), cols);
}

inline MatrixXf RightWeight(int rows, int cols)
{
	MatrixXf m = MatrixXf::Ones(rows, cols);

	for(int x = 0; x < cols; x++)
		m(0,x) /= (x + 1);

	for(int y = 0; y < rows; y++)
		m.block(y,0,1,cols) = m.block(0,0,1,cols);

	return m;
}

inline MatrixXf LeftWeight(int rows, int cols)
{
	MatrixXf w = RightWeight(rows, cols);
	MatrixXf m = MatrixXf::Zero(rows, cols);

	for(int x = 0; x < cols; x++)	m(0,x) = w(0,(cols-1) - x);
	for(int y = 0; y < rows; y++)	m.block(y,0,1,cols) = m.block(0,0,1,cols);

	return m;
}

inline MatrixXi InverseMask(const MatrixXi & mask)
{
	return (mask - MatrixXi::Ones(mask.rows(), mask.cols())).array().abs();
}

inline MatrixXi NumberedMatrix(int rows, int cols, int start = 0)
{
	int size = rows*cols;

	Vector<int> data = Vector<int>(size);
	for(int i = 1; i < size + 1; i++)
		data[i - 1] = i + start;

	return Eigen::Map<MatrixXi>(&data[0],rows,cols);
}

inline MatrixXf FromPositionMatrix(const MatrixXf & src, const MatrixXi & pos)
{
	MatrixXf result = MatrixXf::Zero(pos.rows(), pos.cols());

	for(int y = 0; y < pos.rows(); y++)
	{
		for(int x = 0; x < pos.cols(); x++)
			result.coeffRef(y,x) = src((pos(y,x) - 1));
	}

	return result;
}

inline double randDouble(double low, double high)
{
	return (rand() / (static_cast<double>(RAND_MAX) + 1.0))	* (high - low) + low;
}
