#ifndef SKELETON_H
#define SKELETON_H

#include "Mesh.h"
#include "Spline.h"
#include "Circle.h"

#include "SkeletonNode.h"
#include "SkeletonEdge.h"

#include "Utility/Graph.h"

typedef Graph<int, float> SkeletonGraph;

class Skeleton
{

public:
	Skeleton();
	~Skeleton();

	bool isReady;
	bool isVisible;
	bool isUserFriendly;

	// STRUCTURE
	Vector<SkeletonNode> nodes;
	Vector<SkeletonEdge> edges;
	StdMap<int, Vector<int> > corr;		// Node -> vertex indices
	StdMap<int, int> v_corr;			// vertex index -> Node

	// DATA LOADING
	void loadFromFile(const char* fileName);
	void loadCorrespondence(const char* fileName);

	// COMPUTATIONS
	void calculateEdgesLengths();
	SkeletonGraph getGraph();

	// SELECTION
	Vector<SkeletonEdge *> originalSelectedEdges;
	Vector<SkeletonEdge *> selectedEdges;
	Vector<bool> selectedNodes;
	Vector<int> sortedSelectedNodes;
	Vector<int> originalSelectedNodes;
	int selectNodeStart;
	int selectNodeEnd;
	int originalStart;
	int originalEnd;
	float minEdgeLength;

	void selectNode(int index);
	void selectNodeFromFace(int index);
	void selectEdge(int index, int node1_index);
	void selectEdges(int start, int end);
	void deselectAll();

	Vec selectedEdgesPlane();

	// NODES FUNCTIONS
	Vector<SkeletonNode *> nodeNeighbours(int node_index);
	Vector<SkeletonEdge *> nodeNeighbourEdges(int node_index);
	float nodeRadius(int node_index);
	Vec centerOfNode(SkeletonNode * n);

	// EDGE FUNCTIONS
	int getEdge(int n1, int n2);

	void sampleProjectPoints(float ratio, Mesh * m, Vector<int> & points, HistogramFloat * histogram);

	// GET POINTS
	Vector<Vec> getSelectedSkeletonPoints();

	// FACE SELECTION
	Vector<int> getSelectedFaces(int start = 0, int end = 0);
	Vector<int> getSelectedFaces(bool growSelection);
	Vector<int> lastSelectedFaces;

	// SMOOTH EDGES
	Vector<SkeletonEdge*> smoothEdges;
	Vector<SkeletonNode*> smoothNodes;
	void smoothSelectedEdges(int numSmoothingIterations = 3);
	void cropSelectedEdges(int start = 1, int end = 1);

	// MODIFY OPERATIONS
	std::pair< Vector<int>, Vector<int> > Split(int edgeIndex);

	// EMBEDDING
	Mesh * embedMesh;
	void embed(Mesh * m);

	// RENDERING FOR SELECTION
	void drawNodesNames();
	void drawMeshFacesNames();

	// VISUALIZATION
	Circle start_circle;
	Vector<Color3> colors;

	void draw(bool drawMeshPoints = true);
	void drawUserFriendly();
	// DEBUG:
	StdSet<int> testNodes;
};

#endif // SKELETON_H
