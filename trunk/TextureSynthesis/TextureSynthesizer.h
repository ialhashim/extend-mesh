#ifndef TEXTURESYNTHESIZER_H
#define TEXTURESYNTHESIZER_H
#include <QMetaType>

#include <QThread>
#include <QImage>
#include <QPixmap>
#include <QColor>

#include "Synthesizer.h"
#include "Tiler.h"

namespace Synth
{
	class TextureSynthesizer : public QThread
	{
		Q_OBJECT

		private:
			MatrixXf input;
			MatrixXf output;

			Synthesizer * s;

		protected:
			void run();

		public:
			TextureSynthesizer(){ qRegisterMetaType<MatrixXf>("MatrixXf"); }
			TextureSynthesizer(QImage * inputImage, int bottomPadding);
			TextureSynthesizer(const MatrixXf & inputImage, int bottomPadding);

			void init(MatrixXf source, int synthWidth, int blockSize, int bandSize);

			void setOutput(MatrixXf newOutput);

			QImage imageInput();
			QImage imageOutput();

			MatrixXf imageInputMatrix();
			MatrixXf imageOutputMatrix();

			// Using patch-based texture synthesis
			Vector<Vector<Point> > synthesizeAsPos(const MatrixXf & src, int pad, int width, int block, int band);
			Vector<Vector<Point> > outputAsPos();

			// Using smart tiling
			Vector<Vector<Point> > tileAsPos(const MatrixXf & src, int width, int band);

			int tileCount;

			// Basic re-sampling
			Vector<Vector<Point> > resampleAsPos(const MatrixXf & src, int width);

			// No re-sampling
			Vector<Vector<Point> > asSource(const MatrixXf & src);

			float extendRatio();

		public slots:
			void Print(QString);
			void View(MatrixXf);

		signals:
			void print(QString);
			void update(QPixmap);
			void view(QPixmap);
	};

}

#endif
