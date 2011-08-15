#include "Matrixf.h"

#include "Macros.h"
#include "TextureSynthesis/Globals.h"

#include "NoiseGen.h"

namespace Matrixf
{
	MatrixXf matrixFromImage(QImage * img)
	{
		MatrixXf m = MatrixXf::Zero(img->height(), img->width());

		for(int y = 0; y < img->height(); y++)
		{
			for(int x = 0; x < img->width(); x++)
			{
				QColor c = QColor::fromRgb(img->pixel(x,y));

				m(y,x) = c.red();
			}
		}

		return m;
	}

	QImage imageFromMatrix(const MatrixXf & m, bool isUseFalseColors)
	{
		QImage image = QImage(m.cols(), m.rows(), QImage::Format_RGB32);

		image.fill(0);

		float color = 0;

		float min_value = m.array().minCoeff();
		float max_value = m.array().maxCoeff();
		float range = max_value - min_value;

		Vector<QColor> pColors = pseudoColors();

		// Create target image
		for(int y = 0; y < m.rows(); y++){
			for(int x = 0; x < m.cols(); x++){
				//if(color > 255) color = 255;
				//RGBi c = RGBScale::toRGBi(color);
				//image.setPixel(x, y, qRgb(c.r, c.g, c.b));

				color = m(y,x);

				int value = 0;

				if(range)
					value = ((color - min_value) / range) * 255.0;

				if(!isUseFalseColors)
					image.setPixel(x,y, qRgb(value, value, value));
				else
					image.setPixel(x,y, pColors[value].rgb());
			}
		}

		return image;
	}

	QPixmap pixmapFromMatrix(const MatrixXf & m, bool isUseFalseColors)
	{
		return QPixmap::fromImage(imageFromMatrix(m, isUseFalseColors));
	}

	QImage imageFromPosMatrix(QImage & src, MatrixXi pos)
	{
		QImage img = QImage(pos.cols(), pos.rows(), QImage::Format_RGB32);

		Vector<QPoint> point;
		for(int x = 0; x < src.width(); x++){
			for(int y = 0; y < src.height(); y++){
				point.push_back(QPoint(x,y));
			}
		}

		QRgb * pixel;
		QPoint src_point;

		// Create target image
		for(int y = 0; y < pos.rows(); y++){
			pixel = (QRgb *)img.scanLine(y);

			for(int x = 0; x < pos.cols(); x++){
				src_point = point[ pos(y,x) - 1 ];
				*(pixel + x) = *(((QRgb *)src.scanLine(src_point.y())) + src_point.x());
			}
		}

		return img;
	}

	float getBilinear(const MatrixXf & src, float dX, float dY)
	{
		int iX = (int)dX;
		int iY = (int)dY;

		float tX = dX - iX;
		float tY = dY - iY;

		int max_w = src.cols() - 1;
		int max_h = src.rows() - 1;

		float p[2][2];
		p[0][0] = src(    iY			,     iX);				// a
		p[0][1] = src(Min(iY+1, max_h)	,     iX);				// c
		p[1][0] = src(    iY			, Min(iX+1, max_w));	// b
		p[1][1] = src(Min(iY+1, max_h)	, Min(iX+1, max_w));	// d
		

		float sum = 0;
		sum += p[0][0] * (1-tX) * (1-tY);
		sum += p[1][0] * (  tX) * (1-tY);
		sum += p[0][1] * (1-tX) * (  tY);
		sum += p[1][1] * (  tX) * (  tY);
		return sum;
	}

	float getNearestNeighbour(const MatrixXf & src, float dX, float dY)
	{
		int iX = (int)dX;
		int iY = (int)dY;

		int max_w = src.cols() - 1;
		int max_h = src.rows() - 1;

		return src(Min(iY, max_h), Min(iX, max_w));
	}

	MatrixXf resizeAsImage(const Vector2Df & v, int newWidth, bool isWithBias)
	{
		return resizeAsImage(fromVector2Df(v), newWidth, isWithBias);
	}

	double stretchFunction(double x, double src_width, double width, double alpha)
	{
		double ratio = width / src_width;

		if(ratio <= 1.0) 
			return x;

		double w = 1.0 - ratio;
		double r = x * ratio;

		double startRatio = alpha / ratio;
		double endRatio = 1.0 - startRatio;

		if(x < startRatio)	return r;
		if(x > endRatio)	return r + w;

		double srcRange = 0.5 - startRatio;
		double newRange = 0.5 - alpha;
		double midRatio = newRange / srcRange;

		if(x < 0.5)
		{
			double z = (x - startRatio) * midRatio;
			return z + alpha;
		}
		else
		{
			double z = (x - 0.5) * midRatio;
			return z + 0.5;
		}

		return x;
	}

	Vector<double> stretchFunctionVec(double src_width, double width, double alpha)
	{
		Vector<double> result;

		for(int i = 0; i < width; i++)
			result.push_back( stretchFunction((double)i / width, src_width, width, alpha) );

		return result;
	}

	MatrixXf resizeAsImage(const MatrixXf & src, int new_width, bool isWithBias)
	{
		MatrixXf result = MatrixXf::Zero(src.rows(), new_width);

		float width = src.cols();
		float height = src.rows();

		float newWidth = new_width;

		float wMult = width / newWidth;
		float hMult = height / height;

		float dX, dY;

		double f0 = bias(0);
		double f1 = bias(1);
		double fN = f1 - f0;

		Vector<double> stretchBias = stretchFunctionVec(width, newWidth, 0.20);

		for(int y = 0; y < height; y++)
		{
			for(float x = 0; x < newWidth; x++)
			{
				dX = x * wMult;
				dY = y * hMult;

				// useful?
				if(isWithBias) 
				{
					//double X = (bias(x / newWidth) - f0) / fN;

					dX = RANGED(0, stretchBias[x], 1) * width;
				}

				dX = RANGED(0.0, dX, newWidth - 1);

				result(y,x) = getBilinear(src, dX, dY);
			}
		}

		return result;
	}

	Vector<QColor> pseudoColors()
	{
		Vector<QColor> lutData(256);
                for (int i = 0; i < (int)lutData.size(); i++)
		{
			double r, g, b;
			if (i >= 0 && i <= 63){
				r = 0;
				g = 255.0 / 63 * i;
				b = 255;
			}else if (i > 63 && i <= 127){
				r = 0;
				g = 255;
				b = 255 - (255.0 / (127 - 63) * (i - 63));
			}else if (i > 127 && i <= 191){
				r = 255.0 / (191 - 127) * (i - 127);
				g = 255;
				b = 0;
			}else // if (i > 191 && i < 256)
			{
				r = 255;
				g = 255 - (255.0 / (255 - 191) * (i - 191));
				b = 0;
			}
			lutData[i] = QColor((int)r, (int)g, (int)b);
		}
		return lutData;
	}

	MatrixXf resizeBasic(const MatrixXf & src, int newWidth, int newHeight)
	{
		int height = src.rows();
		int width = src.cols();

		if(newHeight <= 0)
			newHeight = height;

		MatrixXf result = MatrixXf::Zero(newHeight, newWidth);

		float wMult = (float)width / newWidth;
		float hMult = (float)height / newHeight;

		for(int y = 0; y < newHeight; y++)
		{
			for(int x = 0; x < newWidth; x++)
			{
				result(y,x) = getNearestNeighbour(src, x * wMult, y * hMult);
			}
		}

		return result;
	}

	MatrixXf fromVector2Df(const Vector2Df& data)
	{
		MatrixXf m(data.size(), data[0].size());

		for(int u = 0; u < m.rows(); u++)
		{
			for(int v = 0; v < m.cols(); v++)
				m(u,v) = data[u][v];
		}

		return m;
	}

	Vector<float> column(int c, const MatrixXf& M)
	{
		VectorXf col = M.col(c);
		Vector<float> result(col.size());

		for(int i = 0; i < col.size(); i++)
			result[i] = col(i);

		return result;
	}

	MatrixXf synthFromMap(MatrixXf& src_cs, const Vector2DPoint& synth_pattern)
	{
		MatrixXf result = MatrixXf::Zero(src_cs.rows(), synth_pattern[0].size() + 1);

		for(int v = 0; v < result.cols() - 1; v++)
		{
			for(int u = 0; u < result.rows() - 1; u++)
			{
				int x = synth_pattern[u][v].x;
				int y = synth_pattern[u][v].y;

				// Left
				result(u,   v) = src_cs(y   , x);
				result(u+1, v) = src_cs(y+1 , x);

				// Right
				result(u   , v + 1) = src_cs(y   , x+1);
				result(u+1 , v + 1) = src_cs(y+1 , x+1);
			}
		}

		return result;
	}

	MatrixXf tileBlendFromMap(MatrixXf& src_cs, int bandSize, int tileCount)
	{
		float minVal = src_cs.array().minCoeff();

		int src_w = src_cs.cols();
		int h = src_cs.rows();

		// Weights
		MatrixXf wL, wR;

		wL = MatrixXf::Zero(h, bandSize);
		for(int y = 0; y < h; y++) for(int x = 0; x < bandSize; x++) wL(y, x) += x;	wL /= wL.array().maxCoeff();
		wR = MatrixXf::Ones(h, bandSize) - wL;

		MatrixXf result = MatrixXf::Constant(h, src_w * (tileCount + 2), minVal);

		// First source
		result.block(0, 0, h, src_w) = fromLeft(src_cs, src_w);

		int delta = Max(1, src_w - bandSize);
		int c = delta;

		for(int i = 0; i < tileCount + 2; i++)
		{
			MatrixXf A = fromRight(src_cs, bandSize).array() * wR.array();
			MatrixXf B = fromLeft(src_cs, bandSize).array() * wL.array();

			MatrixXf patch = src_cs;

			patch.block(0, 0, h, bandSize) = A + B;

			result.block(0, c, h, src_w) = patch;	

			c += delta;
		}

		return result;
	}

	Vector2Df toVector2Df( const MatrixXf& m )
	{
		Vector2Df result;

		for(int u = 0; u < m.rows(); u++)
		{
			result.push_back(Vector<float>(m.cols()));

			for(int v = 0; v < m.cols(); v++)
				result[u][v] = m(u,v);
		}

		return result;
	}

	void outputToFile( const MatrixXf& m, QString fileName )
	{
		FILE *fp = fopen(fileName.toAscii(), "w");

		if(fp == NULL)	return;

		for(int y = 0; y < m.rows(); y++)
		{
			for(int x = 0; x < m.cols(); x++)
			{
				fprintf(fp, "%.2f ", m(y,x));
			}

			fprintf(fp, "\n");
		}

		fclose(fp);
	}

	void outputAsImage( const MatrixXf& m, QString fileName, bool isUseFalseColors )
	{
		imageFromMatrix(m, isUseFalseColors).save(fileName);
	}

	MatrixXf pattern( int width, int height, QString kind, float scale )
	{
                if(kind == "smooth-random")	return NoiseGen::smoothNoise(width, height, scale);
                else if(kind == "wave")		return NoiseGen::wavePattern(width, height, scale, true);
		
		// just return something..
		return NoiseGen::noise(width, height);
	}

	double bias( double x )
	{
		double y = 0.0;
		double t = 0.5;

		// magic
		y = x - (sqrt((double) M_PI) / 2.0) * t * erf( ((sqrt(2.0) / 2.0) * (x - 0.5)) / t);

		return y;
	}

	void rotate( MatrixXf& m, float amount, bool isLeft /*= true*/ )
	{
		if(amount == 0) return;

		float cols = m.cols();
		int rows = m.rows();

		float delta = 0;

		for(float x = 0; x < cols; x++)
		{
			if(isLeft)
				delta = Round(amount * ((cols - x) / cols));
			else
				delta = Round(amount * ((x + 1) / cols));

			VectorXf newCol(rows);

			for(int y = 0; y < rows; y++)
			{
				int u = x;
				int v = Mod(y + (int)delta, rows);
				
				newCol(y) = m(v,u); 

				if(x == 0)
					printf("\n%d", v);
			}

			m.col(x) = newCol;
		}
	}

}
