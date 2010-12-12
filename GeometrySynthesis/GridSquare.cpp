#include "GridSquare.h"

#include "Vertex.h"

Vector<Vector<double> > GridSquare::uniformSample(int size)
{
	Vector<Vector<double> > samples;

	float step = 1.0f / size;

	Vector<Vec> q;

	q.push_back( Vec(0,0,0) );
	q.push_back( Vec(0,1,0) );
	q.push_back( Vec(1,1,0) );
	q.push_back( Vec(1,0,0) );

	int n = 4, prev, next;
	double weightSum;

	for(float v = 0; v < 1.0; v += step)
	{
		for(float u = 0; u < 1.0; u += step)
		{
			std::vector<double> w(4, 0);

			weightSum = 0.0;

			if(u == 0 && v == 0)
				w[0] = 1.0f;
			else
			{
				Vec p (u,v,0);

				// For each corner
				for(int j = 0; j < n; j++)
				{
					prev = (j + n-1) % n;
					next = (j + 1) % n;

					double len = (p - q[j]).norm();

					// Check if point is Epsilon close to a vertex
					if(len > Epsilon)
					{
						double tanAlpha = Vertex::halfAlphaTangent(q[prev], p, q[j]);
						double tanBeta = Vertex::halfAlphaTangent(q[j], p, q[next]);
						
						w[j] = (tanAlpha + tanBeta) / len;

						weightSum += w[j];
					}
					else
					{
						for(int k = 0; k < n; k++){
							if(k != j)
								w[k] = 0.0f;
							else
								w[k] = 1.0f;

							weightSum = 1.0f;

							j = n; // break statement
						}
					}
				}

				// Normalize
				for(int j = 0; j < n; j++)
					w[j] /= weightSum;
			}
			
			samples.push_back(w);
		}
	}

	return samples;
}

GridPoint * GridSquare::getPointCorr( int corrIndex )
{
	for(Vector<GridPoint>::iterator it = points.begin(); it != points.end(); it++)
	{
		if(it->corrPoint == corrIndex)
			return &(*it);
	}

	return NULL;
}

double * GridSquare::getWeights( int corrIndex )
{
	for(Vector<GridPoint>::iterator it = points.begin(); it != points.end(); it++)
	{
		if(it->corrPoint == corrIndex)
			return it->w;
	}

	return NULL;
}
