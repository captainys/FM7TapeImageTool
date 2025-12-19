#include <iostream>

#include "yswaveedit.h"

#include <yswave_waveutil.h>



YSRESULT YsWaveEdit::RunCommand_Edit(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<2)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	if(0==argv[1].STRCMP("SILENCE"))
	{
		return RunCommand_Edit_Silence(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("DELETE_CHANNEL"))
	{
		return RunCommand_Edit_DeleteChannel(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("MERGE_CHANNEL"))
	{
		return RunCommand_Edit_MergeChannel(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("MAKE_SINEWAVE"))
	{
		return RunCommand_Edit_MakeSineWave(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("SHIFT_PHASE"))
	{
		return RunCommand_Edit_ShiftPhase(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("TRIM"))
	{
		return RunCommand_Edit_Trim(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("REALLY_SILENCE_SILENT_REGIONS"))
	{
		return RunCommand_Edit_ReallySilenceSilentRegion(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("FORCELENGTH"))
	{
		return RunCommand_Edit_ForceWaveLength(fullCmd,argv);
	}

	Error(fullCmd,"Unrecognized sub-command.");
	return YSERR;
}
YSRESULT YsWaveEdit::RunCommand_Edit_Silence(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<5)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	// EDIT SILENCE channel begin end
	auto channel=atoi(argv[2].c_str());
	auto selBegin=atoi(argv[3].c_str());
	auto selEnd=atoi(argv[4].c_str());

	auto &wavRaw=this->wav;
	for(long long int ptr=selBegin; ptr<selEnd; ++ptr)
	{
		wavRaw.SetSignedValue16(channel,ptr,0);
	}

	Region rgn;
	rgn.minmax[0]=selBegin;
	rgn.minmax[1]=selEnd;
	silentSegment.push_back(rgn);

	return YSOK;
}

YSRESULT YsWaveEdit::RunCommand_Edit_DeleteChannel(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<3)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	// EDIT SILENCE channel begin end
	auto channel=atoi(argv[2].c_str());
	this->wav.DeleteChannel(channel);
	return YSOK;
}

YSRESULT YsWaveEdit::RunCommand_Edit_MakeSineWave(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	// EDIT MAKE_SINEWAVE channel 1:highFirst/0:lowFirst begin end amplitude
	if(argv.size()<7)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	auto channel=atoi(argv[2].c_str());
	bool highFirst=(0!=atoi(argv[3].c_str()));
	auto selBegin=atoi(argv[4].c_str());
	auto selEnd=atoi(argv[5].c_str());
	auto amplitude=atoi(argv[6].c_str());

	YsWave_WaveUtil waveUtil;
	waveUtil.MakeSineWave(this->wav,channel,highFirst,selBegin,selEnd,amplitude);

	return YSOK;
}

YSRESULT YsWaveEdit::RunCommand_Edit_ShiftPhase(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<4)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	// EDIT SILENCE channel begin end
	auto channel=atoi(argv[2].c_str());
	auto offset=atoi(argv[3].c_str());
	YsWave_WaveUtil waveUtil;
	waveUtil.ShiftPhase(this->wav,channel,offset);
	return YSOK;
}

YSRESULT YsWaveEdit::RunCommand_Edit_Trim(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<4)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	auto selBegin=atoi(argv[2].c_str());
	auto selEnd=atoi(argv[3].c_str());

	YsWave_WaveUtil waveUtil;
	waveUtil.Trim(this->wav,selBegin,selEnd);
	return YSOK;
}
YSRESULT YsWaveEdit::RunCommand_Edit_MergeChannel(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(2<=this->wav.GetNumChannel())
	{
		for(long long int ptr=0; ptr<this->wav.GetNumSamplePerChannel(); ++ptr)
		{
			int avg=0;
			for(int channel=0; channel<this->wav.GetNumChannel(); ++channel)
			{
				avg+=this->wav.GetSignedValue16(channel,ptr);
			}
			avg/=this->wav.GetNumChannel();
			this->wav.SetSignedValue16(0,ptr,avg);
		}

		for(int channel=wav.GetNumChannel()-1; 1<=channel; --channel)
		{
			this->wav.DeleteChannel(channel);
		}
	}
	return YSOK;
}
YSRESULT YsWaveEdit::RunCommand_Edit_ReallySilenceSilentRegion(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<3)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	auto channel=atoi(argv[2].c_str());

	for(auto rgn : silentSegment)
	{
		for(auto i=rgn.minmax[0]; i<rgn.minmax[1]; ++i)
		{
			this->wav.SetSignedValue16(channel,i,0);
		}
	}
}

// [0]  [1]         [2]     [3]   [4]  [5][6]       [7]       [8]
// EDIT FORCELENGTH channel 0/1/2 from to srcLenMin srcLenMax toThisLen
//                          0... Don't Care
//                          1... Low First
//                          2... High First
YSRESULT YsWaveEdit::RunCommand_Edit_ForceWaveLength(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	const int lowFirstWaveType=1;
	const int highFirstWaveType=2;

	if(argv.size()<9)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	auto channel=atoi(argv[2].c_str());
	auto waveType=atoi(argv[3].c_str());

	auto minSrcLen=atoi(argv[6].c_str());
	auto maxSrcLen=atoi(argv[7].c_str());
	auto newLen=atoi(argv[8].c_str());

	if(this->wav.GetNumChannel()<=channel)
	{
		Error(fullCmd,"Invalid channel.");
		return YSERR;
	}

	size_t i0=atoi(argv[4].c_str());
	size_t i1=atoi(argv[5].c_str());

	i0=std::min<size_t>(this->wav.GetNumSamplePerChannel()-1,i0);
	i1=std::min<size_t>(this->wav.GetNumSamplePerChannel()-1,i1);

	if(i1<=i0) // Nothing to do.
	{
		return YSOK;
	}

	std::vector <int> resample,after;

	for(auto i=i1; i<this->wav.GetNumSamplePerChannel(); ++i)
	{
		after.push_back(this->wav.GetSignedValue16(channel,i));
	}

	bool everProcessed=false;
	for(auto i=i0; i<i1; )
	{
		bool processed=false;
		YsWave_WaveUtil oneWave;
		if(YSOK==oneWave.DetectWave(this->wav,channel,i))
		{
			if(lowFirstWaveType==waveType && true==oneWave.HighFirst())
			{
				// Do nothing.
			}
			else if(highFirstWaveType==waveType && true!=oneWave.HighFirst())
			{
				// Do nothing.
			}
			else
			{
				auto rgn=oneWave.GetRegion();
				auto len=rgn.GetLength();

				if(i0<=rgn.minmax[0] && minSrcLen<=len && len<=maxSrcLen)
				{
					for(size_t k=0; k<newLen; ++k)
					{
						auto source=i+len*k/newLen;
						resample.push_back(this->wav.GetSignedValue16(channel,source));
					}
					// In case new length is shorter.
					for(size_t k=newLen; k<len; ++k)
					{
						resample.push_back(0);
					}

					i=rgn.minmax[1];
					processed=true;
					everProcessed=true;
				}
			}
		}

		if(true!=processed)
		{
			resample.push_back(this->wav.GetSignedValue16(channel,i));
			++i;
		}
	}

	if(true==everProcessed)
	{
		auto oldLen=this->wav.GetNumSamplePerChannel();
		auto newLen=i0+resample.size()+after.size();

		wav.ResizeByNumSample(newLen);
		auto ptr=i0;
		for(auto d : resample)
		{
			this->wav.SetSignedValue16(channel,ptr++,d);
		}
		for(auto d : after)
		{
			this->wav.SetSignedValue16(channel,ptr++,d);
		}
		while(ptr<newLen)
		{
			for(int c=0; c<this->wav.GetNumChannel(); ++c)
			{
				this->wav.SetSignedValue16(c,ptr++,0);
			}
		}

		printf("Old Len %d  New Len %d\n",oldLen,newLen);
	}

	return YSOK;
}
