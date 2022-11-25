#ifndef YSWAVEEDIT_PEAKUTIL_IS_INCLUDED
#define YSWAVEEDIT_PEAKUTIL_IS_INCLUDED
/* { */



#include "yswavekernel.h"

class YsWave_PeakUtil
{
public:
	std::vector <YsWaveKernel::Peak> Detect(const YsWaveKernel &wav,int channel);
};


/* } */
#endif
