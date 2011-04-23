#include "ExtendMeshHeaders.h"
#include "Viewer.h"

#include "Smoother.h"

Viewer::Viewer(QWidget *parent) : QGLViewer(parent)
{
	renderStyle = 1;

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(dequeueLastMessage()));
}

Viewer::~Viewer()
{

}

void Viewer::init()
{
	GLfloat lightColor[] = {0.8f, 0.8f, 0.8f, 1.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);

	// Material
	//float no_mat[] = {0.0f, 0.0f, 0.0f, 1.0f};
	//float mat_ambient_color[] = {0.25f, 0.25f, 0.25f, 1.0f};
	float mat_ambient[] = {0.1745f, 0.01175f, 0.01175f, 1.0f};
	float mat_diffuse[] = {	0.65f, 0.045f, 0.045f, 1.0f};
	float mat_specular[] = {0.09f, 0.09f, 0.09f, 1.0f};
	float high_shininess = 100;

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, high_shininess);

	this->setBackgroundColor(backColor = QColor(50,50,60));
	//this->setBackgroundColor(backColor = QColor(255,255,255));  // white

	this->viewMode = VIEW;
	this->selectMode = NONE;
	this->modifyMode = DEFAULT;

	mouseAltPressed = false;

	//this->setMouseTracking(true);
}

void Viewer::draw()
{
	GLfloat ambientLight[] = {(GLfloat)(0.15f * backColor.redF()),
		(GLfloat)(0.15f * backColor.greenF()),
		(GLfloat)(0.15f * backColor.blueF()), 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

	this->setBackgroundColor(backColor);

	// Anti-aliasing
	glEnable(GL_MULTISAMPLE);

	// Draw Grid Meshes
	for(StdList<GridMesh>::iterator g = gmesh.begin(); g != gmesh.end(); g++)
	{
		g->draw();
	}

	// Draw Displacement Field
	if(df != NULL)	df->draw();

	foreach(Mesh * m, meshes)
	{
		if(m->isReady)
			m->draw();
	}

	// Draw User curve
	if(user_curve && user_curve->isReady)
		user_curve->draw();

	// Draw Skeleton
	if(skeleton.isReady && skeleton.isVisible)
		skeleton.draw(false);

	// Textual log messages
	for(int i = 0; i < messages.size(); i++)
	{
		int margin = 20;//px
		int x = margin;
		int y = (i * QFont().pointSize() * 1.5f) + margin;

		qglColor(Qt::white);
		renderText(x, y, messages.at(i));
	}

	// DEBUG: draw test points, lines, and planes
	glColor3f(1,0,0);	// RED
	for(Vector<Line>::iterator it = testLines1.begin(); it != testLines1.end(); it++)	it->draw();
	for(Vector<Plane>::iterator it = testPlanes1.begin(); it != testPlanes1.end(); it++)	it->draw();
	SimpleDraw::IdentifyPoints(testPoints1, 1,0,0);

	glColor3f(0,1,0);	// GREEN
	for(Vector<Line>::iterator it = testLines2.begin(); it != testLines2.end(); it++)	it->draw();
	for(Vector<Plane>::iterator it = testPlanes2.begin(); it != testPlanes2.end(); it++)	it->draw();
	SimpleDraw::IdentifyPoints(testPoints2, 0,1,0);

	glColor3f(0,0,1);	// BLUE
	for(Vector<Line>::iterator it = testLines3.begin(); it != testLines3.end(); it++)	it->draw();
	for(Vector<Plane>::iterator it = testPlanes3.begin(); it != testPlanes3.end(); it++)	it->draw();
	SimpleDraw::IdentifyPoints(testPoints3, 0,0,1);

	// DEBUG: 3d floating text
	glDisable(GL_LIGHTING);
	glColor3f(1,1,1);	// WHITE
	for (StdMap<int,Vec>::iterator it = testText1.begin(); it != testText1.end(); it++)
	{
		Vec proj = camera()->projectedCoordinatesOf(it->second);
		drawText(proj.x, proj.y, QString::number(it->first));
	}
	glColor3f(1,1,0);	// YELLOW
	for (StdMap<int,Vec>::iterator it = testText2.begin(); it != testText2.end(); it++)
	{
		Vec proj = camera()->projectedCoordinatesOf(it->second);
		drawText(proj.x, proj.y, QString::number(it->first));
	}
	glEnable(GL_LIGHTING);
}

void Viewer::drawWithNames()
{
	setSelectRegionWidth(20);
	setSelectRegionHeight(20);

	switch(selectMode)
	{
	case MESH:
		{
			Mesh * loadedMesh = getMesh("LoadedMesh");
			if(loadedMesh)	loadedMesh->drawFaceNames();
		}
		break;

	case VERTEX:
		{
			Mesh * loadedMesh = getMesh("LoadedMesh");
			if(loadedMesh)	loadedMesh->drawVertexNames();
		}
		break;

	case SKELETON_NODE:
		if(skeleton.isReady)
			skeleton.drawNodesNames();
		break;

	case SKELETON_FACES:
		if(skeleton.isReady)
			skeleton.drawMeshFacesNames();
		break;

	case RECONSTRUCTED_POINTS:
		if(df != NULL && df->isReady)
			df->drawPointNames();
		break;
	case NONE:
	case SKELETON_EDGE:
		break;
	}
}

void Viewer::mousePressEvent(QMouseEvent* e)
{
	switch(viewMode)
	{
	case VIEW:
		break;

	case SELECTION:
		break;

	case MODIFY:
		if(e->button() == Qt::LeftButton && (e->modifiers() == Qt::AltModifier))
		{
			mouseAltPressed = true;

			user_curve->setPlane(skeleton.selectedEdges[0]->n1->v(), -camera()->viewDirection());

			update();
		}
		break;
	}

	// Regular behaivor
	QGLViewer::mousePressEvent(e);
}

void Viewer::mouseMoveEvent(QMouseEvent* e)
{
	switch(viewMode)
	{
	case VIEW:
		break;

	case SELECTION:
		break;

	case MODIFY:
		if(mouseAltPressed && !user_curve->isSimplified)
		{
			Vec origin, direction;

			camera()->convertClickToLine(e->pos(), origin, direction);

			user_curve->addPoint(origin, direction);

			update();
		}
		break;
	}

	QGLViewer::mouseMoveEvent(e);
}

void Viewer::mouseReleaseEvent(QMouseEvent* e)
{
	switch(viewMode)
	{
	case VIEW:
		break;

	case SELECTION:
		break;

	case MODIFY:
		if(mouseAltPressed && user_curve->isReady && !user_curve->isSimplified)
		{
			mouseAltPressed = false;
			user_curve->simplify();
			update();

			if(df_widget) df_widget->DirectSynthesize();
		}
		break;
	}

	// Regular behavior
	QGLViewer::mouseReleaseEvent(e);
}

void Viewer::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_L)
	{

	}

	// Clear all debug items
	if(e->key() == Qt::Key_C)
	{
		testPoints1.clear(); testPoints2.clear(); testPoints3.clear();
		testLines1.clear(); testLines2.clear(); testLines3.clear();
		testPlanes1.clear(); testPlanes2.clear(); testPlanes3.clear();
	}

	// Save / restore viewing states
	if(e->key() == Qt::Key_F5)		saveStateToFile();
	else if(e->key() == Qt::Key_F8)	restoreStateFromFile();

	// Print last statistics
	if(e->key() == Qt::Key_P) 
	{
		printStats();
		saveStatsToFile();
	}

	updateGL();

	// Regular behavior
	QGLViewer::keyPressEvent(e);
}

void Viewer::postSelection(const QPoint& point)
{
	int selected = selectedName();

	switch(selectMode)
	{
	case MESH:
		{
			Mesh * loadedMesh = getMesh("LoadedMesh");
			if(loadedMesh){
				if(loadedMesh->selectedFace != selected)
					printf("Face : (%d) - Selected.\n", selected);

				loadedMesh->selectedFace = selected;
				if(selected >= 0)
				{
					Face * f = loadedMesh->f(selected);
					bool found;
					Vec p = camera()->pointUnderPixel(point, found);

					float minDist = FLT_MAX;

					for(int i = 0; i < 3; i++){
						float dist = (p - f->vec(i)).norm();
						if(dist < minDist){
							minDist = dist;
							loadedMesh->selectedVertex = f->VIndex(i);
						}
					}

					printf("Selected vertex (%d) - IFaces count (%d) \n\t", loadedMesh->selectedVertex,
						loadedMesh->vd(loadedMesh->selectedVertex)->ifaces.size());
					foreach(Face * f, loadedMesh->vd(loadedMesh->selectedVertex)->ifaces)
						printf("\n  FACE (%d) -> {%d,%d,%d}", f->index, f->VIndex(0), f->VIndex(1), f->VIndex(2));
					printf("\n\n");
				}
			}
		}
		break;

	case VERTEX:
		{
			Mesh * loadedMesh = getMesh("LoadedMesh");
			if(loadedMesh){
				if(selected >= 0){
					loadedMesh->selectedVertices.insert(selected);
					printf("Selected vertex (%d)\n", selected);
				} else {
					loadedMesh->selectedVertices.set.clear();
					printf("Clear vertex selection.\n");
				}
			}
		}
		break;

	case SKELETON_NODE:
		if(selected > -1)
		{
			skeleton.isVisible = true;
			skeleton.selectNode(selected);
			skeleton.selectEdges(skeleton.selectNodeStart, skeleton.selectNodeEnd);

			if(skeleton.selectNodeEnd != -1 && skeleton.selectedEdges.size())
				df_widget->CreateDF();
		}
		else
			skeleton.deselectAll();
		break;

	case SKELETON_FACES:
		if(selected > -1)
		{
			skeleton.isVisible = true;
			skeleton.selectNodeFromFace(selected);
			skeleton.selectEdges(skeleton.selectNodeStart, skeleton.selectNodeEnd);

			if(skeleton.selectNodeEnd != -1)
			{
				update();
				parentWidget()->setCursor(Qt::WaitCursor);

				// Select inner part of partial skeleton (help avoid boundary problems)
				skeleton.cropSelectedEdges();

				if(skeleton.selectedEdges.size())
					df_widget->CreateDF();

				parentWidget()->setCursor(Qt::ArrowCursor);
			}
		}
		else
			skeleton.deselectAll();
		break;

	case RECONSTRUCTED_POINTS:
		df->GetGrid()->selectPoint(selected);
		break;

	case NONE:
		break;

	case SKELETON_EDGE:
		break;
	}

	update();
}

void Viewer::setViewMode(ViewMode toMode)
{
	this->viewMode = toMode;
}

void Viewer::setSelectMode(SelectMode toMode)
{
	this->selectMode = toMode;
}

void Viewer::setModifyMode(ModifyMode toMode)
{
	this->modifyMode = toMode;
}

void Viewer::print(QString message, long age)
{
	messages.enqueue(message);

	timer->start(age);

	update();
}

void Viewer::dequeueLastMessage()
{
	if(!messages.isEmpty())
	{
		messages.dequeue();
		update();
	}
}

void Viewer::askUserBackground()
{
	backColor = QColorDialog::getColor(backColor);

	update();
}

void Viewer::changeRenderStyle()
{
	camera()->setType(qglviewer::Camera::ORTHOGRAPHIC);

	camera()->setOrientation(0,0);
	camera()->setPosition(Vec(-2,2,2));
	camera()->lookAt(Vec());
	camera()->setPosition(camera()->position() + Vec(0,1,0));

	update();
}
