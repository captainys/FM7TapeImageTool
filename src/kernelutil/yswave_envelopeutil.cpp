#include "yswave_envelopeutil.h"


std::vector <YsWaveKernel::Envelope> YsWave_EnvelopeUtil::CalculateEnvelope(
		const YsSoundPlayer::SoundData &wav,
		const std::vector <YsWaveKernel::Peak> &realPeak,
		const std::vector <YsWaveKernel::Region> &silentRegion,
		int channel)
{
	std::vector <YsWaveKernel::Envelope> envelope;
	std::vector <Sample> highSample,lowSample;
	std::vector <bool> silentRegionMark;


	silentRegionMark.resize(wav.GetNumSamplePerChannel());
	for(long long idx=0; idx<silentRegionMark.size(); ++idx)
	{
		silentRegionMark[idx]=false;
	}

	{
		std::vector <long long> highPtrArray,lowPtrArray;
		for(auto sr : silentRegion)
		{
			for(long long idx=sr.Min(); idx<=sr.Max() && idx<silentRegionMark.size(); ++idx)
			{
				silentRegionMark[idx]=true;
			}

			Sample s;
			s.ptr=sr.Min();
			s.level=32767;
			highSample.push_back(s);
			highPtrArray.push_back(s.ptr);
			s.level=-32767;
			lowSample.push_back(s);
			lowPtrArray.push_back(s.ptr);

			s.ptr=sr.Max();
			s.level=32767;
			highSample.push_back(s);
			highPtrArray.push_back(s.ptr);
			s.level=-32767;
			lowSample.push_back(s);
			lowPtrArray.push_back(s.ptr);
		}
		for(auto p : realPeak)
		{
			Sample s;
			s.ptr=p.idx;
			if(0<=s.ptr && s.ptr<silentRegionMark.size() && true==silentRegionMark[p.idx])
			{
				continue;
			}

			s.level=wav.GetSignedValue16(channel,p.idx);
			if(true==p.isHigh)
			{
				highSample.push_back(s);
				highPtrArray.push_back(p.idx);
			}
			else
			{
				lowSample.push_back(s);
				lowPtrArray.push_back(p.idx);
			}
		}
		YsSimpleMergeSort <long long,Sample> (lowPtrArray.size(),lowPtrArray.data(),lowSample.data());
		YsSimpleMergeSort <long long,Sample> (highPtrArray.size(),highPtrArray.data(),highSample.data());
	}


	envelope.resize(wav.GetNumSamplePerChannel());
	for(auto &e : envelope)
	{
		e.high=0;
		e.low=0;
	}

	YSSIZE_T selBegin=0;
	YSSIZE_T selEnd=wav.GetNumSamplePerChannel()-1;


	long long int lastHigh=0,lastLow=0;
	auto lastHighPtr=selEnd,lastLowPtr=selEnd;

	// Low
	for(long long idx=0; idx<lowSample[0].ptr; ++idx)
	{
		envelope[idx].low=lowSample[0].level;
	}
	for(long long sampleIdx=0; sampleIdx+1<lowSample.size(); ++sampleIdx)
	{
		auto ptr0=lowSample[sampleIdx].ptr;
		auto level0=lowSample[sampleIdx].level;
		auto ptr1=lowSample[sampleIdx+1].ptr;
		auto level1=lowSample[sampleIdx+1].level;
		for(long long idx=ptr0; idx<=ptr1; ++idx)
		{
			if(0<=idx && idx<wav.GetNumSamplePerChannel())
			{
				// Cosine interpolation
				const double t=(double)(idx-ptr0)/(double)(ptr1-ptr0);
				const double ang=t*YsPi;
				const double param=1.0-(cos(ang)+1.0)/2.0;
				envelope[idx].low=(int)((double)level0*(1.0-param)+(double)level1*param);
			}
		}
	}
	if(0<lowSample.size())
	{
		for(long long idx=lowSample.back().ptr; idx<wav.GetNumSamplePerChannel(); ++idx)
		{
			envelope[idx].low=lowSample.back().level;
		}
	}

	// High
	for(long long idx=0; idx<highSample[0].ptr; ++idx)
	{
		envelope[idx].high=highSample[0].level;
	}
	for(long long sampleIdx=0; sampleIdx+1<highSample.size(); ++sampleIdx)
	{
		auto ptr0=highSample[sampleIdx].ptr;
		auto level0=highSample[sampleIdx].level;
		auto ptr1=highSample[sampleIdx+1].ptr;
		auto level1=highSample[sampleIdx+1].level;
		for(long long idx=ptr0; idx<=ptr1; ++idx)
		{
			if(0<=idx && idx<wav.GetNumSamplePerChannel())
			{
				// Cosine interpolation
				const double t=(double)(idx-ptr0)/(double)(ptr1-ptr0);
				const double ang=t*YsPi;
				const double param=1.0-(cos(ang)+1.0)/2.0;
				envelope[idx].high=(int)((double)level0*(1.0-param)+(double)level1*param);
			}
		}
	}
	if(0<highSample.size())
	{
		for(long long idx=highSample.back().ptr; idx<wav.GetNumSamplePerChannel(); ++idx)
		{
			envelope[idx].high=highSample.back().level;
		}
	}

	return envelope;
}
