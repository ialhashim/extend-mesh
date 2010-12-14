#pragma once

#include "Macros.h"

// Qt includes
#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include <QGLFormat>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QColor>
#include <QImage>
#include <QPixmap>
#include <QElapsedTimer>

// Our classes
#include "Mesh.h"
extern QMap <QString, Mesh *> meshes;

#include "Skeleton.h"
extern Skeleton skeleton;

#include "Displacements.h"
extern Displacements * df;

#include "DisplacementsWidget.h"
extern DisplacementsWidget * df_widget;

#include "GridMesh.h"
extern StdList<GridMesh> gmesh;

#include "UserCurve.h"
extern UserCurve * user_curve;

#include "Stats.h"
extern QMap<QString, Stats> stats;

// UI classes
#include "Viewer.h"
#include "Commander.h"

#include "ui_extendMesh.h"
#include "extendMesh.h"
extern ExtendMesh * mainWindow;

// Mesh management
Mesh * getMesh(QString id);
Mesh * newMesh(QString id);
void clearMeshes();

// Statistics
void clearStats();
void printStats();
void saveStatsToFile(QString fileName = "");

// DEBUG:
#include "SimpleDraw.h"

extern Vector<Vec> testPoints1;
extern Vector<Vec> testPoints2;
extern Vector<Vec> testPoints3;

extern Vector<Plane> testPlanes1;
extern Vector<Plane> testPlanes2;
extern Vector<Plane> testPlanes3;

extern Vector<Line> testLines1;
extern Vector<Line> testLines2;
extern Vector<Line> testLines3;

extern StdMap<int, Vec> testText1;
extern StdMap<int, Vec> testText2;
