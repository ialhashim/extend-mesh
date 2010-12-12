#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QMouseEvent>

class SideBar : public QWidget
{
	Q_OBJECT

private:
	QBoxLayout * layout;

protected:
	void mouseReleaseEvent(QMouseEvent * e);

public:
	SideBar(QWidget *parent = 0);

public slots:
	void displayWidget(QWidget *widget);
	void removeAllWidgets();

};

#endif // SIDEBAR_H
