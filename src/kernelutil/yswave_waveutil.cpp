#include "yswave_waveutil.h"
#include <cstdlib>
#include <algorithm>



YSRESULT YsWave_WaveUtil::DetectWave(const YsSoundPlayer::SoundData &wav,int channel,long long ptr0)
{
	auto ptr=ptr0;
	highFirst=true;

	for(ptr=ptr0; ptr<wav.GetNumSamplePerChannel() && 0==wav.GetSignedValue16(channel,ptr); ++ptr)
	{
	}
	if(wav.GetNumSamplePerChannel()<=ptr)
	{
		return YSERR;
	}

	if(0<wav.GetSignedValue16(channel,ptr))
	{
		highFirst=true;
	}
	else
	{
		highFirst=false;
	}

	long long boundary[2][2];
	if(YSOK==GetHump(boundary[0],wav,channel,ptr))
	{
	}

	for(ptr=boundary[0][1]+1; ptr<wav.GetNumSamplePerChannel() && 0==wav.GetSignedValue16(channel,ptr); ++ptr)
	{
	}
	if(wav.GetNumSamplePerChannel()<=ptr)
	{
		return YSERR;
	}

	if(YSOK==GetHump(boundary[1],wav,channel,ptr))
	{
	}

	zero[0]=boundary[0][0];
	zero[1]=(boundary[0][1]+boundary[1][0])/2;
	zero[2]=boundary[1][1];
	return YSOK;
}

YSRESULT YsWave_WaveUtil::DetectWaveBackward(const YsSoundPlayer::SoundData &wav,int channel,long long ptr0)
{
	auto ptr=ptr0;
	highFirst=true;

	for(ptr=ptr0; 0<ptr && 0==wav.GetSignedValue16(channel,ptr); --ptr)
	{
	}
	if(ptr<=0)
	{
		return YSERR;
	}

	if(0<wav.GetSignedValue16(channel,ptr))
	{
		highFirst=false;
	}
	else
	{
		highFirst=true;
	}

	long long boundary[2][2];
	if(YSOK==GetHump(boundary[0],wav,channel,ptr))
	{
	}

	for(ptr=boundary[0][0]-1; 0<ptr && 0==wav.GetSignedValue16(channel,ptr); --ptr)
	{
	}
	if(ptr<=0)
	{
		return YSERR;
	}

	if(YSOK==GetHump(boundary[1],wav,channel,ptr))
	{
	}

	zero[0]=boundary[1][0];
	zero[1]=(boundary[0][1]+boundary[1][0])/2;
	zero[2]=boundary[0][1];
	return YSOK;
}

YSRESULT YsWave_WaveUtil::GetHump(long long boundary[2],const YsSoundPlayer::SoundData &wav,int channel,long long ptr0) const
{
	boundary[0]=ptr0;
	boundary[1]=ptr0;

	auto refValue=wav.GetSignedValue16(channel,ptr0);
	if(0==refValue)
	{
		return YSERR;
	}

	while(0<boundary[0] && 0<refValue*wav.GetSignedValue16(channel,boundary[0]-1))
	{
		--boundary[0];
	}
	while(boundary[1]+1<wav.GetNumSamplePerChannel() && 0<refValue*wav.GetSignedValue16(channel,boundary[1]+1))
	{
		++boundary[1];
	}
	return YSOK;
}


bool YsWave_WaveUtil::HighFirst(void) const
{
	return highFirst;
}

YsWaveKernel::Region YsWave_WaveUtil::GetRegion(void) const
{
	YsWaveKernel::Region rgn;
	rgn.minmax[0]=zero[0];
	rgn.minmax[1]=zero[2];
	return rgn;
}


long long YsWave_WaveUtil::GetWaveLength(void) const
{
	return (zero[2]-zero[0])+1;
}

long long YsWave_WaveUtil::GetWaveLengthFirstWaveOfByte(const YsSoundPlayer::SoundData &wav,int channel) const
{
	const int firstBitThreshold=40;

	int maxAmpl=0;
	for(int i=zero[0]; i<=zero[1]; ++i)
	{
		auto ampl=std::abs(wav.GetSignedValue16(channel,i));
		maxAmpl=std::max(maxAmpl,ampl);
	}

	long long realZero=zero[0];
	for(; realZero<zero[1]; ++realZero)
	{
		auto ampl=std::abs(wav.GetSignedValue16(channel,realZero));
		if(maxAmpl*firstBitThreshold/100<=ampl)
		{
			break;
		}
	}

	return (zero[2]-realZero)+1;
}



void YsWave_WaveUtil::MakeSineWave(YsSoundPlayer::SoundData &wav,int channel,bool highFirst,long long ptr0,long long ptr1,int amplitude)
{
	double d=(double)(ptr1-ptr0);
	for(auto i=ptr0; i<=ptr1; ++i)
	{
		if(0<=i && i<=wav.GetNumSamplePerChannel())
		{
			double x=(double)(i-ptr0);
			double a=YsPi*2.0*x/d;
			double y=sin(a)*(double)amplitude;
			if(true!=highFirst)
			{
				y=-y;
			}
			wav.SetSignedValue16(channel,i,(int)y);
		}
	}
}

void YsWave_WaveUtil::MakeSineWaveNoZero(YsSoundPlayer::SoundData &wav,int channel,bool highFirst,long long ptr0,long long ptr1,int amplitude)
{
	double d=(double)(ptr1-ptr0);
	for(auto i=ptr0; i<=ptr1; ++i)
	{
		if(0<=i && i<=wav.GetNumSamplePerChannel())
		{
			double x=(double)(i-ptr0);
			double a=YsPi*2.0*(0.025+0.95*x/d);
			double y=sin(a)*(double)amplitude;
			if(true!=highFirst)
			{
				y=-y;
			}
			wav.SetSignedValue16(channel,i,(int)y);
		}
	}
}

void YsWave_WaveUtil::ShiftPhase(YsSoundPlayer::SoundData &wav,int channel,long long int offset)
{
	if(0<offset)
	{
		for(long long int ptr=wav.GetNumSamplePerChannel()-1; 0<ptr; --ptr)
		{
			if(offset<=ptr)
			{
				auto y=wav.GetSignedValue16(channel,ptr-offset);
				wav.SetSignedValue16(channel,ptr,y);
			}
			else
			{
				wav.SetSignedValue16(channel,ptr,0);
			}
		}
	}
	else if(offset<0)
	{
		offset=-offset;  // Make positive.
		for(long long int ptr=0; ptr<wav.GetNumSamplePerChannel(); ++ptr)
		{
			if(ptr+offset<wav.GetNumSamplePerChannel())
			{
				auto y=wav.GetSignedValue16(channel,ptr+offset);
				wav.SetSignedValue16(channel,ptr,y);
			}
			else
			{
				wav.SetSignedValue16(channel,ptr,0);
			}
		}
	}
}

void YsWave_WaveUtil::Trim(YsSoundPlayer::SoundData &wav,long long int from,long long int to)
{
	if(to<from)
	{
		std::swap(from,to);
	}

	if(from<0)
	{
		from=0;
	}
	if(wav.GetNumSamplePerChannel()<to)
	{
		to=wav.GetNumSamplePerChannel();
	}

	std::vector <std::vector <decltype(wav.GetSignedValue16(0,0))> >save;
	save.resize(wav.GetNumChannel());
	for(int channel=0; channel<wav.GetNumChannel(); ++channel)
	{
		for(auto i=from; i<to; ++i)
		{
			save[channel].push_back(wav.GetSignedValue16(channel,i));
		}
	}

	wav.ResizeByNumSample(to-from);
	for(int channel=0; channel<wav.GetNumChannel(); ++channel)
	{
		for(auto i=0; i<save[channel].size(); ++i)
		{
			wav.SetSignedValue16(channel,i,save[channel][i]);
		}
	}
}
