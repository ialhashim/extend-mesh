#include "ExtendMeshHeaders.h"
#include "Commander.h"

Commander::Commander(QWidget *parent) : QWidget(parent)
{
	// Nothing to show for..
	this->hide();
}

Commander::~Commander()
{
	
}

void Commander::OpenMesh()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Mesh"), "", tr("Mesh Files (*.obj)"));

	if(fileName.length())
	{
		parentWidget()->setCursor(Qt::WaitCursor);

		// Clear old data
		meshes.clear();
		gmesh.clear();

		// Clear user curve
		if(user_curve) delete user_curve;
		user_curve = new UserCurve();

		// Load new mesh
		Mesh * m = newMesh("LoadedMesh");

		m->loadFromFile(fileName.toAscii().data());

		// Center camera on object
		viewer->setSceneCenter(m->center.vec());
		viewer->setSceneRadius(m->radius);
		viewer->camera()->showEntireScene();
		viewer->update();

		// clear Side bar
		mainWindow->ui.sideBar->removeAllWidgets();

		loadedFileName = fileName;
		loadedFileName.chop(4);

		// Load skeleton
		QString skeletonFile = loadedFileName + ".skel.cg";
		QString corrFile = skeletonFile + ".txt";
		OpenSkeleton(skeletonFile, corrFile);

		// If a skeleton exists then compute displacements
		if(skeleton.nodes.size() > 0)
		{
			// Create displacement widget and add to the sidebar
			df_widget = new DisplacementsWidget(m, &skeleton, mainWindow->ui.sideBar);
			df = df_widget->df;

			QObject::connect(df_widget, SIGNAL(UpdateDF(Displacements*)), 
				this, SLOT(UpdateDisplacements(Displacements*)));

			mainWindow->ui.sideBar->displayWidget(df_widget);

			viewer->setSelectMode(SKELETON_FACES);

			// try and load a script if any exists
			LoadScript();
		}

		parentWidget()->setCursor(Qt::ArrowCursor);
	}
}

void Commander::OpenSkeleton(QString fileName, QString corrFileName)
{
	if(fileName.length())
	{
		skeleton.loadFromFile(fileName.toAscii().data());
	
		// Load corr file
		if(fileName.length())
		{
			Mesh * loadedMesh = getMesh("LoadedMesh");

			if(loadedMesh)
			{
				skeleton.loadCorrespondence(corrFileName.toAscii().data());
				skeleton.embed(loadedMesh);
				skeleton.calculateEdgesLengths();

				// default behavior
                viewer->setViewMode(SELECTION);
				viewer->setSelectMode(SKELETON_FACES);
			}
			else
			{
				printf("\n ERROR: make sure you loaded a mesh.\n");
			}
		}
	}
}

void Commander::SelectModeReconstructedPoints()
{
	viewer->setSelectMode(RECONSTRUCTED_POINTS);
}

void Commander::ShowHideSkeleton()
{
	skeleton.isVisible = !skeleton.isVisible;

	viewer->selectMode = SKELETON_NODE;
}

void Commander::ToggleSkelUserFriendly()
{
	if(skeleton.isUserFriendly)
		skeleton.isUserFriendly = false;
	else
		skeleton.isUserFriendly = true;
}

void Commander::ToggleCameraProjection()
{
	if(viewer->camera()->type() == qglviewer::Camera::PERSPECTIVE)
		viewer->camera()->setType(qglviewer::Camera::ORTHOGRAPHIC);
	else
		viewer->camera()->setType(qglviewer::Camera::PERSPECTIVE);
}

void Commander::ChangeBackgroundColor()
{
	viewer->makeCurrent();
	viewer->askUserBackground();
}

void Commander::ChangeRenderStyle()
{
	viewer->makeCurrent();
	viewer->changeRenderStyle();
}

void Commander::ToggleVisible()
{
	Mesh * mesh = getMesh("LoadedMesh");

	if(mesh) mesh->isVisible = !mesh->isVisible;
}

void Commander::ToggleWireframe()
{
	Mesh * mesh = getMesh("LoadedMesh");

	if(mesh) mesh->isDrawWireframe = !mesh->isDrawWireframe;
}

void Commander::ToggleShowAsNormalizedPoints()
{
	Mesh * mesh = getMesh("LoadedMesh");

	if(mesh)
	{
		if(mesh->isDrawAsPoints)
		{
			mesh->isDrawAsPoints = false;
			mesh->isDrawSmooth = true;
		}
		else
		{
			mesh->isDrawAsPoints = true;
			mesh->isDrawSmooth = false;
		}
	}
}

void Commander::ToggleShowNormals()
{
	Mesh * mesh = getMesh("LoadedMesh");

	if(mesh)
	{
		if(mesh->isShowVertexNormals)
		{
			mesh->isShowVertexNormals = false;
			mesh->isShowFaceNormals = false;
		}
		else
		{
			mesh->isShowVertexNormals = true;
			mesh->isShowFaceNormals = true;
		}
	}
}

void Commander::SmoothRegular()
{
	Mesh * loadedMesh = getMesh("LoadedMesh");
	if(loadedMesh)	Smoother::LaplacianSmoothing( loadedMesh, 5 );
}

void Commander::SmoothCurvature()
{
	Mesh * loadedMesh = getMesh("LoadedMesh");
	if(loadedMesh)	Smoother::MeanCurvatureFlow( loadedMesh, 0.001, 1 );
}

void Commander::UpdateDisplacements(Displacements* newDF)
{
	df = newDF;
	viewer->update();
}

void Commander::ToggleShowHideAllDF()
{
	if(df)	df->ToggleVisibility();
}

void Commander::ToggleShowDisplacements()
{
	if(df)	df->ToggleDisplacmentsVisibility();
}

void Commander::ToggleShowCrossSections()
{
	for(StdList<GridMesh>::iterator g = gmesh.begin(); g != gmesh.end(); g++)
	{
		g->isDrawCrossSections = !g->isDrawCrossSections;
		g->showHideTriangulated();
	}
}

void Commander::ToggleShowGridMesh()
{
	for(StdList<GridMesh>::iterator g = gmesh.begin(); g != gmesh.end(); g++)
		g->showHideTriangulated();
}

void Commander::ToggleWireframeGridMesh()
{
	for(StdList<GridMesh>::iterator g = gmesh.begin(); g != gmesh.end(); g++)
		g->ToggleWireframe();
}

void Commander::ToggleShowColoredPatches()
{
	for(StdList<GridMesh>::iterator g = gmesh.begin(); g != gmesh.end(); g++)
		g->ToggleShowColoredPatches();
}

void Commander::InfoMesh()
{
	viewer->selectMode = MESH;

	printf("\nSelecting mesh faces.\n");
}

void Commander::SaveMesh()
{
	Mesh * loadedMesh = getMesh("LoadedMesh");

	if(loadedMesh)
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save Mesh"), "", tr("OBJ File (*.obj)"));

		loadedMesh->saveToFile(fileName.toAscii());
	}
}

void Commander::OutputGridMesh()
{
	if(gmesh.size())
	{
		gmesh.back().outputGridMesh();
	}

	if(df)
	{
		Grid * grid = df->GetGrid();

		grid->saveToFile("output_grid.obj");
		grid->getBaseMesh()->saveToFile("output_base.obj");
		grid->getDetailedMesh()->saveToFile("output_detailed.obj");
	}

	Mesh * loadedMesh = getMesh("LoadedMesh");

	if(loadedMesh)
	{
		if(skeleton.selectedNodes.size())
		{
			Vector<int> selectedFaces = skeleton.getSelectedFaces(true);

			Mesh * selected = loadedMesh->CloneSubMesh(selectedFaces);

			selected->saveToFile("output_selectedFaces.obj");
		}
	}
}

void Commander::SaveScript()
{
	FILE *fp = fopen((loadedFileName + ".script").toAscii(),"w");

	if(fp == NULL)	return;

	// Selected mesh part (as skeleton nodes start, finish)
	fprintf(fp, "%d,%d\n", skeleton.selectNodeStart, skeleton.selectNodeEnd);

	// Smoothing Options
	fprintf(fp, "%d,%d,%f\n", df_widget->getNumSmoothSteps(), df_widget->getNumSmoothIter(), 
		df_widget->getSmoothStepSize());

	// Grid & Band size
	fprintf(fp, "%d,%d\n", df_widget->getGridSize(), df_widget->getBandSize());

	// User curve
	if(user_curve)
	{
		Vector<Vec> curve = user_curve->spline.GetPoints();
		foreach(Vec v, curve)
		{
			fprintf(fp, "%f %f %f\n", v.x, v.y, v.z);
		}
	}

	fclose(fp);
}

void Commander::LoadScript()
{
	FileStream file ((loadedFileName + ".script").toAscii());
	std::string inputLine;

	if(!file.is_open())	return;

        CreateTimer(timer);
	printf("\n=========\nExecuting script file..\n");

	// Selection
	int start, end;
	GetLine (file, inputLine);
	sscanf(inputLine.c_str(),"%d,%d", &start, &end);

	// Smoothing Options
	int numSteps, numIter;
	float stepSize;
	GetLine (file, inputLine);
	sscanf(inputLine.c_str(),"%d,%d,%f", &numSteps, &numIter, &stepSize);

	df_widget->setNumSmoothSteps(numSteps);
	df_widget->setNumSmoothIter(numIter);
	df_widget->setSmoothStepSize(stepSize);

	// Band size
	GetLine (file, inputLine);
	int squareSize,bandSize;
	sscanf(inputLine.c_str(),"%d,%d", &squareSize, &bandSize);

	df_widget->setGridSize(squareSize);
	df_widget->setBandSize(bandSize);

	// User curve
	while(!file.eof())
	{
		GetLine (file, inputLine);
		if(inputLine.length() > 5)
		{
			Vec v;
                        sscanf(inputLine.c_str(),"%lf %lf %lf", &v.x, &v.y, &v.z);
			user_curve->spline.AddSplinePoint(v);
		}
	}

	file.close();

	// Apply script
	skeleton.selectNode(start);
	skeleton.selectNode(end);
	skeleton.selectEdges(skeleton.selectNodeStart, skeleton.selectNodeEnd);

	// for creating VBOs, make sure viewer is current OpenGL context
	viewer->makeCurrent();

	df_widget->CreateDF();

	if(user_curve->spline.GetNumPoints() > 0)
	{
		df_widget->DirectSynthesize();
	}

        printf("\n\nScript file done. (%d ms)\n=========\n", (int)timer.elapsed());
}

void Commander::LoadUserCurve()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Mesh"), "", tr("Curve Files (*.curve)"));

	if(fileName.length())
	{
		FileStream file (fileName.toAscii());
		std::string inputLine;

		if(!file.is_open())	return;

		if(user_curve) delete user_curve;
		user_curve = new UserCurve();

		while(!file.eof())
		{
			GetLine (file, inputLine);
			if(inputLine.length() > 5)
			{
				Vec v;
                                sscanf(inputLine.c_str(),"%lf %lf %lf", &v.x, &v.y, &v.z);
				user_curve->spline.AddSplinePoint(v);
			}
		}

		file.close();

		viewer->makeCurrent();

		if(user_curve->spline.GetNumPoints() > 3)
		{
			user_curve->simplify2();

			df_widget->DirectSynthesize();
		}
	}
}
