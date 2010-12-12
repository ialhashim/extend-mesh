#ifndef CROSS_SECTION_H
#define CROSS_SECTION_H

#include "Circle.h"

#include "ClosedPolygon.h"

class Grid;

class CrossSection
{
private:
	Vector<Vec> point;

	Vec center;
	Vec normal;

	Grid * grid;

	Color4 color;

public:
	CrossSection(){ grid = NULL; }
	CrossSection(int v, Grid * grid, const Color4 & newColor = Color4());
	CrossSection(float t, const CrossSection & c1, const CrossSection & c2, const Color4 & newColor = Color4());
	CrossSection(float radius, int numSides, const Vec & fromCenter, 
		const Vec & fromNormal, const Color4 & newColor = Color4());

	// Copy & assignment
	CrossSection(const CrossSection&);
	CrossSection& operator= (const CrossSection&); 

	Vec operator[] (int index);
	Vector<Vec> getAllPoints();

	CrossSection shiftedCopy(const Vec & new_center, const Vec & new_normal);

	void setNormal(const Vec & toNormal);

	Vec pointAt(int u, int offset = 0);

	Vec getNormal();
	Vec getCenter();
	Vec getUp();

	void translate(const Vec & to);
	void translateBy(const Vec & delta);
	void rotate(const Rotation & q);
	void rotateAround(const Vec & pivot, const Rotation & q);
	void align(const Vec & to);
	void scaleBy(float factor);
	void smooth(int numIterations);

	Vector<float> lengths();
	Vector<float> scaleVector(const CrossSection& other);
	void setLengths(const Vector<float> & lengths);

	float borderLength();

	void setColor(const Color4 & newColor);

	int numSegments();

	void draw(float lineWidth = 1.0f);
};

#endif // CROSS_SECTION_H
