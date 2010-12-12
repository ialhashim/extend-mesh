#include "ExtendMeshHeaders.h"
#include "SideBar.h"

SideBar::SideBar(QWidget *parent) : QWidget(parent)
{
	layout = new QBoxLayout(QBoxLayout::TopToBottom);
	layout->setMargin(6);

	this->setLayout(layout);

	this->setMouseTracking(true);
}

void SideBar::displayWidget(QWidget *widget)
{
	removeAllWidgets();

	if(widget)
	{
		// add the new widget
		layout->addWidget(widget);

		this->parentWidget()->setWindowTitle(widget->windowTitle());
	}
}

void SideBar::mouseReleaseEvent(QMouseEvent * e)
{
	mainWindow->ui.viewer->update();

        QWidget::mouseReleaseEvent(e);
}

void SideBar::removeAllWidgets()
{
	QLayoutItem *child;

	while ((child = layout->takeAt(0)) != 0) {
		delete child->widget();
		delete child;
	}

	layout->update();

	this->parentWidget()->setWindowTitle("");
	this->parentWidget()->setMinimumSize(1,1);
	this->parentWidget()->update();
}
