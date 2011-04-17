#ifndef SIMPLE_DRAW_H
#define SIMPLE_DRAW_H

#include "Vertex.h"
#include "Line.h"

class SimpleDraw
{
public:

	static void IdentifyPoint(const Vec& p, float r = 1.0, float g = 0.2f, float b = 0.2f, float pointSize = 10.0)
	{
		glDisable(GL_LIGHTING);

		// Colored dot
		glColor3f(r, g, b);
		glPointSize(pointSize);
		glBegin(GL_POINTS);
                glVertex3f(p.x, p.y, p.z);
		glEnd();

		// White Border
		glPointSize(pointSize + 2);
		glColor3f(1, 1, 1);

		glBegin(GL_POINTS);
			glVertex3f(p.x, p.y, p.z);
		glEnd();

		glEnable(GL_LIGHTING);
	}

	static void IdentifyPoint2(Vec p){	// Green
		IdentifyPoint(p, 0.2f, 1.0f, 0.2f, 12.0f);
	}

	static void IdentifyPoints(Vector<Vec> & points, float r = 1.0, float g = 0.2f, float b = 0.2f, float pointSize = 10.0)
	{
		glDisable(GL_LIGHTING);

		// Colored dot
		glColor3f(r, g, b);
		glPointSize(pointSize);
		glBegin(GL_POINTS);
                        for(unsigned int i = 0; i < points.size(); i++)
				glVertex3fv(points[i]);
		glEnd();

		// White Border
		glPointSize(pointSize + 2);
		glColor3f(1, 1, 1);

		glBegin(GL_POINTS);
                        for(unsigned int i = 0; i < points.size(); i++)
				glVertex3fv(points[i]);
		glEnd();

		glEnable(GL_LIGHTING);
	}

	static void IdentifyConnectedPoints(Vector<Vec> & points, float r = 0.4f, float g = 1.0, float b = 0.2f)
	{
		glDisable(GL_LIGHTING);

		int N = points.size();

		glLineWidth(3.0);
		glBegin(GL_LINE_STRIP);
		for(int i = 0; i < N; i++)
		{
			float t = Min((float(i) / N + 0.25f), 1.0);
			glColor3f(r * t, g * t, b * t);
			glVertex3fv(points[i]);
		}
		glEnd();

		// Colored dot
		glColor3f(r, g, b);
		glPointSize(13.0);
		glBegin(GL_POINTS);
		for(int i = 0; i < N; i++)
		{
			float t = float(i) / N;
			glColor3f(r * t, g * t, b * t);
			glVertex3fv(points[i]);
		}
		glEnd();

		// White Border
		glPointSize(15.0);
		glColor3f(1, 1, 1);

		glBegin(GL_POINTS);
			for(int i = 0; i < N; i++)
				glVertex3fv(points[i]);
		glEnd();

		glEnable(GL_LIGHTING);
	}

	static void IdentifyLine(const Vec& p1, const Vec& p2, bool showPoints = true)
	{
		// Blue line
		IdentifyLine(p1, p2, 0.2f, 0.2f, 1.0, showPoints);
	}

	static void IdentifyLineRed(const Vec& p1, const Vec& p2, bool showPoints = true)
	{
		// Red line
		IdentifyLine(p1, p2, 1.0, 0.2f, 0.2f, showPoints);
	}

	static void IdentifyLine(const Vec& p1, const Vec& p2, float r, float g, float b, bool showPoints = true)
	{
		glDisable(GL_LIGHTING);

		//glClear(GL_DEPTH_BUFFER_BIT);

		// Set color
		glColor3f(r, g, b);

		glLineWidth(3.0);
		glBegin(GL_LINES);
			glVertex3fv(p1);
			glVertex3fv(p2);
		glEnd();

		if(showPoints)
		{
			// Draw colored end points
			glPointSize(12.0);
			glBegin(GL_POINTS);
				glVertex3fv(p1);
				glVertex3fv(p2);
			glEnd();

			// White border end points
			glPointSize(14.0);
			glColor3f(1, 1, 1);

			glBegin(GL_POINTS);
				glVertex3fv(p1);
				glVertex3fv(p2);
			glEnd();
		}

		glEnable(GL_LIGHTING);
	}

	static void IdentifyArrow(const Vec & start, const Vec & end, 
			float lineWidth = 2.0, float r = 1.0, float g = 0.2f, float b = 0.2f)
	{
		glDisable(GL_LIGHTING);

		// Transperncy
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glLineWidth(lineWidth);

		glBegin(GL_LINES);

		glColor4f(r/2, g/2, b/2, 0.2f);
		glVertex3fv(start);

		glColor4f(r, g, b, 1.0);
		glVertex3fv(end);

		glEnd();
		
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
		glLineWidth(1.0);
	}

	static void IdentifyArrows(Vector<Vec> & starts, Vector<Vec> & ends, 
			float lineWidth = 2.0, float r = 1.0, float g = 0.2f, float b = 0.2f)
	{
		glDisable(GL_LIGHTING);

		// Transperncy
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glLineWidth(lineWidth);

		glBegin(GL_LINES);
                for(unsigned int i = 0; i < starts.size(); i++)
		{
			glColor4f(r, g, b, 0.0);
			glVertex3fv(starts[i]);

			glColor4f(r, g, b, 1.0);
			glVertex3fv(ends[i]);
		}
		glEnd();
		
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
	}

	static void IdentifyLines(Vector<Line> & lines, float lineWidth = 1.0, float r = 1.0, float g = 0.6f, float b = 0);

	static void DrawBox(const Vec& center, float width, float length, float height, float r = 0, float g = 1.0, float b = 0);
	static void DrawTriangle(const Vec& v1, const Vec& v2, const Vec& v3,float r = 0.2f, float g = 1.0, float b = 0.1f, float a = 1.0, bool isOpaque = true);
        static void DrawTriangles(const Vector< Vector<Vec> > & tris, float r = 0.2f, float g = 1.0, float b = 0.1f, float a = 1.0, bool isOpaque = true, bool isDrawPoints = true);

	static void DrawSquare(const Vec& v1, const Vec& v2, const Vec& v3, const Vec& v4, bool isOpaque = true, 
		float r = 0.1f, float g = 0.2f, float b = 1.0, float a = 1.0);

        static void DrawSquares(const Vector<Vector<Vec> > & squares, bool isOpaque = true,
		float r = 0.1f, float g = 0.2f, float b = 1.0, float a = 1.0);

	static void DrawLineTick(const Vector<Vec>& start, const  Vector<Vec>& direction, 
		float len = 0.25f, bool border = false, float r = 0.65f, float g = 0.6f, float b = 0.8f, float a = 0.5f);

	static void DrawCube(const Vec& center, float length = 1.0);
	static void DrawSphere(const Vec& center, float radius = 1.0);
	static void DrawCylinder(const Vec& center, const Vec& direction = Vec(0,0,1),
		float height = 1.0, float radius = 1.0, float radius2 = -1);

	static void DrawArrow(Vec from, Vec to, bool isForward = true, bool isFilledBase = true);
	static void DrawArrowDirected(const Vec& pos, const Vec& normal, float height = 1.0, bool isForward = true, bool isFilledBase = true);
	static void DrawArrowDoubleDirected(const Vec& pos, const Vec& normal, float height = 1.0, bool isForward = true, bool isFilledBase = true);

	static void PointArrowAt(Vec point, float radius = 1.0);
};

#endif // SIMPLE_DRAW_H
