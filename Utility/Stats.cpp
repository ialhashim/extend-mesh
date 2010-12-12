#include "Stats.h"

#include "ExtendMeshHeaders.h"

Stats::Stats( QString stat_label)
{
	this->label = stat_label;

	startTime = 0;
	time = 0;

	noTime = false;

	start();
}

Stats::Stats()
{
	this->label = "NOTHING";
	startTime = 0;
	time = 0;
	noTime = false;
}

Stats::Stats( QString stat_label, double newValue )
{
	this->label = stat_label;

	startTime = 0;
	time = 0;

	noTime = true;

	value = newValue;
}

void Stats::start()
{
	this->startTime = clock();
}

void Stats::end()
{
	this->time = clock() - this->startTime;
}

void Stats::print()
{
	if(noTime)
		printf(" :: %s \t = |  %f  |\n", label.toStdString().c_str(), this->value);
	else
		printf(" :: %s \t time = | %d  | ms\n", label.toStdString().c_str(), (int)time);
}

void Stats::addValue( double newValue )
{
	this->value = newValue;
	this->noTime = true;
}

double Stats::getValue()
{
	if(noTime)
		return value;
	else
		return (double) time;
}

QString Stats::getLabel()
{
	return label;
}

bool Stats::isTimeBased()
{
	return !noTime;
}
