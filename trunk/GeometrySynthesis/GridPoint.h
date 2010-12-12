#pragma once

class GridPoint
{
private:
	
public:
	double w[4];
	float h;
	double shiftAngle;
	float shiftMagnitude;
	int corrPoint;

	// Assign weights such as the point is on one of the corners
	GridPoint(int corner)
	{
		w[corner % 4] = 1.0;
		w[(corner+1) % 4] = w[(corner+2) % 4] = w[(corner+3) % 4] = 0;

		h = 0.0;
		shiftAngle = shiftMagnitude = 0;
		corrPoint = -1;
	}

	GridPoint(const GridPoint& fromPoint) 
	{
		w[0] = fromPoint.w[0];
		w[1] = fromPoint.w[1];
		w[2] = fromPoint.w[2];
		w[3] = fromPoint.w[3];

		h = fromPoint.h;

		shiftAngle = fromPoint.shiftAngle;
		shiftMagnitude = fromPoint.shiftMagnitude;

		corrPoint = fromPoint.corrPoint;
	}

	GridPoint& operator= (const GridPoint& fromPoint) 
	{
		w[0] = fromPoint.w[0];
		w[1] = fromPoint.w[1];
		w[2] = fromPoint.w[2];
		w[3] = fromPoint.w[3];

		h = fromPoint.h;
		
		shiftAngle = fromPoint.shiftAngle;
		shiftMagnitude = fromPoint.shiftMagnitude;

		corrPoint = fromPoint.corrPoint;

		return *this;
	}

	GridPoint(double weights[], float height, double shift_angle, float shift_magnitude, int correspondingPoint = -1)
	{
		w[0] = weights[0];
		w[1] = weights[1];
		w[2] = weights[2];
		w[3] = weights[3];

		h = height;
		
		shiftAngle = shift_angle;
		shiftMagnitude = shift_magnitude;

		corrPoint = correspondingPoint;
	}

	static GridPoint MidPoint()
	{
		double weights[4] = {0.25, 0.25, 0.25, 0.25};
		return GridPoint(weights, 0, 0, 0, 0);
	}

	static GridPoint FirstPoint()
	{
		double weights[4] = {1.0, 0, 0, 0};
		return GridPoint(weights, 0, 0, 0, 0);
	}
};
