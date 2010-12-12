#ifndef SMOOTHER_H
#define SMOOTHER_H

#undef min
#undef max

// Linear Solver ================
#undef Vector

#define COMPLEX std::complex<double>

#include "compcol_double.h"
#include "mvblasd.h"

// Preconditioners
#include "diagpre_double.h"
#include "icpre_double.h"
#include "ilupre_double.h"

// Solvers
#include "bicg.h"
#include "bicgstab.h"
#include "cg.h"
#include "cgs.h"

#define Vector std::vector
// End of Solver ================

#include "Mesh.h"

// Use Eigen to help setup sparse matrix
#undef min
#undef max
#include <Eigen/Core>
#include <Eigen/Sparse>
using namespace Eigen;

class Smoother{
private:
	static void treatBorders(Mesh * mesh, Vector<Umbrella> & U);
	static void untreatBorders(Mesh * mesh, Vector<Umbrella> & U);

public:
	static void LaplacianSmoothing(Mesh * m, int numIteration, bool protectBorders = true);
	static void LaplacianSmoothing(Mesh * m, StdSet<Face*> & faceList, int numIteration, bool protectBorders = true);
	static Vertex LaplacianSmoothVertex(Mesh * m, int vi);

	static void ScaleDependentSmoothing(Mesh * m, int numIteration, float step_size = 0.5f, bool protectBorders = true);
	static Vertex ScaleDependentSmoothVertex(Mesh * m, int vi, float step_size = 0.5f);

	static void MeanCurvatureFlow(Mesh * mesh, double step, int numIteration, bool isVolumePreservation = true);
	static void MeanCurvatureFlowExplicit(Mesh * mesh, double step, int numIteration);
};


#endif // SMOOTHER_H
