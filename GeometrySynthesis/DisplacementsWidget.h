#pragma once

#include <QWidget>

#include <QVBoxLayout>
#include <QGridLayout>
#include <QDockWidget>

#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QImage>

#include "Displacements.h"

#include "GridVisualizer.h"

#include "TextureSynthesis/TextureSynthesizer.h"

#include "Slicer.h"

struct SmoothSettings{int numSteps; double stepSize; int numIter;};

class DisplacementsWidget: public QWidget
{
Q_OBJECT

private:
	Mesh * sourceMesh;
	Skeleton * skeleton;

	Mesh * workingMesh;

	// Main layout
	QVBoxLayout * layout;

	// Options
	QFrame * options;
	QGridLayout * optionsLayout;
	QSpinBox * num_smooth_steps;
	QDoubleSpinBox * step_size;
	QSpinBox * num_iteration;
	QCheckBox * volume_preserve;

	QSpinBox * grid_square_size;
	QComboBox * fitting_method;

	// Grid visualizer options
	QDockWidget * visDockWidget;
	QFrame * visFrame;
	QGridLayout * visLayout;
	QSlider * smooth_level;
	QPushButton * buttonTogglePatch;
	QLabel * crossSectionImage;
	QLabel * largeScalePatternImage;
	QLabel * csExtendedImage;
	QLabel * lsExtendedImage;

	// Buttons
	QPushButton * buttonSmoothing;
	QPushButton * buttonCreateDF;

	// Synthesis Options
	QFrame * synthesisFrame;
	QGridLayout * synthesisLayout;
	QComboBox * synthesisType;
	QCheckBox * synthCrossSections;
	QCheckBox * blendCrossSections;
	QCheckBox * changeProfile;
	QCheckBox * fillSeams;
	QCheckBox * blendSeams;
	QCheckBox * sampleSeams;
	QSpinBox * block_size;
	QSpinBox * band_size;
	QSpinBox * rotate_left;
	QSpinBox * rotate_right;
	QPushButton * directSynthesize;
	QPushButton * buttonTriangulate;

	QDockWidget * resultsWidget;
	QFrame * resultsFrame;
	QLabel * outputImage;
	QGridLayout * resultsLayout;
	
	QImage tempImage;

	Vector<Vertex> originalMesh;

	// Cutting and slicing
	SliceResult sliceOp;
	bool isMeshCut;
	Vector<Vertex> resetPosition;
	Vector<int> halfMesh;
	Vector<int> cutPoints, newPoints;

	bool firstGridMesh;

	SmoothSettings prevSmooth;

protected:
	
public:
	DisplacementsWidget(Mesh*, Skeleton*, QWidget *parent);
	
	Displacements * df;
	GridVisualizer * gv;

	int getBandSize();
	int getNumSmoothSteps();
	int getNumSmoothIter();
	float getSmoothStepSize();
	int getGridSize();
		
	void setBandSize(int bsize);
	void setNumSmoothSteps(int);
	void setNumSmoothIter(int);
	void setSmoothStepSize(float smoothStep);
	void setGridSize(int);

public slots:
	void DoSmoothing();
	void CreateDF();
	void DirectSynthesize();
	void Triangulate();

	void Print(QString, int age = 500);

signals:
	void UpdateDF(Displacements*);
	void GridChanged(Grid*);
};
