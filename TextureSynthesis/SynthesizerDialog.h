#ifndef SYNTHESIZER_DIALOG_H 
#define SYNTHESIZER_DIALOG_H

#include <QDialog>
#include <QFrame>
#include <QGridLayout>

#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QLabel>
#include <QPainter>
#include <QStatusBar>

#include "Synthesizer.h"
#include "TextureSynthesizer.h"

using namespace Synth;

class SynthesizerDialog : public QDialog
{
	Q_OBJECT

private:
	TextureSynthesizer * ts;

	QBoxLayout * layout;
	QStatusBar * statusBar;

	QFrame * options;
	QGridLayout * optionsLayout;
	QSpinBox * blockSize;
	QSpinBox * bandSize;
	QSpinBox * targetWidth;
	QPushButton * synthesizeButton;

	QFrame * display;
	QGridLayout * displayLayout;
	QLabel * inputImage;
	QLabel * outputImage;

public:
	SynthesizerDialog(TextureSynthesizer * texSynth, QWidget * parent = 0);

	static void showDialog(TextureSynthesizer * texSynth, QWidget * parent, float enlargeFactor);

public slots:
	void synthesize();
	void updateDisplay();

	void Print(QString message);
	void UpdateOutput(QPixmap pixmap);
};

#endif // SYNTHESIZER_DIALOG_H
