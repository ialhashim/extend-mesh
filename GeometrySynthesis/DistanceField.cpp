#include "DistanceField.h"

Point Inside = Point(0, 0);
Point Empty = Point(9999, 9999);

#define Get(g,x,y) (( x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT ) ? g[y][x] : Empty)
#define Put(g,x,y,p) g[y][x] = p
#define Compare(g,p,x,y,offsetx,offsety) \
	other = Get( g, x+offsetx, y+offsety ); \
	other.x += offsetx;other.y += offsety; \
	if (other.DistSq() < p.DistSq()) p = other;

void DistanceField::GenerateSDF( GridPoints &g )
{
	Point other, p;
	int y,x;

	// Pass 0
	for (y=sy;y<ENDY;y++)
	{
		for (x=sx;x<ENDX;x++)
		{
			p = Get( g, x, y );
			Compare( g, p, x, y, -1,  0 );
			Compare( g, p, x, y,  0, -1 );
			Compare( g, p, x, y, -1, -1 );
			Compare( g, p, x, y,  1, -1 );
			Put( g, x, y, p );
		}

		for (x=ENDX-1;x>=0;x--)
		{
			p = Get( g, x, y );
			Compare( g, p, x, y, 1, 0 );
			Put( g, x, y, p );
		}
	}

	// Pass 1
	for (y=ENDY-1;y>=0;y--)
	{
		for (x=ENDX-1;x>=0;x--)
		{
			p = Get( g, x, y );
			Compare( g, p, x, y,  1,  0 );
			Compare( g, p, x, y,  0,  1 );
			Compare( g, p, x, y, -1,  1 );
			Compare( g, p, x, y,  1,  1 );
			Put( g, x, y, p );
		}

		for (x=sx;x<ENDX;x++)
		{
			p = Get( g, x, y );
			Compare( g, p, x, y, -1, 0 );
			Put( g, x, y, p );
		}
	}
}

int debugNum = 0;

DistanceField::DistanceField(float scale, int offsetX, int offsetY, 
							 const Vector<Vector<SimpleSquare> > & patch, 
							 int totalWidth, int totalHeight)
{
	// Scale
	scaleFactor = scale;

	WIDTH = fullWidth = totalWidth;
	HEIGHT = fullHeight = totalHeight;

	// Initialize empty grids and patch mask
	GridPoints grid1 = Vector<Vector<Point> >(HEIGHT, Vector<Point>(WIDTH, Empty));
	GridPoints grid2 = Vector<Vector<Point> >(HEIGHT, Vector<Point>(WIDTH, Inside));

	MatrixXf bitmap = MatrixXf::Zero(HEIGHT, WIDTH);

	// Fill grid
	for(int y = 0; y < (int)patch.size(); y++)
	{
		for(int x = 0; x < (int)patch[0].size(); x++)
		{
			if(patch[y][x].u >= 0)
			{
				int u = (y + offsetY + HEIGHT) % HEIGHT; // loop
				int v = Max(0, x + offsetX);

				grid1[u][v] = Inside;
				grid2[u][v] = Empty;

				bitmap(u,v) = 1;
			}
		}
	}

	ScaleGrid(grid1, scaleFactor);
	ScaleGrid(grid2, scaleFactor);

	WIDTH *= scaleFactor;
	HEIGHT *= scaleFactor;

	// Optimize : only work on patch region
	sx = Max(0, (offsetX * scaleFactor) - scaleFactor);
	sy = Max(0, (offsetY * scaleFactor) - scaleFactor);
	ENDX = Min(WIDTH, (2 * scaleFactor) + sx + (patch[0].size() * (scaleFactor)));
	ENDY = Min(HEIGHT, (2 * scaleFactor) + sy + (patch.size() * (scaleFactor)));

	GenerateSDF(grid1);
	GenerateSDF(grid2);		// inside?

	blendMap = MatrixXf::Constant(HEIGHT, WIDTH, 0.0);

	bitmap = Matrixf::resizeBasic(bitmap, WIDTH, HEIGHT);

	double dist = 0.0;

	for( int y = sy ; y < ENDY ; y++ )
	{
		for ( int x = sx ; x < ENDX ; x++ )
		{
			// Calculate the actual distance from the dx/dy
			if(bitmap(y,x))
				dist = Max(1.0 - sqrt(grid2[y][x].DistSq()), (double)(-scaleFactor));
			else
				dist = Min( sqrt(grid1[y][x].DistSq()), (double)scaleFactor);

			blendMap(y,x) = 1.0 - (dist / scaleFactor);
		}
	}

	blendMap /= blendMap.maxCoeff();

	//Matrixf::pixmapFromMatrix(blendMap.cast<float>()).save(QString().sprintf("DistMap(%d).png",debugNum++));/**/
}

void DistanceField::ScaleGrid( GridPoints & grid, float s )
{
	int newWidth = WIDTH * s;
	int newHeight = HEIGHT * s;

	Vector<Vector<Point> > result = Vector<Vector<Point> >(newHeight, Vector<Point>(newWidth));

	double wMult = (double)WIDTH / newWidth;
	double hMult = (double)HEIGHT / newHeight;

	int max_w = WIDTH - 1;
	int max_h = HEIGHT - 1;

	int iX, iY;

	for(int y = 0; y < newHeight; y++)
	{
		for(int x = 0; x < newWidth; x++)
		{
			iX = (int)(wMult * x);
			iY = (int)(hMult * y);

			result[y][x] = grid[Min(iY, max_h)][Min(iX, max_w)];
		}
	}

	// Scale
	grid = result;
}

double DistanceField::blendAt( int u, int v, const Vector<double>& w )
{
	u *= scaleFactor;
	v *= scaleFactor;

	Vector<Point> point(4, Point(v,u));

	point[1].y += scaleFactor;
	point[2].add(scaleFactor, scaleFactor);
	point[3].x += scaleFactor;

	Point p;

	for(int i = 0; i < 4; i++)
	{
		p.x += (w[i] * point[i].x);
		p.y += (w[i] * point[i].y);
	}

	return blendMap(p.y, p.x);
}
