#pragma once

#include "ui_extendMesh.h"

class ExtendMesh : public QMainWindow
{
	Q_OBJECT

public:
	ExtendMesh(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ExtendMesh();

//private: not really..
	Ui::ExtendMeshClass ui;
};
