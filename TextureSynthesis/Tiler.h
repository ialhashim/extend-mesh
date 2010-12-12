#pragma once

#include "Globals.h"
#include "Graph.h"

class Tiler
{
private:
	MatrixXf src_img, target_img;
	int band_size;
	int tile_count;
	bool is_done;

	// Stitching function
	MatrixXi stitchAsPos(const MatrixXi & left_img, const MatrixXi & right_img, const MatrixXi & mask);

	// Mask computations
	MatrixXi cut_mask(const MatrixXf & costMatrix);
	void fillMask(int x, int y, MatrixXi & m);

public:
	Tiler(const MatrixXf & src, int bandSize, int tileCount);

	// Tiling function
	MatrixXi tileAsPos();

	bool isDone();
	MatrixXf getTarget();
	MatrixXf getSource();
};
