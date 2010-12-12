#pragma once

#include <vector>
#include <limits>

#undef max
#undef min

template <typename T>
class Histogram
{
private:
	int numBins;

	T minimum;
	T maximum;
	T average;
	T sum;

	T binSize;
	int largestBinSize;

	std::vector<T> rawData;
	std::vector<std::vector<T> > histogram;

	bool isDataCollectionDone;

public:
	Histogram(int numberOfBins)
	{
		this->numBins = numberOfBins;

                this->histogram = std::vector<std::vector<T> > (this->numBins);

		this->minimum = std::numeric_limits<T>::max();
		this->maximum = std::numeric_limits<T>::min();
		this->sum = 0;
		this->average = 0;

		isDataCollectionDone = false;
	}

	void begin(int expectedCount)
	{
		this->rawData.clear();
		this->rawData.reserve(expectedCount);
	}

	void insert(T data)
	{
		if(!isDataCollectionDone)
		{
			this->rawData.push_back(data);
			
			if(data >= this->maximum)
				this->maximum = data;

			if(data <= this->minimum)
				this->minimum = data;

			this->sum += data;
		}
	}

	void end()
	{
		this->average = this->sum / rawData.size();

		if(this->maximum >= this->minimum)
			this->binSize = (maximum - minimum) / numBins;
		else
			this->binSize = -(minimum - maximum) / numBins;

                for(int i = 0; i < (int)rawData.size(); i++)
		{
			int bin = ((rawData[i] - minimum) / binSize) - 1;
			
			if(bin < 0)	bin = 0;

			if(bin >= 0 && bin <= numBins)
				this->histogram[bin].push_back(rawData[i]);
			else
				printf("ERROR: possibly corrupted histogram!");
		}

		// Check histogram integrity
		int sum = 0;
		largestBinSize = 0;

		for(int i = 0; i < numBins; i++)
		{
			int size = this->histogram[i].size();
			sum += size;

			if(size > largestBinSize)
				largestBinSize = size;
		}
		
                if(sum != (int)rawData.size())
			printf("ERROR: missing data. Considred (%d) elements from (%d)", sum, rawData.size());

		isDataCollectionDone = true;
	}

	T Maximum() { if(rawData.size() == 0) return 0; else return this->maximum; }
	T Minimum() { if(rawData.size() == 0) return 0; else return this->minimum; }
	T Average() { if(rawData.size() == 0) return 0; else return this->average; }

	T RangeSize()	{ if(rawData.size() == 0) return 0; else return maximum - minimum; }

	int NumberOfBins() { return this->numBins; }
	int BinCount(int bin) { return this->histogram[bin].size(); }
	int MaximumBinCount() { return this->largestBinSize; }

};

typedef Histogram<int> HistogramInt;
typedef Histogram<float> HistogramFloat;
typedef Histogram<double> HistogramDouble;
