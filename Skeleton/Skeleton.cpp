#define NOMINMAX // So Eigen library compiles under windows

#include "Skeleton.h"

Skeleton::Skeleton()
{
	isReady = false;
	isVisible = false;
	embedMesh = NULL;

	colors = Vector<Color3>();
	selectedEdges = Vector<SkeletonEdge*>();

	originalStart = originalEnd = -1;
}

Skeleton::~Skeleton()
{

}

void Skeleton::loadFromFile(const char* fileName)
{
	isReady = false;
	isVisible = false;
	embedMesh = NULL;

	printf("\n\n==================\n");
	printf("Loading Skeleton file...(%s)\n", fileName);

	std::string inputLine;
	FileStream file (fileName);

	float x,y,z;
	int n1, n2;

	int degree;
	int numberOfNodes;
	int numberOfEdges;

	int nCount = 0;
	int nEdges = 0;

	nodes.clear();
	edges.clear();
	lastSelectedFaces.clear();

	if (file.is_open())
	{
		if(!file.eof() && GetLine (file, inputLine))
		{
			if(sscanf(inputLine.c_str(), "# D:%d NV:%d NE:%d", &degree, &numberOfNodes, &numberOfEdges) != 3)
			{
				printf("Error reading skeleton file (check header).");
				printf("\n%d - %d - %d\n", degree, numberOfNodes, numberOfEdges);
				return;
			}
		}

		while (!file.eof())
		{
			GetLine (file, inputLine);

			switch(inputLine[0])
			{
			case 'v':
				if(sscanf(inputLine.c_str(), "v %f %f %f", &x,&y,&z) == 3)
					nodes.push_back( SkeletonNode(x,y,z, nCount++) );
				break;

			case 'e':
				if(sscanf(inputLine.c_str(), "e %d %d", &n1, &n2) == 2)
					edges.push_back( SkeletonEdge(&nodes[n1 - 1], &nodes[n2 - 1], nEdges++) );
				break;
			}
		}

		file.close();

		printf("\nSkeleton file loaded: \n\n\t Nodes: %d \t Edges: %d\n\n", nCount, nEdges);

		isReady = true;
		isVisible = true;
		isUserFriendly = true;

		// empty selection
		deselectAll();
	}
	else
	{
		printf("\n ERROR: cannot open skeleton file.\n");
	}
}

void Skeleton::loadCorrespondence(const char* fileName)
{
	std::string inputLine;
	FileStream file (fileName);

	printf("Loading correspondence file...");

	int nodeIndex = 0;
	int vIndex = 0;

	corr.clear();

	if (file.is_open())
	{
		while (!file.eof())
		{
			GetLine (file, inputLine);

			if(inputLine.length() > 0 && sscanf(inputLine.c_str(), "%d", &nodeIndex))
			{
				v_corr[vIndex] = nodeIndex - 1;
				corr[v_corr[vIndex]].push_back(vIndex);

				vIndex++;
			}
		}

		file.close();
		printf("done.\n");
	}
	else
	{
		printf("ERROR: cannot open correspondence file.\n");
	}
}

void Skeleton::embed(Mesh * m)
{
	embedMesh = m;

	if(nodes.size() < 2)
		return;

	// Simple check 
	int numCorrVertices = 0;
	for(int i = 0; i < (int)corr.size(); i++)
		numCorrVertices += corr[i].size(); 
	if(numCorrVertices != m->numberOfVertices())
		printf("WARNING: points do not match mesh ( %d = %d ?)", numCorrVertices, m->numberOfVertices());
	// End of check

	printf("Embedding...");

	int n = nodes.size();

	float scale = embedMesh->normalize_scale;

	// Assign new embeded node positions
	for(int i = 0; i < n; i++)
	{
		nodes[i].x *= scale;
		nodes[i].y *= scale;
		nodes[i].z *= scale;
	}

	printf("done.\n\n\n");
}

void Skeleton::calculateEdgesLengths()
{
        double len, minEdgeLength = DBL_MAX;

	for(int i = 0; i < (int)edges.size(); i++)
	{
		len = edges[i].calculateLength();

		if(len < minEdgeLength)
			minEdgeLength = len;

		if(len < Epsilon)
			printf("Bad bone! ");
	}
}

SkeletonGraph Skeleton::getGraph()
{
	SkeletonGraph g;

	for(int i = 0; i < (int)edges.size(); i++)
		g.AddEdge(edges[i].n1->index, edges[i].n2->index, edges[i].length, edges[i].index);

	return g;
}

Vector<int> Skeleton::getSelectedFaces(int start, int end)
{
	StdSet<int> resultSet;

	int numVerticesSelected = 0;

	if(sortedSelectedNodes.size() < 1 && selectNodeStart >= 0) // only one node is selected
	{
		int n = selectNodeStart;

		for(int v = 0; v < (int)corr[n].size(); v++)
		{
			Vector<Face *> ifaces = embedMesh->vd(corr[n][v])->ifaces;

			for(Vector<Face *>::iterator f = ifaces.begin(); f != ifaces.end(); f++)
				resultSet.insert((*f)->index);
		}
	}
	else
	{
		StdSet<int> activeNodes;
		activeNodes.insert(sortedSelectedNodes[start]);
		activeNodes.insert(sortedSelectedNodes[sortedSelectedNodes.size() - Max(1,end)]);

		// Explore branches, help us deal with branchy skeletons
		SkeletonGraph g = getGraph();
		g.explore(sortedSelectedNodes[start + 1], activeNodes);

		foreach(int n, activeNodes)
		{
			numVerticesSelected += corr[n].size();

			for(int v = 0; v < (int)corr[n].size(); v++)
			{
				int vIndex = corr[n][v];

				for(int f = 0; f < (int)embedMesh->vd(vIndex)->ifaces.size(); f++)
				{
					int fIndex = embedMesh->vd(vIndex)->ifaces[f]->index;
					resultSet.insert(fIndex);
				}
			}
		}
	}

	Vector<int> result;

	for(StdSet<int>::iterator it = resultSet.begin(); it != resultSet.end(); it++)
		result.push_back(*it);

	//printf("\nSelected: vertices (%d) => faces (%d)\n", numVerticesSelected, result.size());

	return (lastSelectedFaces = result);
}

Vector<int> Skeleton::getSelectedFaces(bool growSelection)
{
	StdSet<int> faceResultSet;
	int numVerticesSelected = 0;

	SkeletonGraph g = getGraph();

	StdSet<int> activeNodes;
	activeNodes.insert(originalSelectedNodes.front());
	activeNodes.insert(originalSelectedNodes.back());

	if(originalSelectedNodes.size() > 1)
	{
		// Explore branches, help us deal with branchy skeletons
		g.explore(originalSelectedNodes[1], activeNodes);
	}

	// If we need a grown selection, we add the neighbor's nodes at the end points
	if(growSelection)
	{
		StdSet<int> startNeighbours = VECTOR_TO_SET(g.GetNeighbours(originalSelectedNodes.front()));
		foreach(int n, startNeighbours) activeNodes.insert(n);

		StdSet<int> endNeighbours = VECTOR_TO_SET(g.GetNeighbours(originalSelectedNodes.back()));
		foreach(int n, endNeighbours)	activeNodes.insert(n);

		foreach(int i, endNeighbours) 
		{
			Vector<int> NN = g.GetNeighbours(i);
			foreach(int j, NN) activeNodes.insert(j);
		}
	}

	foreach(int n, activeNodes)
	{
		numVerticesSelected += corr[n].size();

		for(int v = 0; v < (int)corr[n].size(); v++)
		{
			int vIndex = corr[n][v];
			for(int f = 0; f < (int)embedMesh->vd(vIndex)->ifaces.size(); f++)
			{
				faceResultSet.insert(embedMesh->vd(vIndex)->ifaces[f]->index);
			}
		}
	}

	// Convert to vector
	Vector<int> result;
	for(StdSet<int>::iterator it = faceResultSet.begin(); it != faceResultSet.end(); it++)
	{
		result.push_back(*it);
	}
	//printf("\nSelected: vertices (%d) => faces (%d)\n", numVerticesSelected, result.size());

	return (lastSelectedFaces = result);
}

void Skeleton::deselectAll()
{
	selectedEdges.clear();
	sortedSelectedNodes.clear();

	smoothNodes.clear();
	smoothEdges.clear();

	lastSelectedFaces.clear();

	selectedNodes = Vector<bool>(nodes.size(), false);
	selectNodeStart = selectNodeEnd = -1; // clear = -1
}

void Skeleton::selectNode(int index)
{
	if((int)selectedNodes.size() > index && index > -1)
		selectedNodes[index] = true;

	if(selectNodeStart == -1)		
	{
		selectNodeStart = index;
		lastSelectedFaces.clear();

		Vector<SkeletonEdge *> startNeighbours = nodeNeighbourEdges(selectNodeStart);
		Vec startPlaneNormal = startNeighbours.front()->direction().unit();
		start_circle = Circle(nodeRadius(selectNodeStart), 40, startPlaneNormal, nodes[selectNodeStart].v());
	}

	if(selectNodeStart != index)	selectNodeEnd = index; 
}

void Skeleton::selectNodeFromFace(int index)
{
	if(embedMesh)
	{
		selectNode(v_corr[embedMesh->f(index)->VIndex(0)]);
	}
}

void Skeleton::selectEdge(int index, int node1_index)
{
	if(index < 0) return;

	// Don't select a selected edge
	for(int i = 0; i < (int)selectedEdges.size(); i++)
		if(selectedEdges[i]->index == index) return;

	selectedEdges.push_back(&edges[index]);

	// order nodes based on selection direction
	if(edges[index].n1->index != node1_index)
	{
		// swap if necessary
		SkeletonNode * temp = edges[index].n1;
		edges[index].n1 = edges[index].n2;
		edges[index].n2 = temp;
	}
}

void Skeleton::selectEdges(int start, int end)
{
	printf("Selection:  Start (%d), End (%d) \n", selectNodeStart, selectNodeEnd);

	if(selectNodeStart != -1 && selectNodeEnd != -1 && selectNodeStart != selectNodeEnd)
	{
		SkeletonGraph g = this->getGraph();

		StdList<int> path = g.DijkstraShortestPath(start, end);

		deselectAll();

		int prevNode = *path.begin();

		for(StdList<int>::iterator it = path.begin(); it != path.end(); it++)
		{
			selectNode(*it);
			selectEdge(getEdge(prevNode, *it), prevNode);

			sortedSelectedNodes.push_back(*it);

			prevNode = *it;
		}

		// If we never crop
		if(originalEnd == -1)
		{
			originalStart = selectNodeStart;
			originalEnd = selectNodeEnd;
		}

		// debug
		//StdList<int>::iterator i = path.begin();	i++;
		//this->testNodes.insert(start);
		//this->testNodes.insert(end);
		//g.explore(*i, this->testNodes);
	}

	this->originalSelectedNodes = sortedSelectedNodes;
	this->originalSelectedEdges = selectedEdges;
}

void Skeleton::smoothSelectedEdges(int numSmoothingIterations)
{
	if(!selectedEdges.size())
		return;

	smoothNodes.clear();
	smoothEdges.clear();

	int nCount = 0;
	int eCount = 0;

	SkeletonNode *n1, *n2, *n3;
	n1 = n2 = n3 = NULL;

	smoothNodes.push_back(new SkeletonNode(selectedEdges[0]->n1, nCount));
	n1 = smoothNodes[nCount];

	for(int i = 0; i < (int)selectedEdges.size(); i++)
	{
		smoothNodes.push_back(SkeletonNode::Midpoint(n1, selectedEdges[i]->n2, nCount + 1));
		smoothNodes.push_back(new SkeletonNode(selectedEdges[i]->n2, nCount + 2));

		n2 = smoothNodes[nCount+1];
		n3 = smoothNodes[nCount+2];

		smoothEdges.push_back(new SkeletonEdge(n1, n2, eCount));	
		smoothEdges.push_back(new SkeletonEdge(n2, n3, eCount+1));	

		smoothEdges[eCount]->calculateLength();
		smoothEdges[eCount+1]->calculateLength();

		nCount += 2;
		eCount += 2;

		n1 = n3;

	}

	// Laplacian smoothing
	for(int stage = 0; stage < numSmoothingIterations; stage++)
	{
		Vector<float*> positions = Vector<float*>(smoothNodes.size());

		for(int i = 1; i < (int)smoothNodes.size() - 1; i++)
		{
			positions[i] = new float[3];

			positions[i][0] = (smoothNodes[i-1]->x + smoothNodes[i+1]->x) / 2;
			positions[i][1] = (smoothNodes[i-1]->y + smoothNodes[i+1]->y) / 2;
			positions[i][2] = (smoothNodes[i-1]->z + smoothNodes[i+1]->z) / 2;
		}

		for(int i = 1; i < (int)smoothNodes.size() - 1; i++)
		{
			smoothNodes[i]->set(positions[i]);
                        delete [] positions[i];
		}
	}

	// clear old list of selected edges
	selectedEdges.clear();

	// fill in the new smooth edges
	selectedEdges = smoothEdges;
}

void Skeleton::cropSelectedEdges(int start, int end)
{
	originalStart = selectNodeStart;
	originalEnd = selectNodeEnd;

	if(sortedSelectedNodes.size() > 5)
	{
		int N = sortedSelectedNodes.size() - 1;
		selectEdges(sortedSelectedNodes[start], sortedSelectedNodes[N - end]);
	}
}		

Vector<Vec> Skeleton::getSelectedSkeletonPoints()
{
	Vector<Vec> skeletonPoints;

	for(int i = 0; i < (int)selectedEdges.size(); i++)
		skeletonPoints.push_back(selectedEdges[i]->n1->v());

	skeletonPoints.push_back((*selectedEdges.rbegin())->n2->v()); // Last point

	return skeletonPoints;
}

int Skeleton::getEdge(int n1, int n2)
{
	int index = -1;

	for(int i = 0; i < (int)edges.size(); i++)
	{
		if((edges[i].n1->index == n1 && edges[i].n2->index == n2) || 
			(edges[i].n1->index == n2 && edges[i].n2->index == n1))
		{
			return edges[i].index;
		}
	}

	return index;
}

Vec Skeleton::selectedEdgesPlane()
{
	if(this->selectedEdges.size() < 3)
		return Vec();
	else
	{
		return selectedEdges[0]->direction() ^ selectedEdges[selectedEdges.size() - 1]->direction();
	}
}

Vector<SkeletonNode *> Skeleton::nodeNeighbours(int node_index)
{
	Vector<SkeletonNode *> neighbours;

	for(int i = 0; i < (int)edges.size(); i++)
	{
		if(edges[i].n1->index == node_index)
			neighbours.push_back(&nodes[edges[i].n2->index]);
		else if(edges[i].n2->index == node_index)
			neighbours.push_back(&nodes[edges[i].n1->index]);
	}

	return neighbours;
}

Vector<SkeletonEdge *> Skeleton::nodeNeighbourEdges(int node_index)
{
	Vector<SkeletonEdge *> neighbours;

	for(int i = 0; i < (int)edges.size(); i++)
	{
		if(edges[i].n1->index == node_index)
			neighbours.push_back(&edges[i]);
		else if(edges[i].n2->index == node_index)
			neighbours.push_back(&edges[i]);
	}

	return neighbours;
}

float Skeleton::nodeRadius(int node_index)
{
	float avgDist = 0;

	Vec node_center = nodes[node_index].v();

	for(int j = 0; j < (int)corr[node_index].size(); j++)
	{
		avgDist += (*embedMesh->v(corr[node_index][j]) - node_center).norm();
	}

	avgDist /= corr[node_index].size();

	return avgDist;
}

Vec Skeleton::centerOfNode(SkeletonNode * n)
{
	Vec center;

	for(int j = 0; j < (int)corr[n->index].size(); j++)
	{
		center += *embedMesh->v(corr[n->index][j]);
	}

	return center / corr[n->index].size();
}

std::pair< Vector<int>, Vector<int> > Skeleton::Split(int edgeIndex)
{
	SkeletonGraph g;

	for(int i = 0; i < (int)edges.size(); i++)
	{
		if(edgeIndex != i)
			g.AddEdge(edges[i].n1->index, edges[i].n2->index, edges[i].length, edges[i].index);
	}

	SkeletonEdge * e = &edges[edgeIndex];

	StdMap<int, int> partA, partB;

	// Find the two connected components
	for(int i = 0; i < (int)nodes.size(); i++)
	{
		if(g.isConnected( e->n1->index, nodes[i].index ))
			partA[i] = i;
		else
			partB[i] = i;
	}

	Vector<int> pointsA, pointsB;


	for(StdMap<int,int>::iterator it = partA.begin(); it != partA.end(); it++)
	{
		int i = (*it).first;
		for(int p = 0; p < (int)corr[i].size(); p++)
			pointsA.push_back(corr[i][p]);
	}

	for(StdMap<int,int>::iterator it = partB.begin(); it != partB.end(); it++)
	{
		int i = (*it).first;
		for(int p = 0; p < (int)corr[i].size(); p++)
			pointsB.push_back(corr[i][p]);
	}

	return std::make_pair(pointsA, pointsB);
}

void Skeleton::drawNodesNames()
{
	for(int i = 0; i < (int)nodes.size(); i++)
	{
		glPushName(i);
		glBegin(GL_POINTS);
		glVertex3f(nodes[i].x, nodes[i].y, nodes[i].z);
		glEnd();
		glPopName();
	}
}

void Skeleton::drawMeshFacesNames()
{
	if(embedMesh)
	{
		StdList<Face> * faces = embedMesh->facesList();

		for(StdList<Face>::iterator it = faces->begin(); it != faces->end(); it++)
		{
			glPushName(it->index);
			glBegin(GL_TRIANGLES);
			glVertex3fv(*it->P(0));
			glVertex3fv(*it->P(1));
			glVertex3fv(*it->P(2));
			glEnd();
			glPopName();
		}
	}
}

void Skeleton::drawUserFriendly()
{
	glDisable(GL_LIGHTING);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Smooth lines
	glEnable (GL_LINE_SMOOTH);
	glEnable (GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);

	if(selectNodeStart >= 0)
	{
		// Draw start
		if(selectNodeEnd < 0)
		{
			start_circle.draw(4, Color4 (0,128,255));
		}

		glEnable(GL_LIGHTING);
		glEnable(GL_CULL_FACE);

		glColor4f(0.0, 0.5, 1, 0.75f);

		glBegin(GL_TRIANGLES);
		for(Vector<int>::iterator it = lastSelectedFaces.begin(); it != lastSelectedFaces.end(); it++)
		{
			for(int j = 0; j < 3; j++)
			{
				int vi = embedMesh->f(*it)->VIndex(j);
				glNormal3fv(*embedMesh->n(vi));
				glVertex3fv(*embedMesh->v(vi));
			}
		}
		glEnd();

		glDisable(GL_CULL_FACE);
	}

	glEnable(GL_LIGHTING);
}

void Skeleton::draw(bool drawMeshPoints)
{
	if(!isVisible || !isReady)
		return;

	if(isUserFriendly)
	{
		drawUserFriendly();
		return;
	}

	// DEBUG:
	/*glPointSize(12.0f);
	glBegin(GL_POINTS);
	foreach(int ni, testNodes)
	{
	glColor3f(0,1,0);
	glVertex3f(nodes[ni].x, nodes[ni].y, nodes[ni].z);
	}
	glEnd();*/

	// Demo COLORS - Generate Random Colors
	/*if(colors.size() < 1){
	for (int i = 0; i < 100; i++){
	float r = ((rand() % 255) + 30) / 255.0f;
	float g = ((rand() % 250) + 25) / 255.0f;
	float b = ((rand() % 250) + 20) / 255.0f;
	colors.push_back(Color3(r, g, b));
	}
	}*/

	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);

	// Draw the nodes
	glPointSize(4.0f);
	glColor3f(1,0,0);
	glEnable(GL_POINT_SMOOTH);

	//======================
	// Draw Skeleton Edges

	float oldLineWidth = 0;
	glGetFloatv(GL_LINE_WIDTH, &oldLineWidth);
	glLineWidth(1.5f);

	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_LINES);
	for(int i = 0; i < (int)edges.size(); i++)
	{
		int nIndex1 = edges[i].n1->index;
		int nIndex2 = edges[i].n2->index;

		glColor4f(1,1,1,0.2f);

		bool isEdgeSelected = false;
		if(selectedNodes[nIndex1] && selectedNodes[nIndex2])
			isEdgeSelected = true;

		if(isEdgeSelected)	glColor3f(0.5f, 0.0f, 0.2f);
		glVertex3f(edges[i].n1->x, edges[i].n1->y, edges[i].n1->z);

		if(isEdgeSelected)	glColor3f(1.0f, 0.2f, 0.2f);
		glVertex3f(edges[i].n2->x, edges[i].n2->y, edges[i].n2->z);
	}
	glEnd();

	glDisable(GL_BLEND);

	//======================
	// Draw Skeleton Nodes
	for(int i = 0; i < (int)corr.size(); i++)
	{
		//glColor4f(colors[i % colors.size()].r, colors[i % colors.size()].g, colors[i % colors.size()].b, 0.5f);

		if(selectNodeStart == i)		glColor3f(0.4f, 0.0f, 0.5f);
		else if(selectNodeEnd == i)		glColor3f(0.8f, 0.3f, 0.95f);
		else if(selectedNodes[i])		glColor3f(0.9f, 0.2f, 0.2f);
		else							glColor3f(0.7f, 0.7f, 0.7f);

		if(drawMeshPoints)
		{
			glEnable(GL_BLEND); 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glPointSize(3.0f);

			glBegin(GL_POINTS);
			for(int j = 0; j < (int)corr[i].size(); j++)
			{
				Vertex * v = embedMesh->v(corr[i][j]);
				glVertex3fv(*v);
			}
			glEnd();

			glDisable(GL_BLEND); 
		}

		glPointSize(7.0f);
		glBegin(GL_POINTS);
		glVertex3f(nodes[i].x, nodes[i].y, nodes[i].z);
		glEnd();

		// White Border
		glPointSize(10.0f);
		glColor3f(1,1,1);

		glBegin(GL_POINTS);
		glVertex3f(nodes[i].x, nodes[i].y, nodes[i].z);
		glEnd();
	}

	// Draw smooth skeletons if any
	glClear(GL_DEPTH_BUFFER_BIT);
	for(int i = 0; i < (int)smoothEdges.size(); i++)
	{
		glLineWidth(2.5f);
		glColor3f(0,0.6f,0);

		SkeletonNode * n1 = smoothEdges[i]->n1;
		SkeletonNode * n2 = smoothEdges[i]->n2;

		glBegin(GL_LINES);
		glVertex3f(n1->x, n1->y, n1->z);
		glVertex3f(n2->x, n2->y, n2->z);
		glEnd();

		glPointSize(8.0f);
		glBegin(GL_POINTS);
		glVertex3f(n1->x, n1->y, n1->z);
		glVertex3f(n2->x, n2->y, n2->z);
		glEnd();

		glPointSize(12.0f);
		glColor3f(0.8f, 0.9f, 0.8f);
		glBegin(GL_POINTS);
		glVertex3f(n1->x, n1->y, n1->z);
		glVertex3f(n2->x, n2->y, n2->z);
		glEnd();
	}

	glLineWidth(oldLineWidth);
	glColor3f(1,1,1);

	glEnable(GL_LIGHTING);
}

void Skeleton::sampleProjectPoints(float ratio, Mesh * m, Vector<int> & points, HistogramFloat * histogram)
{
	float maxHeight = FLT_MIN;
	float minHeight = FLT_MAX;

	StdMap<int, float> T;
	StdMap<int, SkeletonEdge*> ReferenceEdge;
	StdMap<int, float> Height;

	int stepSize = (((1 - ratio) * points.size()) / 4) + 1;

	histogram->begin(points.size());

	Vec P, P0, V, L, N1, N2;
	float t, distance;
	int pIndex;

	Vector<int> pointsIndex;
	for(Vector<int>::iterator it = points.begin(); it != points.end(); it++)
		pointsIndex.push_back(*it);

	for(int i = 0; i < (int)pointsIndex.size(); i += stepSize)
	{
		pIndex = pointsIndex[i];
		Height[pIndex] = FLT_MAX;

		// Project on the skeleton edges
		for(int i = 0; i < (int)selectedEdges.size(); i++)
		{
			SkeletonEdge * e = selectedEdges[i];
			N1 = *e->n1;
			N2 = *e->n2;
			L = N2 - N1;

			P = *m->v(pIndex);
			V = P - N1;
			t =  V * (L / L.norm()) / L.norm();
			P0 = (t * L) + N1;
			distance = (P - P0).norm();

			// Regular: point projects on skeleton edge
			if(distance < Height[pIndex] && RANGE(t, 0.0f, 1.0f)){
				Height[pIndex] = distance;
				T[pIndex] = t;
				ReferenceEdge[pIndex] = e;
				if(distance > maxHeight)	maxHeight = distance;
				if(distance < minHeight)	minHeight = distance;
				histogram->insert(distance);
			}

			// Special case : the point projects directly on skeleton nodes
			float d1 = (P - N1).norm();
			float d2 = (P - N2).norm();
			if(d1 < Height[pIndex]){
				Height[pIndex] = d1;
				T[pIndex] = 0.0f;
				ReferenceEdge[pIndex] = e;
			}
			if(d2 < Height[pIndex]){
				Height[pIndex] = d2;
				T[pIndex] = 1.0f;
				ReferenceEdge[pIndex] = e;
			}
		}
	}

	histogram->end();
}
