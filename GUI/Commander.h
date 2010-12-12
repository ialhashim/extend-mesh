#pragma once

#include <QWidget>
#include <QString>

class Viewer;
class Displacements;

class Commander : public QWidget
{
	Q_OBJECT

public:
	Commander(QWidget *parent = 0);
	~Commander();

	Viewer * viewer;

	void OpenSkeleton(QString fileName, QString corrFileName);

	QString loadedFileName;

public slots:
	
	// LOAD DATA / SAVE
	void OpenMesh();
	void SaveMesh();

	// SMOOTHING
	void SmoothRegular();
	void SmoothCurvature();

	// DISPLACEMENT FIELD
	void UpdateDisplacements(Displacements*);
	void SelectModeReconstructedPoints();

	// CAMERA OPTIONS
	void ToggleCameraProjection();

	void ChangeBackgroundColor();
	void ChangeRenderStyle();

	// RENDER OPTIONS - MESH
	void ToggleVisible();
	void ToggleWireframe();
	void ToggleShowAsNormalizedPoints();
	void ToggleShowNormals();

	// RENDER OPTIONS - SKELETON
	void ShowHideSkeleton();
	void ToggleSkelUserFriendly();

	// RENDER OPTIONS - DF
	void ToggleShowHideAllDF();
	void ToggleShowDisplacements();
	
	// RENDER OPTIONS - GRID MESH
	void ToggleShowCrossSections();
	void ToggleShowGridMesh();
	void ToggleWireframeGridMesh();
	void ToggleShowColoredPatches();

	// SCRIPTING
	void SaveScript();
	void LoadScript();
	void LoadUserCurve();

	// test
	void InfoMesh();
	void OutputGridMesh();
};
