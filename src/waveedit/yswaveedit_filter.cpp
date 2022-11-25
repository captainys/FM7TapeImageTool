#include "yswaveedit.h"
#include "yswave_medianfilter.h"



YSRESULT YsWaveEdit::RunCommand_Filter(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<2)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	if(0==argv[1].STRCMP("MEDIAN"))
	{
		return RunCommand_Filter_Median(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("EVENOUT"))
	{
		return RunCommand_Filter_EvenOut(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("EXPAND_ENVELOPE"))
	{
		return RunCommand_Filter_ExpandEnvelope(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("LOW_AND_SHORT_PEAK"))
	{
		return RunCommand_Filter_LowAndShortPeak(fullCmd,argv);
	}

	Error(fullCmd,"Unrecognized sub-command.");
	return YSERR;
}
YSRESULT YsWaveEdit::RunCommand_Filter_Median(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<3)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	auto channel=atoi(argv[2]);
	YsWave_MedianFilter filter;
	filter.Apply(this->wav,channel);

	return YSOK;
}

YSRESULT YsWaveEdit::RunCommand_Filter_EvenOut(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<5)
	{
		// FILTER EVENOUT channel HEIGHT|WIDTH threshold
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	auto channel=atoi(argv[2]);
	bool byHeight;
	if(0==argv[3].STRCMP("HEIGHT"))
	{
		byHeight=true;
	}
	else if(0==argv[3].STRCMP("WIDTH"))
	{
		byHeight=false;
	}
	else
	{
		return YSERR;
	}



	std::vector <long long> lowPeakIdx;
	std::vector <int> lowPeakScore;

	long long threshold=atoi(argv[4]);
	for(long long int peakIdx=0; peakIdx+1<peak.size(); ++peakIdx)
	{
		if(peak[peakIdx].isHigh!=peak[peakIdx+1].isHigh)
		{
			int diff=0;
			if(true==byHeight)
			{
				int level0=wav.GetSignedValue16(channel,peak[peakIdx].idx);
				int level1=wav.GetSignedValue16(channel,peak[peakIdx+1].idx);
				diff=YsAbs(level1-level0);
			}
			else
			{
				diff=peak[peakIdx+1].idx-peak[peakIdx].idx;
			}

			if(diff<threshold)
			{
				lowPeakIdx.push_back(peakIdx);
				lowPeakScore.push_back(diff);
			}
		}
	}

	YsSimpleMergeSort <int,long long> (lowPeakScore.size(),lowPeakScore.data(),lowPeakIdx.data());

	for(auto peakIdx : lowPeakIdx)
	{
		if(peak[peakIdx].isHigh!=peak[peakIdx+1].isHigh)
		{
			int diff=0;
			if(true==byHeight)
			{
				int level0=wav.GetSignedValue16(channel,peak[peakIdx].idx);
				int level1=wav.GetSignedValue16(channel,peak[peakIdx+1].idx);
				diff=YsAbs(level1-level0);
			}
			else
			{
				diff=peak[peakIdx+1].idx-peak[peakIdx].idx;
			}

			if(diff<threshold)
			{
				ErasePeak(channel,peakIdx);
			}
		}
	}

	return YSOK;
}

YSRESULT YsWaveEdit::RunCommand_Filter_ExpandEnvelope(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<3)
	{
		// FILTER EVENOUT channel HEIGHT|WIDTH threshold
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	auto channel=atoi(argv[2]);
	for(YSSIZE_T idx=0; idx<envelope.size() && idx<wav.GetNumSamplePerChannel(); ++idx)
	{
		auto env=envelope[idx];
		long long s=wav.GetSignedValue16(channel,idx);

		if(0==env.high-env.low)
		{
			printf("Envelope collapse at %lld\n",idx);
			continue;
		}

		s=-30000+(s-env.low)*60000/(env.high-env.low);
		s=YsBound<long long>(s,-30000,30000);
		wav.SetSignedValue16(channel,idx,s);
	}

	return YSOK;
}

YSRESULT YsWaveEdit::RunCommand_Filter_LowAndShortPeak(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<5)
	{
		// FILTER LOW_AND_SHORT_PEAK channel lowThr shortCht
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}


	std::vector <YsWaveKernel::Peak> peak=GetPeak(),newPeak;
	auto channel=argv[2].Atoi();
	auto lowThr=argv[3].Atoi();
	auto shortThr=argv[4].Atoi();

	for(YSSIZE_T i=peak.size()-1; 0<=i; --i)
	{
		peak[i].deleted=false;
	}

	YSSIZE_T i0=0,i1=0;
	for(YSSIZE_T i=0; i<peak.size(); ++i)
	{
		if(peak[i].isHigh!=peak[i1].isHigh)
		{
			i0=i1;
			i1=i;

			if(peak[i1].idx<=peak[i0].idx+shortThr)
			{
				for(auto j=i0; j<=i1; ++j)
				{
					peak[j].deleted=true;
				}
				i1=i+1;
				i0=i1;
				continue;
			}

			auto level0=wav.GetSignedValue16(channel,peak[i0].idx);
			auto level1=wav.GetSignedValue16(channel,peak[i1].idx);
			auto diff=YsAbs(level1-level0);
			if(diff<lowThr)
			{
				for(auto j=i0; j<=i1; ++j)
				{
					peak[j].deleted=true;
				}
				i1=i+1;
				i0=i1;
				continue;
			}
		}
	}

	for(auto &p : peak)
	{
		if(true!=p.deleted)
		{
			newPeak.push_back(p);
		}
	}

	SetPeak(newPeak);

	return YSOK;
}
