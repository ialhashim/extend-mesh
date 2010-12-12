#include "SimpleDraw.h"

#include "QGLViewer/qglviewer.h"

void SimpleDraw::IdentifyLines(Vector<Line> & lines, float lineWidth, float r, float g, float b)
{
	glDisable(GL_LIGHTING);
	glLineWidth(lineWidth);

	glColor3f(r, g, b);

	glBegin(GL_LINES);
        for(int i = 0; i < (int)lines.size(); i++)
	{
		glVertex3fv(lines[i].a);
		glVertex3fv(lines[i].b);
	}
	glEnd();

	glEnable(GL_LIGHTING);
}

void SimpleDraw::DrawBox(const Vec& center, float width, float length, float height, float r, float g, float b)
{
	glDisable(GL_LIGHTING);

	glColor3f(r, g, b);

	Vec c1, c2, c3, c4;
	Vec bc1, bc2, bc3, bc4;

	c1 = Vec(width, length, height) + center;
	c2 = Vec(-width, length, height) + center;
	c3 = Vec(-width, -length, height) + center;
	c4 = Vec(width, -length, height) + center;

	bc1 = Vec(width, length, -height) + center;
	bc2 = Vec(-width, length, -height) + center;
	bc3 = Vec(-width, -length, -height) + center;
	bc4 = Vec(width, -length, -height) + center;

	glLineWidth(1.0f);

	glBegin(GL_LINES);
		glVertex3fv(c1);glVertex3fv(bc1);
		glVertex3fv(c2);glVertex3fv(bc2);
		glVertex3fv(c3);glVertex3fv(bc3);
		glVertex3fv(c4);glVertex3fv(bc4);
		glVertex3fv(c1);glVertex3fv(c2);
		glVertex3fv(c3);glVertex3fv(c4);
		glVertex3fv(c1);glVertex3fv(c4);
		glVertex3fv(c2);glVertex3fv(c3);
		glVertex3fv(bc1);glVertex3fv(bc2);
		glVertex3fv(bc3);glVertex3fv(bc4);
		glVertex3fv(bc1);glVertex3fv(bc4);
		glVertex3fv(bc2);glVertex3fv(bc3);
	glEnd();

	glEnable(GL_LIGHTING);
	
}

void SimpleDraw::DrawTriangle(const Vec& v1, const Vec& v2, const Vec& v3, float r, float g, float b, float a, bool isOpaque)
{
	glDisable(GL_LIGHTING);

	if(a < 1.0f)
	{
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Draw the filled triangle
	if(isOpaque)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset( 0.5, 0.5 );

		glColor4f(r * 0.75f, g * 0.75f, b * 0.75f, RANGED(0,a,1.0f));

		if(a < 1.0f){
			glEnable(GL_BLEND); 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		glBegin(GL_TRIANGLES);
			glVertex3fv(v1);
			glVertex3fv(v2);
			glVertex3fv(v3);
		glEnd();

		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	// Draw the edges
	glLineWidth(1.0f);

	glColor3f(r * 0.5f, g * 0.5f, b * 0.5f);

	glBegin(GL_LINE_STRIP);
		glVertex3fv(v1);
		glVertex3fv(v2);
		glVertex3fv(v3);
		glVertex3fv(v1);
	glEnd();

	// Draw the points
	int pointSize = 4;

	glEnable(GL_POINT_SMOOTH);

	// Colored dot
	glColor3f(r,g,b);
	glPointSize(pointSize);
	glBegin(GL_POINTS);
		glVertex3fv(v1);
		glVertex3fv(v2);
		glVertex3fv(v3);
	glEnd();

	// White Border
	glPointSize(pointSize + 2);
	glColor3f(1, 1, 1);

	glBegin(GL_POINTS);
		glVertex3fv(v1);
		glVertex3fv(v2);
		glVertex3fv(v3);
	glEnd();

	glDisable (GL_BLEND);

	glEnable(GL_LIGHTING);
}

void SimpleDraw::DrawTriangles( const Vector<Vector<Vec> >& tris, float r, float g, float b, float a, bool isOpaque, bool isDrawPoints)
{
	glDisable(GL_LIGHTING);

	if(a < 1.0f)
	{
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Draw the filled triangle
	if(isOpaque)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset( 0.5, 0.5 );

		glColor4f(r * 0.75f, g * 0.75f, b * 0.75f, RANGED(0,a,1.0f));

		if(a < 1.0f){
			glEnable(GL_BLEND); 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		glBegin(GL_TRIANGLES);
                for(int i = 0; i < (int)tris.size(); i++)
		{
			glVertex3fv(tris[i][0]);
			glVertex3fv(tris[i][1]);
			glVertex3fv(tris[i][2]);
		}
		glEnd();

		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	// Draw the edges
	glLineWidth(1.0f);

	if(isOpaque)
		glColor3f(r * 0.5f, g * 0.5f, b * 0.5f);
	else
		glColor3f(r, g, b);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBegin(GL_TRIANGLES);
        for(int i = 0; i < (int)tris.size(); i++)
	{
		glVertex3fv(tris[i][0]);
		glVertex3fv(tris[i][1]);
		glVertex3fv(tris[i][2]);
	}
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if(isDrawPoints)
	{
		// Draw the points
		int pointSize = 4;

		glEnable(GL_POINT_SMOOTH);

		// Colored dot
		glColor3f(r,g,b);
		glPointSize(pointSize);
		glBegin(GL_POINTS);
                for(int i = 0; i < (int)tris.size(); i++)
		{
			glVertex3fv(tris[i][0]);
			glVertex3fv(tris[i][1]);
			glVertex3fv(tris[i][2]);
		}
		glEnd();

		// White Border
		glPointSize(pointSize + 2);
		glColor3f(1, 1, 1);

		glBegin(GL_POINTS);
                for(int i = 0; i < (int)tris.size(); i++)
		{
			glVertex3fv(tris[i][0]);
			glVertex3fv(tris[i][1]);
			glVertex3fv(tris[i][2]);
		}
		glEnd();
	}

	glDisable (GL_BLEND);

	glEnable(GL_LIGHTING);
}

void SimpleDraw::DrawLineTick(const Vector<Vec>& start, const Vector<Vec>& direction, float len, bool border, float r, float g, float b, float a)
{
	glPushAttrib( GL_ALL_ATTRIB_BITS );

	glDisable(GL_LIGHTING);

	if(a < 1.0f){
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	if(border)
	{
		glClearStencil(0);
		glClear( GL_STENCIL_BUFFER_BIT );
		glEnable( GL_STENCIL_TEST );

		glStencilFunc( GL_ALWAYS, 1, 0xFFFF );
		glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glColor3f( 0.0f, 0.0f, 0.0f );
	}

	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glColor4f(r, g, b, a);
        for(int i = 0; i < (int)start.size(); i++)
	{
		glVertex3fv(start[i]);
		glVertex3fv(start[i] + (direction[i] * len));
	}
	glEnd();

	if(border)
	{
		glStencilFunc( GL_NOTEQUAL, 1, 0xFFFF );
		glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

		glLineWidth(4.0f);
		glBegin(GL_LINES);
		glColor4f(0, 0, 0, 1);
                for(int i = 0; i < (int)start.size(); i++)
		{
			glVertex3fv(start[i]);
			glVertex3fv(start[i] + (direction[i] * len * 1.05f));
		}
		glEnd();
	}


	glPopAttrib();

}

void SimpleDraw::DrawSquare(const Vec& v1, const Vec& v2, const Vec& v3, const Vec& v4, 
							bool isOpaque, float r, float g, float b, float a)
{
	glDisable(GL_LIGHTING);

	if(isOpaque)
	{
		// Draw the filled square
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset( 0.5, 0.5 );

		glColor4f(r * 0.75f, g * 0.75f, b * 0.75f, RANGED(0, a, 1.0f));

		if(a < 1.0f){
			glEnable(GL_BLEND); 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		glBegin(GL_QUADS);
		glVertex3fv(v1);
		glVertex3fv(v2);
		glVertex3fv(v3);
		glVertex3fv(v4);
		glEnd();

		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	// Draw the edges
	glLineWidth(1.0f);

	glColor3f(r, g, b);

	glBegin(GL_LINE_STRIP);
	glVertex3fv(v1);
	glVertex3fv(v2);
	glVertex3fv(v3);
	glVertex3fv(v4);
	glVertex3fv(v1);
	glEnd();

	glEnable(GL_LIGHTING);
}

void SimpleDraw::DrawSquares( const Vector<Vector<Vec> >& squares, bool isOpaque, float r, float g, float b, float a )
{
	glDisable(GL_LIGHTING);

	if(isOpaque)
	{
		// Draw the filled square
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset( 0.5, 0.5 );

		glColor4f(r * 0.75f, g * 0.75f, b * 0.75f, RANGED(0, a, 1.0f));

		if(a < 1.0f){
			glEnable(GL_BLEND); 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		glBegin(GL_QUADS);
                for(int i = 0; i < (int)squares.size(); i++)
		{
			glVertex3fv(squares[i][0]);
			glVertex3fv(squares[i][1]);
			glVertex3fv(squares[i][2]);
			glVertex3fv(squares[i][3]);
		}
		glEnd();

		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	// Draw the edges
	glLineWidth(1.0f);

	glColor3f(r, g, b);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBegin(GL_QUADS);
        for(int i = 0; i < (int)squares.size(); i++)
	{
		glVertex3fv(squares[i][0]);
		glVertex3fv(squares[i][1]);
		glVertex3fv(squares[i][2]);
		glVertex3fv(squares[i][3]);
	}
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_LIGHTING);
}

void SimpleDraw::DrawCube( const Vec& center, float length /*= 1.0f*/ )
{
	static GLfloat n[6][3] ={
		{-1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{1.0, 0.0, 0.0},
		{0.0, -1.0, 0.0},
		{0.0, 0.0, 1.0},
		{0.0, 0.0, -1.0}};
	static GLint faces[6][4] ={
		{0, 1, 2, 3},
		{3, 2, 6, 7},
		{7, 6, 5, 4},
		{4, 5, 1, 0},
		{5, 6, 2, 1},
		{7, 4, 0, 3}};
	GLfloat v[8][3];GLint i;

	v[0][0] = v[1][0] = v[2][0] = v[3][0] = -length / 2;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = length / 2;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = -length / 2;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = length / 2;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = -length / 2;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = length / 2;

	glPushMatrix();
	glTranslatef(center.x, center.y, center.z);

	for (i = 0; i < 6; i++) {
		glBegin(GL_QUADS);
		glNormal3fv(&n[i][0]);
		glVertex3fv(&v[faces[i][0]][0]);
		glVertex3fv(&v[faces[i][1]][0]);
		glVertex3fv(&v[faces[i][2]][0]);
		glVertex3fv(&v[faces[i][3]][0]);
		glEnd();
	}

	glPopMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void SimpleDraw::DrawSphere( const Vec& center, float radius /*= 1.0f*/ )
{
	glPushMatrix();
	glTranslatef(center.x, center.y, center.z);
	GLUquadricObj *quadObj = gluNewQuadric();

	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricNormals(quadObj, GLU_SMOOTH);

	gluSphere(quadObj, radius, 16, 16);

	gluDeleteQuadric(quadObj);
	glPopMatrix();
}

void SimpleDraw::DrawCylinder( const Vec& center, const Vec& direction /*= Vec(0,0,1)*/,
							  float height, float radius /*= 1.0f*/, float radius2 /*= -1*/ )
{
	glPushMatrix();
	glTranslatef(center.x, center.y, center.z);
	glMultMatrixd(qglviewer::Quaternion(Vec(0,0,1), direction).matrix());

	GLUquadricObj *quadObj = gluNewQuadric();
	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricNormals(quadObj, GLU_SMOOTH);

	if(radius2 < 0)
		radius2 = radius;

	gluCylinder(quadObj, radius, radius2, height, 16, 16);

	gluDeleteQuadric(quadObj);
	glPopMatrix();
}

void SimpleDraw::DrawArrow( Vec from, Vec to, bool isForward /*= true*/ , bool isFilledBase)
{
	if(!isForward){
		Vec temp = from;
		from = to;
		to = temp;
	}

	if(isFilledBase) 
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	float length = (from-to).norm();
	float radius = length * 0.02f;
	if (radius < 0.0)
		radius = 0.05 * length;

	// draw cube base
	DrawCube(from, radius * 3);

	glPushMatrix();
	glTranslatef(from[0],from[1],from[2]);
	glMultMatrixd(qglviewer::Quaternion(Vec(0,0,1), to-from).matrix());

	static GLUquadric* quadric = gluNewQuadric();

	const float head = 2.5*(radius / length) + 0.1;
	const float coneRadiusCoef = 4.0 - 5.0 * head;

	gluCylinder(quadric, radius, radius, length * (1.0 - head/coneRadiusCoef), 16, 1);
	glTranslatef(0.0, 0.0, length * (1.0 - head));
	gluCylinder(quadric, coneRadiusCoef * radius, 0.0, head * length, 16, 1);

	glPopMatrix();
}

void SimpleDraw::DrawArrowDirected( const Vec& pos, const Vec& normal, float height /*= 1.0f*/, 
								    bool isForward /*= true*/ , bool isFilledBase)
{
	DrawArrow(pos, pos + (normal*height), isForward, isFilledBase);
}

void SimpleDraw::PointArrowAt( Vec point, float radius /*= 1.0f*/ )
{
	DrawArrowDirected(point, point + (radius*Vec(Max(0.2, point.x),point.y,point.z).unit()), radius, false);
}

void SimpleDraw::DrawArrowDoubleDirected( const Vec& pos, const Vec& normal, float height /*= 1.0f*/, 
										 bool isForward /*= true*/, bool isFilledBase /*= true*/ )
{
	DrawArrowDirected(pos, normal, height, isForward, isFilledBase);
	
	glColor3f(1,0,0);
	DrawArrowDirected(pos, -normal, height, isForward, isFilledBase);
}
