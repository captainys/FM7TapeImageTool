#ifndef YSWAVE_WAVEUTIL_IS_INCLUDED
#define YSWAVE_WAVEUTIL_IS_INCLUDED
/* { */

#include "yswavekernel.h"

class YsWave_WaveUtil
{
private:
	bool highFirst;
	long long zero[3];
public:
	YSRESULT DetectWave(const YsSoundPlayer::SoundData &wav,int channel,long long ptr);
	YSRESULT DetectWaveBackward(const YsSoundPlayer::SoundData &wav,int channel,long long ptr);
	YSRESULT GetHump(long long boundary[2],const YsSoundPlayer::SoundData &wav,int channel,long long ptr0) const;

	bool HighFirst(void) const;
	YsWaveKernel::Region GetRegion(void) const;
	long long GetWaveLength(void) const;

	/*! The first wave of a byte trailing a silent segment may be stretched.
	    Cut off part before it reaches the 40% (hard-coded) of the max amplitude.
	*/
	long long GetWaveLengthFirstWaveOfByte(const YsSoundPlayer::SoundData &wav,int channel) const;


	void MakeSineWave(YsSoundPlayer::SoundData &wav,int channel,bool highFirst,long long ptr0,long long ptr1,int amplitude);
	void MakeSineWaveNoZero(YsSoundPlayer::SoundData &wav,int channel,bool highFirst,long long ptr0,long long ptr1,int amplitude);

	void ShiftPhase(YsSoundPlayer::SoundData &wav,int channel,long long int offset);

	void Trim(YsSoundPlayer::SoundData &wav,long long int from,long long int to);
};

/* } */
#endif
