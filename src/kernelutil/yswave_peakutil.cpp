#include "yswave_peakutil.h"



std::vector <YsWaveKernel::Peak> YsWave_PeakUtil::Detect(const YsWaveKernel &wav,int channel)
{
	auto &rawWav=wav.GetWave();
	std::vector <YsWaveKernel::Peak> peak;
	for(int ptr=1; ptr+1<rawWav.GetNumSamplePerChannel(); ++ptr)
	{
		bool isHigh;
		if(true==wav.IsPeak(isHigh,channel,ptr))
		{
			YsWaveKernel::Peak p;
			p.isHigh=isHigh;
			p.idx=ptr;
			peak.push_back(p);
		}
	}
	printf("%d peaks.\n",(int)peak.size());
	return peak;
}
