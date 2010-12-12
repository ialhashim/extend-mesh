#pragma once

#include <QString>
#include <stdio.h>
#include <time.h>

class Stats
{
private:
	QString label;
	
	clock_t startTime;
	clock_t time;

	bool noTime;
	double value;

public:
	Stats();
        Stats(QString stat_label);
	Stats(QString stat_label, double newValue);

	void start();
	void end();

	void addValue(double newValue);

	void print();

	double getValue();
	QString getLabel();
	bool isTimeBased();
};
