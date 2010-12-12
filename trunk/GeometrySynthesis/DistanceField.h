#pragma once

/* based on http://www.codersnotes.com/notes/signed-distance-fields */

#include "Matrixf.h"
#include "SimpleSquare.h"

typedef Vector< Vector<Point> > GridPoints;

class DistanceField{

private:
	void GenerateSDF( GridPoints & grid );
	void ScaleGrid( GridPoints & grid, float s );

	int HEIGHT;
	int WIDTH;

	int sx, sy;
	int ENDX, ENDY;

	int fullWidth, fullHeight;

public:
	DistanceField(){HEIGHT = WIDTH = -1;};

	DistanceField(float scale, int offsetX, int offsetY, 
		const Vector<Vector<SimpleSquare> > & patch, 
		int totalWidth, int totalHeight);

	MatrixXf blendMap;

	float scaleFactor;

	double blendAt(int u, int v,  const Vector<double>& w );
};
