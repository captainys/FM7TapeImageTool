#include "yswaveedit.h"
#include "yswave_peakutil.h"
#include "yswave_waveutil.h"
#include "yswave_envelopeutil.h"


YSRESULT YsWaveEdit::RunCommand_Analyze(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<2)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	if(0==argv[1].STRCMP("DETECT_PEAK"))
	{
		return RunCommand_Analyze_DetectPeak(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("MARK_SILENT"))
	{
		return RunCommand_Analyze_MarkSilentSegment(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("CALCULATE_ENVELOPE"))
	{
		return RunCommand_Analyze_CalculateEnvelope(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("MARK_LONG_WAVE_SILENT"))
	{
		return RunCommand_Analyze_MarkLongWaveAsSilent(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("UNSILENCE"))
	{
		return RunCommand_Analyze_Unsilence(fullCmd,argv);
	}

	Error(fullCmd,"Unrecognized sub-command.");
	return YSERR;
}

YSRESULT YsWaveEdit::RunCommand_Analyze_DetectPeak(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<3)
	{
		Error(fullCmd,"Too few arguments. Need channel.");
		return YSERR;
	}
	YsWave_PeakUtil util;
	this->SetPeak(util.Detect(*this,atoi(argv[2])));
	return YSOK;
}

YSRESULT YsWaveEdit::RunCommand_Analyze_MarkSilentSegment(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<5)
	{
		Error(fullCmd,"Too few arguments. Need channel.");
		return YSERR;
	}

	const int channel=atoi(argv[2]);
	const int levelThr=atoi(argv[3]);
	const double durationThr=atof(argv[4]);

	const auto numStepThr=wav.SecToNumSample(durationThr);

	silentSegment.clear();

	bool inSilence=false;
	long long segBegin=0;
	for(long long ptr=0; ptr<wav.GetNumSamplePerChannel(); ++ptr)
	{
		auto level=YsAbs(wav.GetSignedValue16(channel,ptr));
		if(true!=inSilence)
		{
			if(level<=levelThr)
			{
				segBegin=ptr;
				inSilence=true;
			}
		}
		else // if(true!=inSilence)
		{
			if(levelThr<level || ptr+1==wav.GetNumSamplePerChannel())
			{
				auto segEnd=ptr;
				if(numStepThr<=segEnd-segBegin+1)
				{
					Region rgn;
					rgn.minmax[0]=segBegin;
					rgn.minmax[1]=segEnd;
					silentSegment.push_back(rgn);
				}
				inSilence=false;
			}
		}
	}
	return YSOK;
}

YSRESULT YsWaveEdit::RunCommand_Analyze_CalculateEnvelope(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<3)
	{
		Error(fullCmd,"Too few arguments. Need channel.");
		return YSERR;
	}
	YsWave_EnvelopeUtil util;
	this->SetEnvelope(util.CalculateEnvelope(this->GetWave(),this->peak,this->silentSegment,atoi(argv[2])));
	return YSOK;
}
YSRESULT YsWaveEdit::RunCommand_Analyze_MarkLongWaveAsSilent(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	// ANALYZE MARK_LONG_WAVE_SILENT ch longHumpThr shortHumpThr silenceThr followedByDownWave
	if(argv.size()<7)
	{
		Error(fullCmd,"Too few arguments. Need channel.");
		return YSERR;
	}

	auto &wavRaw=this->wav;

	const int channel=atoi(argv[2]);

	unsigned int longHumpThr=argv[3].Atoi();
	unsigned int shortHumpThr=argv[4].Atoi();
	unsigned int trailingSilenceThr=argv[5].Atoi();
	bool followedByDownWave=(0!=argv[6].Atoi());

	int state=0;
	unsigned int longWaveBegin=0,longWaveEnd=0;

	YsLoopCounter ctr;
	ctr.Begin(wavRaw.GetNumSamplePerChannel());
	for(YSSIZE_T ptr=0; ptr+longHumpThr<wavRaw.GetNumSamplePerChannel(); ++ptr)
	{
		ctr.Show(ptr);

		YsWave_WaveUtil waveUtil;
		long long minmax[2];
		int lastSign;
		if(YSOK==waveUtil.GetHump(minmax,wavRaw,channel,ptr))
		{
			auto len=minmax[1]-minmax[0];
			if(0==state)
			{
				if(longHumpThr<=len)
				{
					longWaveBegin=minmax[0];
					longWaveEnd=minmax[1];
					ptr+=len-1;
					state=1;
					lastSign=wavRaw.GetSignedValue16(channel,(minmax[0]+minmax[1])/2);
				}
			}
			else
			{
				if(longHumpThr<=len)
				{
					longWaveEnd=minmax[1];
					lastSign=wavRaw.GetSignedValue16(channel,(minmax[0]+minmax[1])/2);
				}
				else
				{
					while(longWaveEnd<wavRaw.GetNumSamplePerChannel() &&
					      YsAbs(wavRaw.GetSignedValue16(channel,longWaveEnd))<trailingSilenceThr)
					{
						++longWaveEnd;
					}

					if(longWaveEnd<wavRaw.GetNumSamplePerChannel())
					{
						auto sign0=wavRaw.GetSignedValue16(channel,longWaveEnd);

						auto ptr=longWaveEnd;
						while(ptr<wavRaw.GetNumSamplePerChannel() && 0<=sign0*wavRaw.GetSignedValue16(channel,ptr))
						{
							++ptr;
						}

						if(longWaveEnd+shortHumpThr<ptr)
						{
							longWaveEnd=ptr-shortHumpThr;
						}
					}

					if(longWaveEnd+shortHumpThr/2<wavRaw.GetNumSamplePerChannel() &&
					   YSOK==waveUtil.GetHump(minmax,wavRaw,channel,longWaveEnd+shortHumpThr/2))
					{
						auto sign=wavRaw.GetSignedValue16(channel,(minmax[0]+minmax[1])/2);
						if((true==followedByDownWave && 0<sign) ||
						   (true!=followedByDownWave && sign<0))
						{
							if(shortHumpThr<longWaveEnd)
							{
								longWaveEnd-=shortHumpThr;
							}
						}
					}

					Region rgn;
					rgn.minmax[0]=longWaveBegin;
					rgn.minmax[1]=longWaveEnd;
					silentSegment.push_back(rgn);
					state=0;
				}
			}
			ptr=std::max<YSSIZE_T>(ptr,minmax[1]);
		}
	}

	return YSOK;
}

YSRESULT YsWaveEdit::RunCommand_Analyze_Unsilence(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<5)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	auto &wavRaw=this->wav;

	// unsigned int ch=argv[2].Atoi(); // DUmmy Param
	unsigned int start=argv[3].Atoi();
	unsigned int end=argv[4].Atoi();

	std::vector <bool> silent;
	silent.resize(wavRaw.GetNumSamplePerChannel());

	for(YSSIZE_T i=0; i<silent.size(); ++i)
	{
		silent[i]=false;
	}
	for(auto rgn : silentSegment)
	{
		for(auto i=rgn.minmax[0]; i<rgn.minmax[1]; ++i)
		{
			silent[i]=true;
		}
	}
	for(auto i=start; i<end; ++i)
	{
		silent[i]=false;
	}

	silentSegment.clear();
	int state=0;
	Region rgn;
	for(unsigned int i=0; i<silent.size(); ++i)
	{
		if(0==state && true==silent[i])
		{
			rgn.minmax[0]=i;
			state=1;
		}
		else if(1==state && true!=silent[i])
		{
			rgn.minmax[1]=i;
			silentSegment.push_back(rgn);
			state=0;
		}
	}
	return YSOK;
}
