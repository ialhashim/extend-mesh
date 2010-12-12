#pragma once

#include <Eigen/Core>
using namespace Eigen;

#include "Grid.h"

#include <QGLWidget>
#include <QMouseEvent>
#include <QWheelEvent>

enum DrawMode { GRID_POINTS, SQUARE_AVERAGE };

class GridVisualizer : public QGLWidget
{
Q_OBJECT

private:
	Grid * grid;

	HashMap<int, Vec> pointPosMap;
	Vec PointFromSquare(GridPoint & gp, int u, int v);

	HashMap<int, Color4> pointColorMap;

	DrawMode Mode;
	bool isReady;
	bool isDrawPatch;
	double pointSize;

	void computeSquaresValues();
	void computePoints();

	void drawGridPoints();
	void drawSquareAverage();
	void drawPatch(bool isFilled = false);
	int drawEdge( Vec p0, Vec p1, double threshold );

	void bakePatch();

	bool isInRectangle(int x, int y, Rect & r);

	int x_count, y_count, squareWidth, squareHeight;
	int scroll_y, total_y;

	MatrixXf squareValue;
	
	IntSet showFaces;

	MatrixXf patchMask;

	int selectedLevel;

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();

	void mouseReleaseEvent (QMouseEvent * e);
	void mousePressEvent (QMouseEvent * e);
	void wheelEvent (QWheelEvent * e);

public:
	GridVisualizer(QWidget* parent);

	Rect Crop();

	MatrixXf getSquaresValues();

public slots:
	void SetGrid(Grid*);
	void SetLevel(int);
	void ToggleShowPatch();

};
