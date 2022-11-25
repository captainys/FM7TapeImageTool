#ifndef YSWAVEEDIT_ENVELOPEUTIL_IS_INCLUDED
#define YSWAVEEDIT_ENVELOPEUTIL_IS_INCLUDED
/* { */


#include "yswavekernel.h"

class YsWave_EnvelopeUtil
{
protected:
	class Sample
	{
	public:
		int ptr;
		int level;
	};

public:
	std::vector <YsWaveKernel::Envelope> CalculateEnvelope(
		const YsSoundPlayer::SoundData &wav,
		const std::vector <YsWaveKernel::Peak> &peak,
		const std::vector <YsWaveKernel::Region> &silentRegion,
		int channel);
};



/* } */
#endif
