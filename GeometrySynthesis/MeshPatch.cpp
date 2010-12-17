#include "MeshPatch.h"

#include "SimpleDraw.h"

#include "kdtree.h"

MeshPatch::MeshPatch(int numVerts, int ID)
{
	mesh = Mesh(numVerts);
	id = ID;
	falseColor = Color4::random();
}

void MeshPatch::insertPoint(int corrPointIndex, const Vec& pos)
{
	int vindex = vertex.size();

	corrPoint[vindex] = corrPointIndex;

	vertex[corrPointIndex] = vindex;

	cloud[vindex] = pos;
}

void MeshPatch::insertFace(int cp1, int cp2, int cp3)
{
	int vIndex1 = vertex[cp1];
	int vIndex2 = vertex[cp2];
	int vIndex3 = vertex[cp3];

	usedPoints.insert(vIndex1);
	usedPoints.insert(vIndex2);
	usedPoints.insert(vIndex3);

	triangles.push_back(SimpleTriangle(vIndex1, vIndex2, vIndex3, triangles.size()));
}

void MeshPatch::insertNonTriangulated(int corrPointIndex)
{
	nonTriangulated.insert(vertex[corrPointIndex]);
}

Vector<int> MeshPatch::getBorder()
{
	Vector<int> result;

	VertexMap::iterator it, theEnd = usedPointsMap.end();

	foreach(int index, nonTriangulated)
	{
		it = usedPointsMap.find(index);

		if(it != theEnd)
			result.push_back(it->second);
	}

	return result;
}

int MeshPatch::convertToCorr(int index)
{
	return usedIndexToCorr[index];
}

StdSet<int> MeshPatch::convertToCorr(const StdList<int>& indices)
{
	StdSet<int> result;

	for(StdList<int>::const_iterator it = indices.begin(); it != indices.end(); it++)
	{
		result.insert(usedIndexToCorr[*it]);
	}

	return result;
}

void MeshPatch::makeMesh()
{
	mesh.isReady = false;
	mesh.isVisible = false;

	int oldIndex, newIndex;

	// Add the vertices
	for(StdSet<int>::iterator it = usedPoints.begin(); it != usedPoints.end(); it++)
	{
		oldIndex = *it;

		newIndex = mesh.numberOfVertices();		// new index

		usedPointsMap[oldIndex] = newIndex;

		usedIndexToCorr[newIndex] = corrPoint[oldIndex];

		// Add to mesh structure
		mesh.addVertex(cloud[oldIndex], newIndex);
	}

	// Add the faces
	foreach(SimpleTriangle t, triangles)
	{
		mesh.addFace(usedPointsMap[t.v1], usedPointsMap[t.v2], usedPointsMap[t.v3], t.index);
	}
}

void MeshPatch::replaceMesh( Mesh * from, bool isReplaceCorr )
{
	KDTree points(3);
	Vector<int> border;

	if(isReplaceCorr)
	{
		border = getBorder(); // border in terms of usedPointsMap

		// Fill in border points
		foreach(int vi, border)
		{
			Vec p = mesh.ver(vi);
			points.insert3f(p.x, p.y, p.z, usedIndexToCorr[vi]);
		}
	}

	mesh = *from;

	// Replace corresponding points
	if(isReplaceCorr)
	{
		// get new border
		border = mesh.getBorderVertices();

		foreach(int vi, border)
		{
			Vec p = mesh.ver(vi);
			kdnode * node = points.get_nearest(p.x, p.y, p.z);

			// overwrite old results
			usedIndexToCorr[vi] = node->index;
		}
	}
}

void MeshPatch::drawPointCloud()
{
	// Draw non-triangulated points
	/*for(StdMap<int, Vec>::iterator it = nonTriangulated.begin(); it != nonTriangulated.end(); it++)
	{
		SimpleDraw::IdentifyPoint(it->second);
	}*/
}

/*Vector<Vector<Vec> > MeshPatch::readyToDrawTris()
{
	Vector<Vector<Vec> > trisToDraw;

	foreach(Triangle t, paramterTriangles)
	{
		Vector<Vec> face;

		face.push_back(t.p[0]);
		face.push_back(t.p[1]);
		face.push_back(t.p[2]);

		trisToDraw.push_back(face);
	}

	return trisToDraw;
}*/

StdList<BaseTriangle*> MeshPatch::parameterTriPointers()
{
    StdList<BaseTriangle*> listTris;

    for(int i = 0; i < (int)parameterTriangles.size(); i++)
    {
        listTris.push_back(&parameterTriangles[i]);
    }

    return listTris;
}

