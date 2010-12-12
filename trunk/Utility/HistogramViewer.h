#ifndef HISTOGRAM_VIEWER_H
#define HISTOGRAM_VIEWER_H

#include <QWidget>
#include <QPen>
#include <QColor>

#include "Histogram.h"

template <typename T>
class HistogramViewer : public QWidget
{
private:
	Histogram<T> * histogram;

public:
	HistogramViewer(Histogram<T> * histogram, QWidget * parent = 0) : QWidget(parent)
	{
		this->histogram = histogram;
		this->setWindowTitle("Histogram");
		this->setMinimumSize(200, 200);
		this->resize(400,300);
		this->setWindowFlags(Qt::Tool);
		this->show();
	}

	virtual void paintEvent(QPaintEvent *event)
	{
		QPainter p(this);

		p.fillRect(this->rect(), QColor::fromRgb(255,255,255));

		QRect graphRect = this->rect();
		graphRect.adjust(40, 5, 5, -40);

		p.drawRect(graphRect);

		int binWidth = graphRect.width() / histogram->NumberOfBins();
		int maxBinCount = histogram->MaximumBinCount();

		QPen pen(QBrush(QColor::fromRgb(33, 109, 148)), 1, Qt::SolidLine);
		p.setPen(pen);

		for(int i = 0; i < histogram->NumberOfBins(); i++)
		{
			int binHeight = ((float)(histogram->BinCount(i) / (float)maxBinCount) * graphRect.height());

			QRect bin = QRect((i * binWidth) + graphRect.left(), 
				graphRect.bottom() - binHeight, binWidth, binHeight);

			p.fillRect(bin, QColor::fromRgb(195, 217, 255));
			p.drawRect(bin);
		}
	}
};

#endif // HISTOGRAM_VIEWER_H
