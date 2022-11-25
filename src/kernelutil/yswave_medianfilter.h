#ifndef YSWAVE_MEDIANFILTER_IS_INCLUDED
#define YSWAVE_MEDIANFILTER_IS_INCLUDED
/* { */



#include "yswavekernel.h"

class YsWave_MedianFilter
{
public:
	void Apply(YsSoundPlayer::SoundData &wav,int channel);
};



/* } */
#endif
