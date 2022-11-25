#include "yswave_medianfilter.h"



void YsWave_MedianFilter::Apply(YsSoundPlayer::SoundData &wav,int channel)
{
	for(int i=1; i<wav.GetNumSamplePerChannel()-1; ++i)
	{
		int sam[3]=
		{
			wav.GetSignedValue16(channel,i-1),
			wav.GetSignedValue16(channel,i  ),
			wav.GetSignedValue16(channel,i+1),
		};
		for(int a=0; a<3; ++a)
		{
			for(int b=a+1; b<3; ++b)
			{
				if(sam[a]>sam[b])
				{
					std::swap(sam[a],sam[b]);
				}
			}
		}
		wav.SetSignedValue16(channel,i,sam[1]);
	}
}
