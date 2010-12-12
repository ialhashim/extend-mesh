#include "ExtendMeshHeaders.h"
#include "DisplacementsWidget.h"

#include "TextureSynthesizer.h"
using namespace Synth;

#include "Matrixf.h"
using namespace Matrixf;

DisplacementsWidget::DisplacementsWidget(Mesh *mesh, Skeleton *s, QWidget *parent) : QWidget(parent)
{
	this->sourceMesh = mesh;
	this->skeleton = s;

	this->setWindowTitle( "Displacement Field" );
	this->setAutoFillBackground( true );
	this->setMinimumWidth(275);

	// MAIN LAYOUT
	layout = new QVBoxLayout;
	layout->setMargin(5);
	layout->setAlignment(Qt::AlignTop);

	// OPTIONS FRAME =====================================================
	options = new QFrame(this);
	optionsLayout = new QGridLayout();

	step_size = new QDoubleSpinBox();
	num_smooth_steps = new QSpinBox();
	num_iteration = new QSpinBox();
	grid_square_size = new QSpinBox();
	fitting_method = new QComboBox();
	volume_preserve = new QCheckBox("preserve volume");

	int row = 0;
	optionsLayout->addWidget(new QLabel("Smoothing Options:"), row++, 0, 1, 3);
	optionsLayout->addWidget(new QLabel("Num Smooth Steps"), row, 0);
	optionsLayout->addWidget(num_smooth_steps, row++, 2);
	optionsLayout->addWidget(new QLabel("Iterations"), row, 0);
	optionsLayout->addWidget(num_iteration, row++, 2);
	optionsLayout->addWidget(new QLabel("Step Size"), row, 0);
	optionsLayout->addWidget(step_size, row++, 2);
	optionsLayout->addWidget(volume_preserve, row++, 0);

	buttonSmoothing = new QPushButton("Compute Base");
	optionsLayout->addWidget(buttonSmoothing, row++, 0, 1, 3);
	
	// Separator line
	QFrame *hline = new QFrame( this );
	hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );
	optionsLayout->addWidget(hline, row++, 0, 1, 3);

	optionsLayout->addWidget(new QLabel("Grid Options:"), row++, 0, 1, 3);
	optionsLayout->addWidget(new QLabel("Square Size"), row, 0);
	optionsLayout->addWidget(grid_square_size, row++, 2);
	optionsLayout->addWidget(new QLabel("Fitting Method"), row, 0);
	optionsLayout->addWidget(fitting_method, row++, 2);

	// Default values
	num_smooth_steps->setRange(1, 256);
	num_smooth_steps->setValue(1);

	num_iteration->setRange(1, 512);
	num_iteration->setValue(1);

	step_size->setDecimals(5);
	step_size->setRange (0.0, 10.0);
	step_size->setValue(0.001f);
	step_size->setSingleStep(0.0005f);

	grid_square_size->setRange(6, 1024);
	grid_square_size->setValue(70);
	grid_square_size->setSingleStep(10);

	fitting_method->addItem("Cross section");
	fitting_method->addItem("Simple Cylinder");
	fitting_method->addItem("None");

	optionsLayout->setAlignment(Qt::AlignTop);
	options->setLayout(optionsLayout);
	options->setFrameStyle(QFrame::StyledPanel);
	options->setMinimumWidth(this->minimumWidth());
	options->setMaximumSize(options->sizeHint());

	// Rotation experiment
	rotate_left = new QSpinBox();
	rotate_right = new QSpinBox();

	rotate_left->setRange(-512,512);
	rotate_left->setValue(0);
	rotate_right->setRange(-512,512);
	rotate_right->setValue(0);

	optionsLayout->addWidget(rotate_left, row, 0);
	optionsLayout->addWidget(rotate_right, row++, 2);

	layout->addWidget(options);

	buttonCreateDF = new QPushButton("Create Displacement Field..");
	layout->addWidget(buttonCreateDF);

	// Visulaziation =====================================================
	visDockWidget = new QDockWidget("Grid Visualization", this);
	visFrame = new QFrame(this);
	visLayout = new QGridLayout();

	gv = new GridVisualizer(this);
	crossSectionImage = new QLabel(" ");
	largeScalePatternImage = new QLabel(" ");
	buttonTogglePatch = new QPushButton("Show/Hide Patch");
	smooth_level = new QSlider(Qt::Horizontal);
	smooth_level->setRange(0, 1);

	row = 1;
	visLayout->addWidget(smooth_level, row, 0);
	visLayout->addWidget(buttonTogglePatch, row++, 1, 1, 1, Qt::AlignRight);
	visLayout->addWidget(gv, row, 0, 1, 1);
	visLayout->addWidget(new QLabel("CS"), row, 1, 1, 1, Qt::AlignRight);
	visLayout->addWidget(crossSectionImage, row, 2);
	visLayout->addWidget(new QLabel("Ls"), row, 3, 1, 1, Qt::AlignRight);
	visLayout->addWidget(largeScalePatternImage, row++, 4);

	visFrame->setLayout(visLayout);
	visDockWidget->setWidget(visFrame);

	layout->addWidget(visDockWidget);

	// SYNTHESIS FRAME =====================================================
	synthesisFrame = new QFrame(this);

	synthesisLayout = new QGridLayout();
	synthesisLayout->setAlignment(Qt::AlignTop);

	synthesisType = new QComboBox();
	synthCrossSections = new QCheckBox("Synth cross-section");
	blendCrossSections = new QCheckBox("Blended cross-sections");
	changeProfile = new QCheckBox("Profile effect");

	fillSeams = new QCheckBox("Fill seams");
	blendSeams = new QCheckBox("Blend seams");
	sampleSeams = new QCheckBox("Sample seams");
	
	block_size = new QSpinBox();
	band_size = new QSpinBox();

	fillSeams->setCheckState(Qt::Checked);

	synthesisType->addItem("Tiling");
	synthesisType->addItem("Patch-Based");
	synthesisType->addItem("Nearest-neighbor interpolation");

	block_size->setRange(4, 1024);
	block_size->setValue(14);
	block_size->setSingleStep(4);

	band_size->setRange(0, 1024);
	band_size->setValue(0);
	band_size->setSingleStep(1);

	row = 0;
	synthesisLayout->addWidget(new QLabel("Synth type"), row, 0);
	synthesisLayout->addWidget(synthesisType, row++, 2);
	synthesisLayout->addWidget(synthCrossSections, row, 0);
	synthesisLayout->addWidget(blendCrossSections, row++, 2);
	synthesisLayout->addWidget(changeProfile, row, 0);
	synthesisLayout->addWidget(sampleSeams, row++, 2);
	synthesisLayout->addWidget(fillSeams, row, 0);
	synthesisLayout->addWidget(blendSeams, row++, 2);

	synthesisLayout->addWidget(new QLabel("Block Size"), row, 0);
	synthesisLayout->addWidget(block_size, row++, 2);
	synthesisLayout->addWidget(new QLabel("Band Size"), row, 0);
	synthesisLayout->addWidget(band_size, row++, 2);

	synthesisFrame->setLayout(synthesisLayout);
	synthesisFrame->setFrameStyle(QFrame::StyledPanel);
	synthesisFrame->setMinimumWidth(this->minimumWidth());
	synthesisFrame->setMaximumSize(options->sizeHint());

	layout->addWidget(synthesisFrame);

	directSynthesize = new QPushButton("Synthesize..");
	layout->addWidget(directSynthesize);

	buttonTriangulate = new QPushButton("Merge and Output..");
	layout->addWidget(buttonTriangulate);

	// RESULTS =====================================================
	resultsWidget = new QDockWidget("Results", this);
	resultsFrame = new QFrame(this);
	resultsLayout = new QGridLayout();

	outputImage = new QLabel(" ", this);

	row = 0;
	resultsLayout->addWidget(outputImage, row, 0);
	resultsLayout->addWidget(csExtendedImage = new QLabel("CS_EXTEND"), row++, 2);
	//resultsLayout->addWidget(lsExtendedImage = new QLabel("Ls_EXTEND"));

	resultsFrame->setLayout(resultsLayout);
	resultsFrame->setFrameStyle(QFrame::StyledPanel);

	resultsFrame->setLayout(resultsLayout);
	resultsWidget->setWidget(resultsFrame);

	layout->addWidget(resultsWidget);

	// Register types =====================================================
	qRegisterMetaType<Displacements*>("Displacements*");
	qRegisterMetaType<Grid*>("Grid*");

	// Signals & actions
	QObject::connect(buttonSmoothing, SIGNAL(clicked()), this, SLOT(DoSmoothing()));
	QObject::connect(buttonCreateDF, SIGNAL(clicked()), this, SLOT(CreateDF()));
	QObject::connect(directSynthesize, SIGNAL(clicked()), this, SLOT(DirectSynthesize()));
	QObject::connect(this, SIGNAL(GridChanged(Grid*)), gv, SLOT(SetGrid(Grid*)));
	QObject::connect(smooth_level, SIGNAL(sliderMoved(int)), gv, SLOT(SetLevel(int)));
	QObject::connect(buttonTogglePatch,  SIGNAL(clicked()), gv, SLOT(ToggleShowPatch()));
	QObject::connect(buttonTriangulate, SIGNAL(clicked()), this, SLOT(Triangulate()));

	this->setLayout(layout);

	df = NULL;

	firstGridMesh = false;
}

void DisplacementsWidget::DoSmoothing()
{
	// UI stuff
	QApplication::setOverrideCursor(Qt::WaitCursor);
	Print("Smoothing..", 500);

	printf("\n\nInitializing Displacement Field... (grid size :%d)\n", grid_square_size->value());
	
	workingMesh = this->sourceMesh;

	if(df) delete df;

	stats["smoothing"] = Stats("Base extraction (smoothing)");

	df = new Displacements(workingMesh, this->skeleton, num_smooth_steps->value(), 
		num_iteration->value(), step_size->value(), volume_preserve->isChecked());

	stats["smoothing"].end();

	UpdateDF(df);

	smooth_level->setRange(0, df->Stair()->numberOfSteps() - 1);

	gv->doneCurrent();

	// UI stuff
	QApplication::restoreOverrideCursor();
}

void DisplacementsWidget::CreateDF()
{
	int numSteps = num_smooth_steps->value();
	int numIter = num_iteration->value();
	double stepSize = step_size->value();

	// Check if smoothing not done or changed, if so re-smooth
	if(!df || numSteps != prevSmooth.numSteps || numIter != prevSmooth.numIter 
		|| stepSize != prevSmooth.stepSize)
	{
		DoSmoothing();

		prevSmooth.numSteps = numSteps;
		prevSmooth.stepSize = stepSize;
		prevSmooth.numIter = numIter;
	}

	if(skeleton->sortedSelectedNodes.size() < 2)
		return;

	skeleton->isVisible = false;

	mainWindow->ui.viewer->setViewMode(MODIFY);

	// Select type of fitting
	int fitMethod = 3; // no fitting

	if(fitting_method->currentText() == "Cross section")		fitMethod = 1;
	else if(fitting_method->currentText() == "Simple Cylinder")	fitMethod = 2;

	Print("Computing parameterization..");

	// Compute the displacement representation
	df->computeField(grid_square_size->value(), fitMethod, rotate_left->value(), rotate_right->value());

	isMeshCut = false;

	df->setVisibility(true);
	UpdateDF(df);

	// Cross sections
	MatrixXf cs_pattern = fromVector2Df(df->GetGrid()->sectionsPattern());
	crossSectionImage->setPixmap( pixmapFromMatrix(cs_pattern) );

	// Large scale pattern
	MatrixXf ls_pattern = fromVector2Df(df->GetGrid()->largeScalePattern());
	largeScalePatternImage->setPixmap( pixmapFromMatrix(ls_pattern, true) );

	GridChanged(df->GetGrid());

	firstGridMesh = true;

	Print("Ready to stretch.");
}

void DisplacementsWidget::DirectSynthesize()
{
	Print("Synthesize extension..");
	int allStartTime = clock();

	int startTime = clock();
        printf("\nTexture synthesis (band size %d)", band_size->value());

	df->setVisibility(false);
	user_curve->isVisible = false;

	MatrixXf src = gv->getSquaresValues();
	
	float bottomPadding = 0.5f;

	// Get full grid as matrix
	/*MatrixXf fullGrid = MatrixXf::Zero(src.rows() + (src.rows() * bottomPadding), src.cols());
	fullGrid.block(0,0,src.rows(),src.cols()) = src;
	fullGrid.block(src.rows(), 0, fullGrid.rows() - src.rows(), src.cols()) = src.block(0,0,fullGrid.rows() - src.rows(), src.cols());
			
	fullGrid -= MatrixXf::Constant(fullGrid.rows(), fullGrid.cols(), df->Grid()->min_height[0]);

	fullGrid.array() *= 255;*/

	MatrixXf fullGrid = src;

	// Crop unused parts
	Rect cropArea = gv->Crop();
	//Rect cropArea(0, fullGrid.cols(), 0, fullGrid.rows());

	// Auto-overlap size
	if(band_size->value() == 0)
		band_size->setValue(Max(3, cropArea.width() * 0.20)); // %20 of width

	printf("(band = %d px).", band_size->value());

	// hack for special cases (sheet)
	if (cropArea.r - cropArea.l < band_size->value())
		cropArea = Rect(0, fullGrid.cols(), 0, fullGrid.rows());

	MatrixXf grid = fullGrid.block(0, cropArea.l, fullGrid.rows(), cropArea.width());

	// Find expected extension length
	Vector<Vec> path = user_curve->getPath( df->GetGrid()->segmentLength );
	int expectedExtension = path.size() - 1;

	stats["textureSynthesis"] = Stats("Texture Synthesis");

	TextureSynthesizer ts;
	Vector2DPoint textureSynthResult;

	// Synthesize using selected synthesizer
	if(synthesisType->currentText() == "Patch-Based")
	{
		textureSynthResult = ts.synthesizeAsPos(grid, (src.rows() * bottomPadding),
			expectedExtension, block_size->value(), band_size->value());
	}
	else if(synthesisType->currentText() == "Nearest-neighbor interpolation")
	{
		textureSynthResult = ts.resampleAsPos(grid, expectedExtension);
	}
	else if(synthesisType->currentText() == "Tiling")
	{
		textureSynthResult = ts.tileAsPos(grid, expectedExtension, band_size->value());
	}

	stats["textureSynthesis"].end();

	// Timing for Texture synthesis
        printf(".Done (%d ms)", (int)clock() - startTime);
	startTime = clock();
	printf("\n\n\n==========\nCreate extension..\n");

	int midSkeleIndex = skeleton->originalSelectedEdges.size() / 2;
	SkeletonEdge * midSkeleton = skeleton->originalSelectedEdges[midSkeleIndex];

	//Vector<int> halfMesh = skeleton->Split(midSkeleton->index).second; // second half

	// Make sure viewer is the current OpenGL
	mainWindow->ui.viewer->makeCurrent();

	// Delete previous synthesis if any
	if(firstGridMesh)
		firstGridMesh = false;
	else
		if(gmesh.size()) gmesh.pop_back();

	// Blend cross-sections experiment
	if(blendCrossSections->isChecked())
	{
		df->GetGrid()->item_int["tileCount"] = ts.tileCount;
	}

	// Create extension part
	gmesh.push_back(GridMesh(df->GetGrid(), cropArea, user_curve->getSpline(), 
		textureSynthResult, 
		synthCrossSections->isChecked(), blendCrossSections->isChecked(), 
		band_size->value(), changeProfile->isChecked(), 
		fillSeams->isChecked(), blendSeams->isChecked(), sampleSeams->isChecked()));

	// cleaner code
	GridMesh * gm = &gmesh.back();
	
	// Show cross-section and large scale detail synthesized result
	csExtendedImage->setPixmap( pixmapFromMatrix(gm->extendedSections) );
	//lsExtendedImage->setPixmap( QPixmap::fromImage(imageFromMatrix(resizeAsImage(ls_pattern, ls_pattern.cols() * 5), true)) );

	// Debug
	gm->testPoints.push_back(midSkeleton->n1->v());
	gm->testPoints.push_back(midSkeleton->n2->v());

	// Now modify mesh
	Mesh * m = getMesh("LoadedMesh");

	// Timing
        printf("\n\nDone (%d ms)\n==========\n\n", (int)clock() - startTime);

        startTime = (int)clock();
	printf("Slicing and moving..");

	if(!isMeshCut)
	{
		startTime = clock();

		// slice the mesh
		Plane slicePlane = df->GetGrid()->getMidPlane();
                Vector<int> selectedFaces = skeleton->getSelectedFaces();
                sliceOp = Slicer::SliceAt(m, selectedFaces, slicePlane);
		
		// Make sure all half of the mesh is selected
		newPoints = sliceOp.newPoints.ToVector();
		cutPoints = sliceOp.cutPoints.ToVector();

		StdSet<int> halfMeshSet;
		foreach(int vi, cutPoints)
		{
			// If a cut point is not in the last connected part, make it a seed and add part
			if(halfMeshSet.find(vi) == halfMeshSet.end())
			{
				StdSet<int> part = m->getConnectedPart(vi);
				halfMeshSet.insert(part.begin(), part.end());
			}
		}

		halfMesh = SET_TO_VECTOR(halfMeshSet);
		
		//DEBUG by coloring
		//m->setColor(0,255,0); m->setColor(halfMesh, 255,0,0);

		resetPosition = m->getCopyPoints();

		isMeshCut = true;
	}
	else
	{
		// undo previous changes
		m->setMeshPoints(resetPosition);
	}

	// Transform half mesh
	Transform3D::transformVertices(m, halfMesh, gm->endTransform());

        printf("done slice/move (%d ms).", (int)clock() - startTime);
	startTime = clock();
	printf(".Remaining..");

	// Set render options
	//m->setColorFaces (skeleton->getSelectedFaces(2, 2), 1, 1, 1, 0);
	//m->isDrawAsPoints = true;
	//m->isDrawSmooth = false;

	m->computeNormals();
	m->computeBounds();

	mainWindow->ui.viewer->setSceneRadius(m->radius);
	mainWindow->ui.viewer->update();

        printf("Synthesize extension Done. (%d ms)", (int)clock() - startTime);

	QString timeAsString; timeAsString.sprintf("Synthesis Done. (%.2f s)", (clock() - allStartTime) / 1000.0f);
	Print(timeAsString, 1000);

	stats["extendAmount"] = Stats("Amount of extension", 
		100.0 * ((float)textureSynthResult[0].size() / grid.cols()));

	// Preview texture synthesis result
	outputImage->setPixmap(QPixmap::fromImage(ts.imageOutput()));

	// Output debug images:
	QImage q (imageFromMatrix(grid));
	q.save("_input_texture.bmp");
	outputImage->pixmap()->save("_output_texture.bmp");
	tempImage = q;

	crossSectionImage->pixmap()->save("_input_cross-section.bmp");
	csExtendedImage->pixmap()->save("_output_cross-section.bmp");
}

void DisplacementsWidget::Triangulate()
{
	int startTime = clock();
	printf("\nMerging & Outputting...");

	// Get the mesh
	Mesh * mesh = getMesh("LoadedMesh");

	// Get the cut points
	GridMesh * gm = &gmesh.back();

	StdSet<int> firstCut = gm->firstCut; 
	StdSet<int> lastCut = gm->lastCut;

	// Get original patch
	StdSet<int> visitedA, visitedB; 

	visitedA = mesh->visitFromBoundry(newPoints.front(), firstCut);
	visitedB = mesh->visitFromBoundry(cutPoints.front(), lastCut);

	// Find detach faces (A)
	StdSet<Face *> detachFacesA_original;
	StdSet<Face *> fromBorderFacesA = mesh->getFacesFromVertices(visitedA);
	foreach(Face * f, mesh->getFacesFromVertices(firstCut)){
		if(fromBorderFacesA.find(f) != fromBorderFacesA.end())
			detachFacesA_original.insert(f);
	}
	
	// Add faces with all vertices being on cut (A)
	StdSet<Face *> cutNeighbourhoodA;
	foreach(int vi, firstCut) foreach(Face * f, mesh->vd(vi)->ifaces) cutNeighbourhoodA.insert(f);
	foreach(Face * f, cutNeighbourhoodA){
		if(SET_HAS(firstCut, f->vIndex[0]) && SET_HAS(firstCut, f->vIndex[1]) 
			&& SET_HAS(firstCut, f->vIndex[2]))
			detachFacesA_original.insert(f);
	}

	// Find detach faces (B)
	StdSet<Face *> detachFacesB_original;
	StdSet<Face *> fromBorderFacesB = mesh->getFacesFromVertices(visitedB);
	foreach(Face * f, mesh->getFacesFromVertices(lastCut)){
		if(fromBorderFacesB.find(f) != fromBorderFacesB.end())
			detachFacesB_original.insert(f);
	}

	// Add faces with all vertices being on cut (B)
	StdSet<Face *> cutNeighbourhoodB;
	foreach(int vi, lastCut) foreach(Face * f, mesh->vd(vi)->ifaces) cutNeighbourhoodB.insert(f);
	foreach(Face * f, cutNeighbourhoodB){
		if(SET_HAS(lastCut, f->vIndex[0]) && SET_HAS(lastCut, f->vIndex[1]) 
			&& SET_HAS(lastCut, f->vIndex[2]))
			detachFacesB_original.insert(f);
	}

	// DEBUG:
	/*StdSet<Face*> green, yellow;
	foreach(Face * f, detachFacesA_original) green.insert(f);
	foreach(Face * f, detachFacesB_original) yellow.insert(f);
	mesh->greenFaces.push_back(green);
	mesh->yellowFaces.push_back(yellow);*/

	mainWindow->ui.viewer->update();

	// Create copy
	Mesh outMesh = *mesh;
	int offset = outMesh.numberOfVertices();

	// Get synthesized parts
	Mesh * firstPart = &gm->triangluatedExtension;

	// Merge it with extension (simple merge)
	outMesh.mergeWith(*firstPart);
	outMesh.reassignFaces();
	outMesh.getUmbrellas();

	Vector<int> firstCutVector = SET_TO_VECTOR(firstCut);
	Vector<int> lastCutVector = SET_TO_VECTOR(lastCut);

	int indexOnStart = firstPart->vertexIndexClosest(mesh->vec(firstCutVector[firstCutVector.size() / 2]));
	int indexOnEnd = firstPart->vertexIndexClosest(mesh->vec(lastCutVector[lastCutVector.size() / 2]));

	// Get new boundary of extension
	StdList<int> extensionStart = outMesh.getBoundry(offset + indexOnStart);
	StdList<int> extensionEnd = outMesh.getBoundry(offset + indexOnEnd);

	printf("extent start size = %d, end = %d\n", extensionStart.size(), extensionEnd.size());

	// Find attach faces (from new extension)
	StdSet<Face *> attachFacesA = outMesh.getFacesFromVertices(LIST_TO_SET(extensionStart));
	StdSet<Face *> attachFacesB = outMesh.getFacesFromVertices(LIST_TO_SET(extensionEnd));

	StdSet<Face *> detachFacesA, detachFacesB;
	foreach(Face * src_f, detachFacesA_original) detachFacesA.insert(outMesh.f(src_f->index));
	foreach(Face * src_f, detachFacesB_original) detachFacesB.insert(outMesh.f(src_f->index));

	HashMap<int, int> replacment;

	// First cut replacements
	foreach(int i, firstCut){
		float minDist = FLT_MAX;
		int closest = 0;
		foreach(int j, extensionStart){
			float dist = (outMesh.vec(i) - outMesh.vec(j)).norm();
			if(dist < minDist){
				minDist = dist;
				closest = j;
			}
		}
		replacment[closest] = i;
	}

	// Last cut replacements
	foreach(int i, lastCut){
		float minDist = FLT_MAX;
		int closest = 0;
		foreach(int j, extensionEnd){
			float dist = (outMesh.vec(i) - outMesh.vec(j)).norm();
			if(dist < minDist){
				minDist = dist;
				closest = j;
			}
		}
		replacment[closest] = i;
	}

	// Do vertex index replacements
	for(HashMap<int,int>::iterator it = replacment.begin(); it != replacment.end(); it++)
	{
		int oldPoint = it->second;
		int newPoint = it->first;

		foreach(Face * f, attachFacesA)		f->replacePoint(newPoint, oldPoint);
		foreach(Face * f, attachFacesB)		f->replacePoint(newPoint, oldPoint);

		foreach(Face * f, detachFacesA)		f->replacePoint(oldPoint, -1);
		foreach(Face * f, detachFacesB)		f->replacePoint(oldPoint, -1);

		outMesh.vd(oldPoint)->unsetAndMark();
	}

	StdSet<int> removeSet;

	foreach(Face *f, fromBorderFacesA) removeSet.insert(f->index);
	foreach(Face *f, fromBorderFacesB) removeSet.insert(f->index);
	foreach(Face *f, detachFacesA) removeSet.insert(f->index);
	foreach(Face *f, detachFacesB) removeSet.insert(f->index);

	outMesh.removeAllFaces(removeSet);
	
	QString fileName = QFileDialog::getSaveFileName(this, tr("Output Mesh"), "", tr("OBJ (*.obj)"));

	outMesh.saveToFile(fileName.toAscii());

	// get rid of .OBJ
	fileName.chop(4);

	// output images too
	tempImage.save((fileName + "_input_texture.bmp").toAscii());
	outputImage->pixmap()->save((fileName + "_output_texture.bmp").toAscii());
	crossSectionImage->pixmap()->save((fileName + "_input_cross-section.bmp").toAscii());
	csExtendedImage->pixmap()->save((fileName + "_output_cross-section.bmp").toAscii());

	saveStatsToFile(fileName);

	mainWindow->ui.viewer->saveSnapshot(fileName + "_screenshot.bmp", true);

        printf("\n\nOutput Done. (%d ms)\n=========\n", (int)clock() - startTime);
}

void DisplacementsWidget::Print(QString message, int age)
{
	mainWindow->ui.viewer->print(message, age);

	qApp->processEvents();
}

// GETTERS
int DisplacementsWidget::getNumSmoothSteps(){return num_smooth_steps->value();}
int DisplacementsWidget::getNumSmoothIter(){return num_iteration->value();}
int DisplacementsWidget::getGridSize(){return grid_square_size->value();}
int DisplacementsWidget::getBandSize(){	return band_size->value();}
float DisplacementsWidget::getSmoothStepSize(){	return step_size->value();}

// SETTERS
void DisplacementsWidget::setNumSmoothSteps(int numSteps){num_smooth_steps->setValue(numSteps);}
void DisplacementsWidget::setNumSmoothIter(int numIter){num_iteration->setValue(numIter);}
void DisplacementsWidget::setBandSize(int bsize){	band_size->setValue(bsize);	}
void DisplacementsWidget::setSmoothStepSize(float smoothStep){	step_size->setValue(smoothStep);}
void DisplacementsWidget::setGridSize(int gridResolution){ grid_square_size->setValue(gridResolution);}
