#include <ysport.h>
#include "yswavekernel.h"

long long YsWaveKernel::Region::GetLength(void) const
{
	return YsAbs(minmax[1]-minmax[0]);
}
long long YsWaveKernel::Region::Max(void) const
{
	return YsGreater(minmax[0],minmax[1]);
}
long long YsWaveKernel::Region::Min(void) const
{
	return YsSmaller(minmax[0],minmax[1]);
}

////////////////////////////////////////////////////////////

YsWaveKernel::YsWaveKernel()
{
	SetViewportZero(0);
	SetViewportWidth(1);
}
YsWaveKernel::~YsWaveKernel()
{
	CleanUp();
}
void YsWaveKernel::CleanUp(void)
{
	wav.CleanUp();
	peak.clear();
	envelope.clear();
	silentSegment.clear();
}

const YsSoundPlayer::SoundData &YsWaveKernel::GetWave(void) const
{
	return wav;
}

YSRESULT YsWaveKernel::LoadWav(const YsWString fName)
{
	YsFileIO::File fp(fName,"rb");
	if(nullptr!=fp)
	{
		if(YSOK==wav.LoadWav(fp))
		{
			SetViewportZero(0);
			SetViewportWidth(wav.GetNumSamplePerChannel());
			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT YsWaveKernel::SaveWav(const YsWString fName) const
{
	YsFileIO::File fp(fName,"wb");
	if(nullptr!=fp)
	{
		auto byteData=wav.MakeWavByteData();
		if(byteData.size()==fwrite(byteData.data(),1,byteData.size(),fp))
		{
			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT YsWaveKernel::AppendWav(const YsSoundPlayer::SoundData &incoming)
{
	if(incoming.GetNumChannel()==wav.GetNumChannel())
	{
printf("%s %d\n",__FUNCTION__,__LINE__);
		auto prevNumSample=wav.GetNumSamplePerChannel();
printf("%s %d\n",__FUNCTION__,__LINE__);
		wav.ResizeByNumSample(wav.GetNumSamplePerChannel()+incoming.GetNumSamplePerChannel());
printf("%s %d\n",__FUNCTION__,__LINE__);

printf("%s %d\n",__FUNCTION__,__LINE__);
		for(int channel=0; channel<wav.GetNumChannel(); ++channel)
		{
			for(long long int i=0; i<incoming.GetNumSamplePerChannel(); ++i)
			{
				auto value=incoming.GetSignedValue16(channel,i);
				wav.SetSignedValue16(channel,prevNumSample+i,value);
			}
		}
printf("%s %d\n",__FUNCTION__,__LINE__);
		return YSOK;
	}
	return YSERR;
}

YsWaveKernel::Viewport YsWaveKernel::GetViewport(void) const
{
	return this->viewport;
}
void YsWaveKernel::SetViewport(Viewport vp)
{
	viewport=vp;
}
void YsWaveKernel::SetViewportZero(int zero)
{
	viewport.zero=zero;
}
void YsWaveKernel::SetViewportWidth(int wid)
{
	viewport.wid=wid;
}


YsWaveKernel::Selection YsWaveKernel::GetSelection(void) const
{
	return selection;
}
void YsWaveKernel::SetSelection(Selection sel)
{
	this->selection=sel;
}


const std::vector <YsWaveKernel::Peak> &YsWaveKernel::GetPeak(void) const
{
	return peak;
}
void YsWaveKernel::SetPeak(std::vector <Peak> &&peak)
{
	std::swap(this->peak,peak);
}
void YsWaveKernel::SetPeak(const std::vector <Peak> &peak)
{
	this->peak=peak;
}


const std::vector <YsWaveKernel::Envelope> &YsWaveKernel::GetEnvelope(void) const
{
	return envelope;
}
void YsWaveKernel::SetEnvelope(std::vector <Envelope> &&env)
{
	std::swap(this->envelope,env);
}
void YsWaveKernel::SetEnvelope(const std::vector <Envelope> &env)
{
	this->envelope=env;
}

const std::vector <YsWaveKernel::Region> &YsWaveKernel::GetSilentSegment(void) const
{
	return silentSegment;
}

bool YsWaveKernel::IsPeak(bool &isHigh,int channel,YSSIZE_T ptr) const
{
	if(0<ptr && ptr+1<wav.GetNumSamplePerChannel())
	{
		int baseValue=wav.GetSignedValue16(channel,ptr);
		int left=wav.GetSignedValue16(channel,ptr-1);
		int right=wav.GetSignedValue16(channel,ptr+1);

		if(left==baseValue && baseValue==right)
		{
			return false;
		}

		for(YSSIZE_T i=ptr-1; 0<=i; --i)
		{
			left=wav.GetSignedValue16(channel,i);
			if(baseValue!=left)
			{
				break;
			}
		}
		for(YSSIZE_T i=ptr+1; i<wav.GetNumSamplePerChannel(); ++i)
		{
			right=wav.GetSignedValue16(channel,i);
			if(baseValue!=right)
			{
				break;
			}
		}
		if(left>baseValue && right>baseValue)
		{
			isHigh=false;
			return true;
		}
		if(left<baseValue && right<baseValue)
		{
			isHigh=true;
			return true;
		}
	}
	return false;
}

long long YsWaveKernel::ErasePeak(int channel,YSSIZE_T peakIdx)
{
// *
//     *           B       *D
//          C*     A*              *
// 
// Moving A to B just creates a new peak C.
// If A is moved up to match D, the left samples that are lower than B must be matched B.
// If A is moved down to match D, the left samples that are higher than B must be matched B.

	if(peakIdx+1>=peak.size())
	{
		return 0;
	}

	auto peak=this->peak[peakIdx];
	auto nextPeak=this->peak[peakIdx+1];
	if(peak.isHigh==nextPeak.isHigh)
	{
		return 0;
	}

	bool isHigh;
	if(YSTRUE!=IsPeak(isHigh,channel,peak.idx))
	{
		// If it is no longer a peak, don't bother.
		return 0;
	}

	auto curValue=wav.GetSignedValue16(channel,peak.idx);
	auto newValue=wav.GetSignedValue16(channel,nextPeak.idx);
	for(auto idx=peak.idx; idx<nextPeak.idx; ++idx)
	{
		wav.SetSignedValue16(channel,idx,newValue);
	}

	int nUpdate=1;

	if(curValue<newValue)
	{
		auto idx=peak.idx-1;
		while(0<=idx && wav.GetSignedValue16(channel,idx)<newValue)
		{
			wav.SetSignedValue16(channel,idx,newValue);
			--idx;
			++nUpdate;
		}
	}
	else
	{
		auto idx=peak.idx-1;
		while(0<=idx && newValue<wav.GetSignedValue16(channel,idx))
		{
			wav.SetSignedValue16(channel,idx,newValue);
			--idx;
			++nUpdate;
		}
	}

	return nUpdate;
}
