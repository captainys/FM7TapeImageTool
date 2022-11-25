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

