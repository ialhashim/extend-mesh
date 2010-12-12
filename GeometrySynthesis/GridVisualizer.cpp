#include "GridVisualizer.h"

#include "Matrixf.h"
using namespace Matrixf;

GridVisualizer::GridVisualizer(QWidget *parent) : QGLWidget(parent)
{
	this->Mode = SQUARE_AVERAGE;
	this->isReady = false;

	this->pointSize = 4.0f;

	this->scroll_y = 1.0;
	this->total_y = 1.0;

	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	this->selectedLevel = 0;
}

void GridVisualizer::SetGrid(Grid * sourceGrid)
{
	this->grid = sourceGrid;

	x_count = Max(1, grid->lengthCount);
	y_count = Max(1, grid->widthCount);

	squareWidth = width() / x_count;
	squareHeight = Max(10, height()) / y_count;
	total_y = squareHeight * y_count;

	computeSquaresValues();
	computePoints();

	this->update();

	this->isReady = true;
}

void GridVisualizer::SetLevel(int level)
{
	this->selectedLevel = level;

	computeSquaresValues();
	computePoints();

	this->update();
}

void GridVisualizer::ToggleShowPatch()
{
	this->isDrawPatch = !isDrawPatch;

	this->update();
}

void GridVisualizer::computeSquaresValues()
{
	squareValue = MatrixXf::Constant(grid->widthCount, grid->lengthCount, FLT_MIN);

	Vector<GridSquare> * squares = grid->getSquares( selectedLevel );

	for(Vector<GridSquare>::iterator square = squares->begin(); square != squares->end(); square++)
	{
		int u = square->u;
		int v = square->v;

		squareValue(u,v) = square->height;
	}
}

void GridVisualizer::computePoints()
{
	pointColorMap.clear();
	pointPosMap.clear();

	squareHeight = Max(1, squareHeight);
	total_y = Max(1, total_y);

	double totalWidth = squareWidth * x_count;
	double totalHeight = squareHeight * y_count;

	if(this->isReady)
	{
		double heightScale = grid->getHeightRange( 0 );

		Vector<GridSquare> * squares = grid->getSquares( selectedLevel );

		// For each square on the grid
		for(Vector<GridSquare>::iterator square = squares->begin(); square != squares->end(); square++)
		{
			// Get collection of points
			Vector<GridPoint> * points = &square->points;

			// For each point in the square
			for(Vector<GridPoint>::iterator p = points->begin(); p != points->end(); p++)
			{
				// Color value
				double value = (p->h - grid->min_height[selectedLevel]) / heightScale;
				value = Max(0.0f, Min(1.0f, value));

				// Record point color
				pointColorMap[p->corrPoint] = Color4::fromRGB(value, value, value);

				// Using accurate coordinates
				Vec v = grid->pointVecMap[p->corrPoint];
	
				// Place on visualizer window
				Vec interpolatedPoint(v.x * totalWidth, v.y * totalHeight,0);

				// scroll feature
				interpolatedPoint.y = Mod((int)(interpolatedPoint.y + (scroll_y * squareHeight)), (int)totalHeight);

				// Record point position
				pointPosMap[p->corrPoint] = Vec(interpolatedPoint.x, interpolatedPoint.y, 0);
			}
		}
	}

	// Find the faces that should be visible on this patch
	Mesh * m = grid->getBaseMesh();
	Vector<int> selectedFaces = *grid->getSelectedFaces();
	foreach(int fi, selectedFaces)
	{
		Face * f = m->f(fi);

		int v0 = f->vIndex[0];
		int v1 = f->vIndex[1];
		int v2 = f->vIndex[2];

		HashMap<int, Vec>::iterator end = pointPosMap.end();

		if(pointPosMap.find(v0) != end && pointPosMap.find(v1) != end && pointPosMap.find(v2) != end)
		{
			showFaces.insert(fi);
		}
	}
}

MatrixXf GridVisualizer::getSquaresValues()
{
	return squareValue;
}

Vec GridVisualizer::PointFromSquare(GridPoint & gp, int u, int v)
{
	Vec p, corner[4];

	corner[1].x = u;
	corner[2].x = u;
	corner[2].y = v;
	corner[3].y = v;

	for(int i = 0; i < 4; i++)
	{
		p.x += corner[i].y * gp.w[i];
		p.y += corner[i].x * gp.w[i];
		//p.z += 0;
	}

	return p;
}

Rect GridVisualizer::Crop()
{
	Rect crop;

	crop.t = 0;
	crop.b = crop.t + grid->widthCount;

	// Default values (no cropping)
	crop.l = 0;
	crop.r = grid->lengthCount;

	bakePatch();

	int startX = 0;
	int endX = grid->lengthCount;

	for(int u = 0; u < grid->widthCount; u++)
	{
		// Left side
		for(int v = 0; v < grid->lengthCount; v++)
		{
			if(patchMask(u, v) == 0.0f)
				startX = Max(startX, v);
			else
				break;
		}

		// Right side
		for(int v = 1; v < grid->lengthCount; v++)
		{
			if(patchMask(u, grid->lengthCount - v) == 0.0f)
				endX = Min(endX, grid->lengthCount - v);
			else
				break;
		}
	}

	crop.l = startX;
	crop.r = endX;

	return crop;
}

void GridVisualizer::initializeGL()
{
	glShadeModel(GL_SMOOTH);
	qglClearColor(QColor(30,30,90)); // Background color: blue-ish

	isDrawPatch = false;
}

void GridVisualizer::resizeGL(int width, int height)
{
	glViewport(0,0,width,height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0,width,height,0.0,-1.0,1.0);
	glMatrixMode(GL_MODELVIEW);

	if(isReady)
	{
		squareWidth = this->width() / x_count;
		squareHeight = this->height() / y_count;

		total_y = squareHeight * y_count;

		computePoints();
	}
}

void GridVisualizer::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(isReady)
	{
		// Draw grid lines
		glLineWidth(1.0f);
		glBegin(GL_LINES);
		glColor4f(1,0,0, 0.25f);  // Red
		for(int x = 0; x < x_count; x++)
		{
			glVertex3f(x * squareWidth, 0, 0);
			glVertex3f(x * squareWidth, y_count * squareHeight, 0);
		}
		glColor4f(0,0,1, 0.25f);  // Blue
		for(int y = 0; y < y_count; y++)
		{
			glVertex3f(0, y * squareHeight, 0);
			glVertex3f(x_count * squareWidth, y * squareHeight, 0);
		}
		glEnd();

		// Draw points / squares
		if(Mode == GRID_POINTS)			drawGridPoints();
		if(Mode == SQUARE_AVERAGE)		drawSquareAverage();
		if(isDrawPatch)					drawPatch();
	}
}

void GridVisualizer::drawGridPoints()
{
	if(grid->isReady)
	{
		glPointSize(pointSize);
		glBegin(GL_POINTS);

		Vector<GridSquare> * squares = grid->getSquares( selectedLevel );

		// For each square on the grid
		for(Vector<GridSquare>::iterator square = squares->begin(); square != squares->end(); square++)
		{
			// Get collection of points
			Vector<GridPoint> * points = &square->points;

			// For each point in the square
			for(Vector<GridPoint>::iterator p = points->begin(); p != points->end(); p++)
			{
				// Color value
				double value = pointColorMap[p->corrPoint].r();

				if(grid->activeGridSquare == &(*square) && grid->activeGridPoint == &(*p))
					glColor3f(1, value / 2.0, value / 2.0);
				else
					glColor3f(value, value, value);

				// Get recorded position
				Vec point(pointPosMap[p->corrPoint]);

				// Draw the point
				glVertex3fv(point);
			}
		}

		glEnd();
	}
}

void GridVisualizer::drawSquareAverage()
{
	if(grid->isReady)
	{
		glBegin(GL_QUADS);

		Vector<GridSquare> * squares = grid->getSquares( selectedLevel );

		double heightScale = grid->getHeightRange( 0 );

		// Draw squares
		for(Vector<GridSquare>::iterator square = squares->begin(); square != squares->end(); square++)
		{
			int x = square->v;
			int y = square->u;

			// Set square color value
			double value = (squareValue(y,x) - grid->min_height[selectedLevel]) / heightScale;
			value = Max(0, Min(1, value));

			if(grid->activeGridSquare == &(*square))
				glColor3f(value/2, value/2, 1);
			else
				glColor3f(value, value, value);

			// Scrolling
			y = (y + scroll_y) % y_count;
			if(y < 0) y += y_count;

			// draw grid square
			glVertex3f(x * squareWidth		, y * squareHeight, 0);
			glVertex3f((x+1) * squareWidth	, y * squareHeight, 0);
			glVertex3f((x+1) * squareWidth	, (y+1) * squareHeight, 0);
			glVertex3f(x * squareWidth		, (y+1) * squareHeight, 0);
		}

		glEnd();
	}
}

void GridVisualizer::drawPatch(bool isFilled)
{
	if(grid->isReady)
	{
		Mesh * m = grid->getBaseMesh();

		glPushAttrib(GL_ALL_ATTRIB_BITS);

		if(!isFilled)	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glColor4f(0.8f, 0, 0,  0.6f);
		glLineWidth(1.0f);
		
		glBegin(GL_LINES);

		foreach(int fi, showFaces.set)
		{
			Face * f = m->f(fi);

			int v0 = f->vIndex[0];
			int v1 = f->vIndex[1];
			int v2 = f->vIndex[2];

			HashMap<int, Vec>::iterator end = pointPosMap.end();

			if(pointPosMap.find(v0) != end && pointPosMap.find(v1) != end && pointPosMap.find(v2) != end)
			{
				Vec p0 = pointPosMap[f->vIndex[0]];
				Vec p1 = pointPosMap[f->vIndex[1]];
                                Vec p2 = pointPosMap[f->vIndex[2]];
			}
		}

		glEnd();

		glPopAttrib();
	}
}

int GridVisualizer::drawEdge( Vec p0, Vec p1, double threshold )
{
	double diff = p0.y - p1.y;

	if(abs(diff) < threshold) // Regular case
	{
		glVertex3f(p0.x, p0.y, 0);
		glVertex3f(p1.x, p1.y, 0);
		return 0;
	}
	else
	{	
		double fullHeight = threshold * 2.0f;

		if(diff > 0)
			p0.y -= fullHeight;
		else
			p1.y -= fullHeight;	

		// Top
		glVertex3fv(p0);	
		glVertex3fv(p1);

		p0.y += fullHeight;
		p1.y += fullHeight;

		// Bottom
		glVertex3fv(p0);	
		glVertex3fv(p1);
	}

	return 1; // edge belongs to loop triangle
}

void GridVisualizer::bakePatch()
{
	this->makeCurrent();

	glFlush();

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	this->qglClearColor(QColor(0,0,0,0));

	glDisable(GL_LIGHTING);

	int old_width = width();
	int old_height = height();

	int old_squareWidth = squareWidth;
	int old_squareHeight = squareHeight;

	int old_scroll_y = scroll_y;

	squareWidth = x_count;
	squareHeight =  y_count;

	this->resize(x_count, y_count);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawPatch(true);

	QImage img = grabFrameBuffer();

	glPopAttrib();

	// Reset settings
	squareWidth = old_squareWidth;
	squareHeight = old_squareHeight;
	this->resize(old_width, old_height);
	scroll_y = old_scroll_y;

	this->doneCurrent();

	patchMask = matrixFromImage(&img);;
}

void GridVisualizer::mouseReleaseEvent(QMouseEvent * e)
{
	// Switch draw modes by right mouse click
	if(e->button() == Qt::RightButton)
	{
		if(Mode == SQUARE_AVERAGE)
			Mode = GRID_POINTS;
		else
			Mode = SQUARE_AVERAGE;
	}

	e->ignore();

	this->update();
}

void GridVisualizer::mousePressEvent(QMouseEvent * e)
{
	if(e->button() == Qt::LeftButton)
	{
		Vector<GridSquare> * squares = grid->getSquares( selectedLevel );

		int x_count = grid->lengthCount;
		int y_count = grid->widthCount;

		int squareWidth = width() / x_count;
		int squareHeight = height() / y_count;

		// Assume nothing is selected by default
		this->grid->activeGridSquare = NULL;
		this->grid->activeGridPoint = NULL;

		for(Vector<GridSquare>::iterator square = squares->begin(); square != squares->end(); square++)
		{
			int x = square->v;
			int y = square->u;

			Rect r;
			r.l = x * squareWidth;
			r.r = r.l + squareWidth;
			r.t = y * squareHeight;
			r.b = r.t + squareHeight;

			if(isInRectangle(e->x(), e->y(), r))
			{
				// Select grid square
				grid->activeGridSquare = &(*square);

				// debug
				printf("GridSquare (u = %d, v = %d), Points Count (%d), Height (%f)\n", square->u, 
					square->v, square->points.size(), square->height);

				if(Mode == GRID_POINTS)
				{
					Vector<GridPoint> * points = &square->points;

					// Find exact point
					for(Vector<GridPoint>::iterator p = points->begin(); p != points->end(); p++)
					{
						int h = pointSize / 2.0f;

						Vec ip = PointFromSquare(*p, squareHeight, squareWidth);
						ip.x += (x * squareWidth);
						ip.y += (y * squareHeight);

						r.l = ip.x - h;
						r.r = ip.x + h;
						r.t = ip.y - h;
						r.b = ip.y + h;
						
						if(isInRectangle(e->x(), e->y(), r))
						{
							grid->activeGridPoint = &(*p);
							break;
						}
					}
				}

				break;
			}
		}
	}

	this->update();

	e->ignore();
}

bool GridVisualizer::isInRectangle(int x, int y, Rect & r)
{
	if(x >= r.l && x <= r.r && y >= r.t && y <= r.b) 
		return true;
	else
		return false;
}

void GridVisualizer::wheelEvent(QWheelEvent * e)
{
	int numDegrees = e->delta() / 8;
	int numSteps = numDegrees / 15;

	scroll_y = scroll_y + numSteps;

	computePoints();

	this->update();
}
