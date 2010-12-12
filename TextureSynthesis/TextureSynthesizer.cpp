#include "TextureSynthesizer.h"

#include "../Utility/Matrixf.h"

using namespace Synth;

TextureSynthesizer::TextureSynthesizer(QImage * input, int bottomPadding)
{
	qRegisterMetaType<MatrixXf>("MatrixXf");

	this->input = Matrixf::matrixFromImage(input);
	this->s = new Synthesizer(bottomPadding);
}

TextureSynthesizer::TextureSynthesizer(const MatrixXf & input, int bottomPadding)
{
	qRegisterMetaType<MatrixXf>("MatrixXf");

	this->input = input;
	this->s = new Synthesizer(bottomPadding);
}

void TextureSynthesizer::init(MatrixXf source, int synthWidth, int blockSize, int bandSize)
{
	this->input = source;

	s->init(source, synthWidth, blockSize, bandSize);

	connect(this->s, SIGNAL(print(QString)), this, SLOT(Print(QString)));
	connect(this->s, SIGNAL(view(MatrixXf)), this, SLOT(View(MatrixXf)));
}	

void TextureSynthesizer::run()
{
	int startTime = clock();

	s->start();

	// update progress
	while(!s->isDone)
	{
		update(QPixmap::fromImage(Matrixf::imageFromMatrix(s->result())));
	}

	// Crop unused parts
	s->crop();

	// Show final result
	update(QPixmap::fromImage(Matrixf::imageFromMatrix(s->result())));
	print("Done! (" + QString::number(clock() - startTime) + QString(" ms)"));
	
	// save it as our output
	this->output = s->result();
}

Vector<Vector<Point> > TextureSynthesizer::synthesizeAsPos(const MatrixXf & src, int pad, int width, int block, int band)
{
	this->input = src;

	s = new Synthesizer(pad);
	s->init(src, width, block, band);

	s->synthesizeAll();

	s->crop();

	this->output = s->result();

	return s->resultAsPos();
}

Vector<Vector<Point> > TextureSynthesizer::outputAsPos()
{
	return s->resultAsPos();
}

Vector<Vector<Point> > TextureSynthesizer::tileAsPos(const MatrixXf & src, int width, int band)
{
	this->input = src;
	tileCount = ceil((float)width / src.cols());

	MatrixXi pos = Tiler(src, band, tileCount).tileAsPos(); // synthesize
	MatrixXi indices = NumberedMatrix(src.rows(), src.cols());
	
	// Create an integer map : 1->(0,0) ... etc
	StdMap< int, Point > indexMap;
	for(int y = 0; y < indices.rows(); y++)
	{
		for(int x = 0; x < indices.cols(); x++)
			indexMap[indices.coeffRef(y,x)] = Point(x,y);
	}

	// Convert to our 2D grid of Points format
        Vector<Vector<Point> > tempAsPos = Vector<Vector<Point> >(pos.rows(), Vector<Point>(pos.cols(), Point(-1,-1)));
	for(int y = 0; y < pos.rows(); y++)
	{
		for(int x = 0; x < pos.cols(); x++)
			tempAsPos[y][x] = indexMap[pos(y,x)];
	}

	this->output = FromPositionMatrix(src, pos);

	return tempAsPos;
}

Vector<Vector<Point> > TextureSynthesizer::resampleAsPos(const MatrixXf & src, int width) 
{
	this->input = src;
	
	StdMap< int, Point > indexMap;

	MatrixXi indices = NumberedMatrix(src.rows(), src.cols());
	MatrixXi pos = Matrixf::resizeBasic(indices.cast<float>(), width).cast<int>(); // Nearest-Neighbour

	for(int y = 0; y < indices.rows(); y++)
	{
		for(int x = 0; x < indices.cols(); x++)
			indexMap[indices.coeffRef(y,x)] = Point(x,y);
	}
        Vector<Vector<Point> > tempAsPos = Vector<Vector<Point> >(pos.rows(), Vector<Point>(pos.cols(), Point(-1,-1)));
	for(int y = 0; y < pos.rows(); y++)
	{
		for(int x = 0; x < pos.cols(); x++)
			tempAsPos[y][x] = indexMap[(int)pos(y,x)];
	}
	this->output = FromPositionMatrix(src, pos);
	return tempAsPos;
}

void TextureSynthesizer::Print(QString message)
{
	print(message);
}

void TextureSynthesizer::View(MatrixXf m)
{
	view(QPixmap::fromImage(Matrixf::imageFromMatrix(m)));
}

void TextureSynthesizer::setOutput(MatrixXf newOutput)
{
	this->output = newOutput;
}

float TextureSynthesizer::extendRatio()
{
	return (float)this->output.cols() / (float)this->input.cols();
}

QImage TextureSynthesizer::imageInput()
{
	return Matrixf::imageFromMatrix(this->input);
}

QImage TextureSynthesizer::imageOutput()
{
	return Matrixf::imageFromMatrix(this->output);
}

MatrixXf TextureSynthesizer::imageInputMatrix()
{
	return this->input;
}

MatrixXf TextureSynthesizer::imageOutputMatrix()
{
	return this->output;
}
