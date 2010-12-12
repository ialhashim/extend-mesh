#include "Grid.h"

#include "CrossSection.h"

CrossSection::CrossSection(int v, Grid * grid, const Color4 & newColor)
{
	Vec sumPoints;

	for(int u = 0; u < grid->widthCount; u++)
	{
		GridSquare * square = grid->getSquare(u, Min(v, grid->lengthCount - 1));

		Vertex * pp;

		if(v < grid->lengthCount)
			pp = grid->v(square->p[0]);
		else
			pp = grid->v(square->p[3]);

		Vec p = *pp;

		point.push_back(p);

		sumPoints += p;
	}
	
	//center = sumPoints / grid->widthCount;
	center = grid->spinePoints[v];

	// Subtract center
        for(int i = 0; i < (int)point.size(); i++)
		point[i] -= center;

	normal = (point[0] ^ point[1]).unit();

	color = newColor;
}

CrossSection::CrossSection(float t, const CrossSection & c1, const CrossSection & c2, const Color4 & newColor)
{
	this->grid = c1.grid;
	this->center = c1.center;
	this->normal = c1.normal;

	if(t == 0.0f)
	{
		this->point = c1.point;
	}
	else
	{
		CrossSection c2temp = c2;
		c2temp.rotate( Rotation (c2.normal, c1.normal) );

                for(int i = 0; i < (int)c1.point.size(); i++)
			this->point.push_back(Vertex::interpolateVec(t, c1.point[i], c2temp.point[i]));
	}

	color = newColor;
}

CrossSection CrossSection::shiftedCopy(const Vec & new_center, const Vec & new_normal)
{
	CrossSection modified_copy = CrossSection(*this);

	modified_copy.translate(new_center);
	modified_copy.align(new_normal);

	return modified_copy;
}

CrossSection::CrossSection(float radius, int numSides,const Vec & fromCenter, const Vec & fromNormal, const Color4 & newColor)
{
	this->center = Vec(0,0,0);
	this->normal = Vec(0,0,1);

	this->point = Circle(radius, numSides).getPoints();

        this->color = newColor;

	translate(fromCenter);
	align(fromNormal);
}

CrossSection::CrossSection(const CrossSection & from)
{
	this->center = from.center;
	this->normal = from.normal;
	this->point = from.point;
	this->color = from.color;
	this->grid = from.grid;
}

CrossSection& CrossSection::operator= (const CrossSection & from)
{
    if (this != &from)
	{
		this->center = from.center;
		this->normal = from.normal;
		this->point = from.point;
		this->color = from.color;
		this->grid = from.grid;
	}

	return *this;
}

Vec CrossSection::operator[] (int index)
{
	return pointAt(index);
}

Vector<Vec> CrossSection::getAllPoints()
{
	return point;
}

void CrossSection::translate(const Vec & to)
{
	center = to;
}

void CrossSection::translateBy(const Vec & delta)
{
	center += delta;
}

void CrossSection::rotate(const Rotation & q)
{
        for(unsigned int i = 0; i < point.size(); i++)
        {
                Vec old_p = point[i];
                point[i] = q.rotate(old_p);
        }

	normal = q.rotate(normal);
}

void CrossSection::rotateAround(const Vec & pivot, const Rotation & q)
{
	int N = point.size();

	Vec oldCenter = center;

	Vector<Vec> absPoint;

	for(int i = 0; i < N; i++)
	{
		absPoint.push_back(Vertex::RotateAround(pointAt(i), pivot, q));
	}

	center = Vertex::RotateAround(center, pivot, q);

	for(int i = 0; i < N; i++)
		point[i] = absPoint[i] - center;

	normal = q.rotate(normal);
}

void CrossSection::align(const Vec & to)
{
	rotate(Rotation(normal, to.unit()));
}

void CrossSection::setColor(const Color4 & newColor)
{
	color = newColor;
}

Vec CrossSection::pointAt(int u, int offset)
{
	return center + point[(u + offset) % point.size()];
}

void CrossSection::scaleBy(float factor)
{
	int N = point.size();

	for(int i = 0; i < N; i++)
		point[i] = (point[i] * (1 + factor));
}

void CrossSection::smooth(int numIterations)
{
	int N = point.size();

	for(int itr = 0; itr < numIterations; itr++)
	{
		Vector<Vec> temp = point;
		for(int i = 0; i <= N; i++)
			temp[(i+1) % N] = ((temp[(i) % N] + temp[(i+2) % N]) / 2.0f).unit();
		point = temp;
	}
}

Vector<float> CrossSection::scaleVector(const CrossSection& other)
{
	int N = point.size();
	Vector<float> scaleVector(N);

	for(int i = 0; i < N; i++)
	{
		Vec v1 = point[i] - center;
		Vec v2 = other.point[i] - other.center;

		scaleVector[i] = v1.norm() / v2.norm();
	}

	return scaleVector;
}

Vector<float> CrossSection::lengths()
{
	int N = point.size();
	Vector<float> lengths(N);

	for(int i = 0; i < N; i++)
		lengths[i] = point[i].norm();

	return lengths;
}

void CrossSection::setLengths(const Vector<float> & lengths)
{
	int N = point.size();

	for(int i = 0; i < N; i++)
		point[i] = lengths[i] * point[i].unit();
}

void CrossSection::setNormal(const Vec & toNormal)
{
	normal = toNormal;
}

Vec CrossSection::getNormal()
{
	return normal.unit();
}

Vec CrossSection::getCenter()
{
	return center;
}

Vec CrossSection::getUp()
{
	return point[0].unit();
}

int CrossSection::numSegments()
{
	return point.size();
}

float CrossSection::borderLength()
{
	int N = point.size();

	float len = 0;

	for(int i = 1; i < N; i++)
	{
		len += (point[i] - point[i - 1]).norm();
	}

	return len;
}

void CrossSection::draw(float lineWidth)
{
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(lineWidth);
	glColor4ubv(color);

	glBegin(GL_LINE_STRIP);
        for(int i = 0; i <= (int)point.size(); i++)
		glVertex3fv(point[i % point.size()] + center);
	glEnd();

	// draw center point
	glPointSize(6);
	glColor3f(1,0,0);
	glBegin(GL_POINTS);
		glVertex3fv(center);
	glEnd();

	// Draw normal
	glLineWidth(2.0f);
	glBegin(GL_LINES);
		glColor4f(0.4f,0.6f,0.4f, 0.25f);
		glVertex3fv(center);

		glColor4f(0.85f,1,0.85f, 1.0f);
		glVertex3fv(center + (normal * 0.025f));
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
}
