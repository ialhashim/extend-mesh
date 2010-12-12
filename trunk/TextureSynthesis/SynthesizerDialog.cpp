#include "SynthesizerDialog.h"
#include "TextureSynthesizer.h"

SynthesizerDialog::SynthesizerDialog(TextureSynthesizer * texSynth, QWidget * parent) : QDialog(parent)
{
	this->resize(800,500);

	// Assign our pointer to the texture synthesizer
	this->ts = texSynth;

	// Dialog structure
	options = new QFrame(this);
	display = new QFrame(this);
	layout = new QBoxLayout(QBoxLayout::TopToBottom, this);

	optionsLayout = new QGridLayout();
	displayLayout = new QGridLayout();

	statusBar = new QStatusBar(this);
	statusBar->showMessage("(please click synthesize)");

	// Options items
	blockSize = new QSpinBox();
	bandSize = new QSpinBox();
	targetWidth = new QSpinBox();
	synthesizeButton = new QPushButton("Synthesize...");

	blockSize->setMinimum(7);
	bandSize->setMinimum(3);
	targetWidth->setMinimum(4);

	blockSize->setMaximum(512);
	bandSize->setMaximum(255);
	targetWidth->setMaximum(4096);

	blockSize->setSingleStep(20);
	bandSize->setSingleStep(10);
	targetWidth->setSingleStep(40);

	blockSize->setValue(40);
	bandSize->setValue(10);
	targetWidth->setValue(320);

	blockSize->setSuffix(" px");
	bandSize->setSuffix(" px");
	targetWidth->setSuffix(" px");

	int r = 0;

	optionsLayout->addWidget(new QLabel("Block Size"), r, 0);
	optionsLayout->addWidget(blockSize, r++, 1);

	optionsLayout->addWidget(new QLabel("Band Size"), r, 0);
	optionsLayout->addWidget(bandSize, r++, 1);

	optionsLayout->addWidget(new QLabel("Target width"), r, 0);
	optionsLayout->addWidget(targetWidth, r++, 1);

	optionsLayout->addWidget(synthesizeButton, r++, 6);

	// Display items
	inputImage = new QLabel("input", this);
	outputImage = new QLabel(" ", this);

	inputImage->setPixmap(QPixmap::fromImage(ts->imageInput()));

	displayLayout->addWidget(inputImage);
	displayLayout->addWidget(outputImage);

	// Add to dialog
	layout->addLayout(optionsLayout);
	layout->addLayout(displayLayout);
	layout->addWidget(statusBar);
	this->setLayout(layout);

	// Connections
	QObject::connect(synthesizeButton, SIGNAL(clicked()), this, SLOT(synthesize()));

	// Update display connections
	QObject::connect(synthesizeButton, SIGNAL(clicked()), this, SLOT(updateDisplay()));
	QObject::connect(blockSize, SIGNAL(valueChanged(int)), this, SLOT(updateDisplay()));
	QObject::connect(bandSize, SIGNAL(valueChanged(int)), this, SLOT(updateDisplay()));
	QObject::connect(targetWidth, SIGNAL(valueChanged(int)), this, SLOT(updateDisplay()));

	updateDisplay();

	// Connect status bar and output update events
	connect(this->ts, SIGNAL(print(QString)), this, SLOT(Print(QString)));
	connect(this->ts, SIGNAL(update(QPixmap)), this, SLOT(UpdateOutput(QPixmap)));
}

void SynthesizerDialog::showDialog(TextureSynthesizer * texSynth, QWidget * parent, float enlargeFactor)
{
	SynthesizerDialog * synthDialog = new SynthesizerDialog(texSynth, parent);

	synthDialog->targetWidth->setValue(texSynth->imageInputMatrix().cols() * enlargeFactor);

	synthDialog->exec();
}

void SynthesizerDialog::synthesize()
{
	// Bound check
	if(bandSize->value() > blockSize->value() / 2)
		bandSize->setValue((blockSize->value() / 2) - 1);

	MatrixXf img = ts->imageInputMatrix();

	// Prepare then synthesize extension!!!
	ts->init(img, targetWidth->value(), blockSize->value(), bandSize->value());
	ts->start();
}

void SynthesizerDialog::Print(QString message)
{
	this->statusBar->showMessage(message);
}

void SynthesizerDialog::UpdateOutput(QPixmap pixmap)
{
	this->outputImage->setPixmap(pixmap);
}

void SynthesizerDialog::updateDisplay()
{
	QImage inputImageModified = QImage(ts->imageInput());

	QPainter painter;

	painter.begin(&inputImageModified);
	
	painter.setRenderHint(QPainter::Antialiasing);

	int margin_x = 5;
	int margin_y = 5;

	int bandWidth = blockSize->value() - bandSize->value();
	int blockWidth = blockSize->value();

	QRect bandBox = QRect(margin_x + bandSize->value(), margin_y + bandSize->value(), bandWidth, bandWidth) ;

	QRect blockBox = QRect(margin_x, margin_y, blockWidth, blockWidth);
	QRect blockBoxShadow = QRect(margin_x+2, margin_y+2, blockWidth, blockWidth);

	// shadow :)
	painter.setPen(QPen(QBrush(QColor::fromRgb(0,0,0,100)), 4));
	painter.drawRect(blockBoxShadow);

	// Band box
	painter.setPen(QPen(QBrush(QColor::fromRgb(114,207,63)), 1));
	painter.drawRect(bandBox);
	
	// Block box
	painter.setPen(QPen(QBrush(QColor::fromRgb(133,255,0)), 2));
	painter.drawRect(blockBox);

	painter.end();

	inputImage->setPixmap(QPixmap::fromImage(inputImageModified));
}
