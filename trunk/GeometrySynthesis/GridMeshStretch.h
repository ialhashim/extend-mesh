#pragma once

#include "GridMesh.h"

class GridMeshStretch: public GridMesh{

private:
	

public:
	GridMeshStretch(Grid * src_grid, const Rect & cropArea,
		Spline * curve, std::vector<std::vector<Point> > & synthOutput,
		bool isSynthesizeCS,  bool isBlendCrossSections, int bandSize, bool isChangeProfile, 
		bool isFillSeams, bool isBlendSeams, bool isSampleSeams);
};
