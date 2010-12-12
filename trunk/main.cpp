#include "ExtendMeshHeaders.h"

// Shared memory
QMap <QString, Mesh *> meshes;
Skeleton skeleton;
Displacements * df;
DisplacementsWidget * df_widget;
StdList<GridMesh> gmesh;
UserCurve * user_curve;

// Share UI
ExtendMesh * mainWindow;

// Debug stuff
Vector<Vec> testPoints1,testPoints2,testPoints3;
Vector<Line> testLines1,testLines2,testLines3;
Vector<Plane> testPlanes1,testPlanes2,testPlanes3;
StdMap<int, Vec> testText1,testText2;

// Statistics stuff
QMap<QString, Stats> stats;

double Epsilon = 1.0e-7f;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// Anti-aliasing
	QGLFormat glf = QGLFormat::defaultFormat();
	glf.setSampleBuffers(true);
	glf.setSamples(8);
	QGLFormat::setDefaultFormat(glf);

	ExtendMesh w;
	w.showMaximized();

	// Pointers to expose to the entire world!
	mainWindow = &w;
	mainWindow->ui.commander->viewer = mainWindow->ui.viewer;

	return a.exec();
}

Mesh * getMesh(QString id)
{
	if(meshes.find(id) == meshes.end())
		return NULL;

	return meshes[id];
}

Mesh * newMesh(QString id)
{
	meshes[id] = new Mesh();
	meshes[id]->id = id.toStdString();

	return meshes[id];
}

void clearStats()
{
	stats.clear();
}

void printStats()
{
	printf("\n===========================================================");
	printf("\n================   STATS  =================================\n\n");

	for(QMap<QString, Stats>::iterator it = stats.begin(); it != stats.end(); it++)
		it->print();

	printf("===========================================================\n");
}

void saveStatsToFile(QString fileName)
{
	Mesh * loadedMesh = getMesh("LoadedMesh");

	if(!fileName.length())
		fileName = QString(loadedMesh->id.c_str());

	FILE *fp = fopen((fileName + ".log.txt").toAscii(),"w");

	if(fp == NULL)	return;

	for(QMap<QString, Stats>::iterator it = stats.begin(); it != stats.end(); it++)
	{
		fprintf(fp, "%s,%f\n", it->getLabel().toStdString().c_str(), it->getValue());
	}

	fclose(fp);
}
