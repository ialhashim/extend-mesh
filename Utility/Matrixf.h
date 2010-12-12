#pragma once

// for Eigen Library's sake!
#undef min
#undef max

// Eigen Library
#include <Eigen/Core>
using namespace Eigen;

#include <QColor>
#include <QImage>
#include <QPixmap>

#include <vector>
#define Vector std::vector 
typedef Vector< Vector<float> > Vector2Df;

#include "Point.h"

namespace Matrixf
{
	Vector<QColor> pseudoColors();

	MatrixXf matrixFromImage(QImage * img);
	QImage imageFromMatrix(const MatrixXf & m, bool isUseFalseColors = false);
	QPixmap pixmapFromMatrix(const MatrixXf & m, bool isUseFalseColors = false);
	QImage imageFromPosMatrix(QImage & src, MatrixXi pos);

	float getBilinear(const MatrixXf & src, float dX, float dY);
	float getNearestNeighbour(const MatrixXf & src, float dX, float dY);
	MatrixXf resizeAsImage(const MatrixXf & src, int newWidth, bool isWithBias = false); // Bilinear
	MatrixXf resizeBasic(const MatrixXf & src, int newWidth, int newHeight = -1); // nearest neighbor

	MatrixXf fromVector2Df(const Vector2Df& v);
	MatrixXf resizeAsImage(const Vector2Df& v, int newWidth, bool isWithBias = false);

	MatrixXf synthFromMap(MatrixXf& src_cs, const std::vector<std::vector<Point> > & synth_pattern);
	MatrixXf tileBlendFromMap(MatrixXf& src_cs, int bandSize, int tileCount);

	MatrixXf pattern( int width, int height, QString kind, float scale = 8);

	Vector<float> column(int c, const MatrixXf& M);

	Vector2Df toVector2Df(const MatrixXf& m);

	void outputToFile( const MatrixXf& m, QString fileName );
	void outputAsImage( const MatrixXf& m, QString fileName, bool isUseFalseColors = false);

	void rotate(MatrixXf& m, float amount, bool isLeft = true);

	double bias(double x);
};

// Missing math functions... why MSFT, why..
// from:http://www.cs.princeton.edu/introcs/21function/ErrorFunction.java.html
inline double erf(double z) {
	double t = 1.0 / (1.0 + 0.5 * abs(z));

	// use Horner's method
	double ans = 1 - t * exp( -z*z   -   1.26551223 +
		t * ( 1.00002368 +	t * ( 0.37409196 + 
		t * ( 0.09678418 + 	t * (-0.18628806 + 
		t * ( 0.27886807 + 	t * (-1.13520398 + 
		t * ( 1.48851587 + 	t * (-0.82215223 + 
		t * ( 0.17087277))))))))));
	if (z >= 0) return  ans;
	else        return -ans;
}

inline int Round (double x) {
	int i = (int) x; 
	if (x >= 0.0) return ((x-i) >= 0.5) ? (i + 1) : (i); 
	else return (-x+i >= 0.5) ? (i - 1) : (i); 
} 
