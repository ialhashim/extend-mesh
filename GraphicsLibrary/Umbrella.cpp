#include "Umbrella.h"

Umbrella::Umbrella(VertexDetail * vd)
{
	index = vd->index;
	flag = vd->flag;
	ifaces = vd->ifaces;
	
	loadHalfEdgeSet();
}

void Umbrella::loadHalfEdgeSet()
{
	halfEdge.clear();

	Face *f, *g;

        for(int i = 0; i < (int)ifaces.size(); i++)
	{
		f = ifaces[i];
		EdgeSet edges = f->edgesWithVertex(index);

		for(EdgeSet::iterator it = edges.begin(); it != edges.end(); it++)
		{
			int otherVertex = it->neighbor();
			Edge currEdge (index, otherVertex);

			g = NULL;

                        for(int j = 0; j < (int)ifaces.size(); j++)
			{
				if(i != j && ifaces[j]->hasEdge(currEdge))
				{
					g = ifaces[j];
					break;
				}
			}

			halfEdge.insert( HalfEdge ( currEdge, f, g ) );
		}
	}

	neighbor.clear();
	neighbor.reserve(halfEdge.size());

	for(HalfEdgeSet::iterator h =  halfEdge.begin(); h != halfEdge.end(); h++)
		neighbor.push_back( (*h).edge.neighbor() );
}

PairInt Umbrella::borderNeighbours()
{
	int result[] = {-1, -1};

	int c = 0;

	for(HalfEdgeSet::iterator h = halfEdge.begin(); h != halfEdge.end(); h++)
	{
		if(h->isBorder())
		{
			result[c++] = h->edge.neighbor();

			if(c > 1)	break;
		}
	}

	if(result[1] == -1) // Non-manifold !!
		result[1] = result[0];

	return PairInt(result[0], result[1]);
}

Pair<Face *, Face *> Umbrella::sharedFaces(Umbrella * other)
{
	for(HalfEdgeSet::iterator h = halfEdge.begin(); h != halfEdge.end(); h++)
	{
		if(h->edge.neighbor() == other->index)
			return Pair<Face *, Face *>(h->face(0), h->face(1));
	}

        Face * nullFace = 0;

        return std::pair<Face *, Face *> (nullFace, nullFace);
}

HalfEdge Umbrella::sharedEdge(Umbrella * other)
{
	for(HalfEdgeSet::iterator h = halfEdge.begin(); h != halfEdge.end(); h++)
	{
		if(h->edge.neighbor() == other->index)
			return HalfEdge(*h);
	}

	return HalfEdge();
}

bool Umbrella::hasNeighbour(int index)
{
        for(int i = 0; i < (int)neighbor.size(); i++)
	{
		if(neighbor[i] == index)
			return true;
	}

	return false;
}

bool Umbrella::hasNeighbour(Umbrella * other)
{
	return hasNeighbour(other->index);
}

bool Umbrella::shareTriangle(Umbrella * b, Umbrella * c)
{
	HalfEdge h1 = sharedEdge(b);
	HalfEdge h2 = sharedEdge(c);

	if(h1.isNull() || h2.isNull())
	{
		return false;
	}
	else
	{
		return !(h1.isBorder() && h2.isBorder());
	}
}

int Umbrella::valence()
{
	return neighbor.size();
}

StdSet<int> Umbrella::faceNighbours(Face * face)
{
	StdSet<int> nighbours;

	for(HalfEdgeSet::iterator h = halfEdge.begin(); h != halfEdge.end(); h++)
	{
		if(h->pFace[0] == face)	nighbours.insert(h->pFace[1]->index);
		if(h->pFace[1] == face)	nighbours.insert(h->pFace[0]->index);
	}

	return nighbours;
}

Normal Umbrella::normal()
{
	Vec n;
	for(Vector<Face *>::iterator it = ifaces.begin(); it != ifaces.end(); it++)
		n += (*it)->normal();
	n.normalize();
	return n;
}
