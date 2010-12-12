#ifndef HALFEDGE_H
#define HALFEDGE_H

#include "Face.h"
#include "Edge.h"

class HalfEdge
{
public:
	Face* pFace[2];
	Edge edge;

	HalfEdge()
	{
		pFace[0] = pFace[1] = NULL;
	}

	HalfEdge(Edge & e, Face * f1, Face * f2)
	{
		edge.vIndex[0] = e.vIndex[0];
		edge.vIndex[1] = e.vIndex[1];

		pFace[0] = f1;
		pFace[1] = f2;
	}

	HalfEdge(const HalfEdge& other)
	{
		this->pFace[0] = other.pFace[0];
		this->pFace[1] = other.pFace[1];

		this->edge = other.edge;
	}

        inline Face * face(int i)	const { return pFace[i]; }
        inline int vertex(int i)	const { return edge.vIndex[i]; }

        bool isBorder() const
	{
		if((pFace[0] == NULL && pFace[1] != NULL) || (pFace[1] == NULL && pFace[0] != NULL))
			return true;
		else 
			return false;
	}

        bool isNull() const
	{
                return (pFace[0] == NULL) && (pFace[1] == 0);
	}

	int fIndex(int i)
	{
		return pFace[i]->index;
	}

	Face * borderFace()
	{
		if(pFace[0] == NULL)
			return pFace[0];
		else
			return pFace[1];
	}
};

struct CompareHalfEdge{
	bool operator()(const HalfEdge &a, const HalfEdge &b) 
	{
		if (a.edge.vIndex[0] < b.edge.vIndex[0]) return true;
		if (a.edge.vIndex[0] > b.edge.vIndex[0]) return false;
		return a.edge.vIndex[1] < b.edge.vIndex[1];
	}
};

typedef StdSet<HalfEdge,CompareHalfEdge> HalfEdgeSet;

#endif // HALFEDGE_H
