#pragma once

#include "Mesh.h"
#include "Skeleton.h"

#include <QMetaType>
#include <QColorDialog>
#include <QQueue>

#include <QGLViewer/qglviewer.h>

enum ViewMode { VIEW, SELECTION, MODIFY };
enum SelectMode { NONE, MESH, SKELETON_NODE, SKELETON_EDGE, SKELETON_FACES, RECONSTRUCTED_POINTS, VERTEX};
enum ModifyMode { DEFAULT, CP_REF_VECTOR, MOVE_VERTEX };

class Viewer : public QGLViewer
{
	Q_OBJECT

private:
	bool mouseAltPressed;
	int renderStyle;
	QColor backColor;

	QQueue<QString> messages;
	QTimer *timer;

public:
	Viewer(QWidget *parent = 0);
	~Viewer();

	virtual void draw();
	virtual void drawWithNames();
	virtual void init();

	// Mouse & Keyboard stuff
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void keyPressEvent(QKeyEvent *e);

	// SELECTION
	virtual void postSelection(const QPoint& point);

	// STATE
	ViewMode viewMode;
	SelectMode selectMode;
	ModifyMode modifyMode;

	void setViewMode(ViewMode toMode);
	void setSelectMode(SelectMode toMode);
	void setModifyMode(ModifyMode toMode);

	// Useful functions for producing results for paper
	void askUserBackground();
	void changeRenderStyle();	

public slots:
	// TEXT ON SCREEN
	void print(QString message, long age = 500);
	void dequeueLastMessage();

signals:
	void CreateDF();
};
