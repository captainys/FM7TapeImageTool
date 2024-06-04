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
