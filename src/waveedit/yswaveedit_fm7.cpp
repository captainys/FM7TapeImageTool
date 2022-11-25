#include "yswaveedit.h"
#include "yswave_fm7util.h"
#include "yswave_waveutil.h"



YSRESULT YsWaveEdit::RunCommand_FM7(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	if(argv.size()<2)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	if(0==argv[1].STRCMP("REPAIR_PREPOSTFF"))
	{
		return RunCommand_FM7_RepairPrecedingAndTrailingFF(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("CLEAR_BLOCKGAP"))
	{
		return RunCommand_FM7_ClearFileBlockGap(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("REPAIR_BYTE"))
	{
		return RunCommand_FM7_RepairByte(fullCmd,argv);
	}
	else if(0==argv[1].STRCMP("WAVELENGTH_THR"))
	{
		return RunCommand_FM7_WaveLengthThr(fullCmd,argv);
	}

	Error(fullCmd,"Unrecognized sub-command.");
	return YSERR;
}
YSRESULT YsWaveEdit::RunCommand_FM7_RepairPrecedingAndTrailingFF(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	// FM7 REPAIR_PREPOSTFF channel fileStartPtr
	if(argv.size()<4)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	auto channel=atoi(argv[2].c_str());
	long long wavPtr=atoi(argv[3].c_str());

	auto &wavRaw=this->wav;
	YsWave_FM7Util fm7Util;
	auto file=fm7Util.ReadFile(wavRaw,channel,wavPtr,fm7UtilOption);
	for(auto &blk : file.block)
	{
		if(YsWave_FM7Util::ERROR_NOERROR==blk.errorCode)
		{
			// 01111111111
			// Repair first zero.

			bool brokenFF=true;
			auto ptr=blk.minmax[0]-1;
			bool highFirst=true;
			for(int i=0; i<10; ++i)
			{
				YsWave_WaveUtil waveUtil;
				waveUtil.DetectWaveBackward(wavRaw,channel,ptr);
				if(true!=fm7UtilOption.IsOne(waveUtil.GetWaveLength()))
				{
					brokenFF=false;
					break;
				}
				ptr=waveUtil.GetRegion().Min()-1;
				highFirst=waveUtil.HighFirst();
			}
			if(true==brokenFF)
			{
				auto selBegin=ptr-(fm7UtilOption.zeroMinWaveLength+fm7UtilOption.zeroMaxWaveLength)/2;
				auto selEnd=ptr;
				YsWave_WaveUtil waveUtil;
				waveUtil.MakeSineWave(wavRaw,channel,highFirst,selBegin,selEnd,30000);
			}



			brokenFF=true;
			ptr=blk.minmax[1]+1;
			highFirst=true;
			for(int i=0; i<10; ++i)
			{
				YsWave_WaveUtil waveUtil;
				waveUtil.DetectWave(wavRaw,channel,ptr);
				if((0==i && true!=fm7UtilOption.IsZero(waveUtil.GetWaveLength())) ||
				   (0!=i && true!=fm7UtilOption.IsOne(waveUtil.GetWaveLength())))
				{
					brokenFF=false;
					break;
				}
				ptr=waveUtil.GetRegion().Max()+1;
				highFirst=waveUtil.HighFirst();
			}
			if(true==brokenFF)
			{
				auto selBegin=ptr;
				auto selEnd=ptr+(fm7UtilOption.oneMinWaveLength+fm7UtilOption.oneMaxWaveLength)/2;
				YsWave_WaveUtil waveUtil;
				waveUtil.MakeSineWave(wavRaw,channel,highFirst,selBegin,selEnd,30000);
			}
		}
	}

	return YSOK;
}

YSRESULT YsWaveEdit::RunCommand_FM7_ClearFileBlockGap(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	// FM7 CLEAR_BLOCKGAP channel fileStartPtr
	if(argv.size()<4)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	auto channel=atoi(argv[2].c_str());
	long long wavPtr=atoi(argv[3].c_str());

	auto &wavRaw=this->wav;
	YsWave_FM7Util fm7Util;
	auto file=fm7Util.ReadFile(wavRaw,channel,wavPtr,fm7UtilOption);
	for(long long idx=0; idx+1<file.block.size(); ++idx)
	{
		auto selBegin=file.block[idx].minmax[1];
		auto selEnd=file.block[idx+1].minmax[0];
		for(long long ptr=selBegin; ptr<selEnd; ++ptr)
		{
			wav.SetSignedValue16(channel,ptr,0);
		}
		Region rgn;
		rgn.minmax[0]=selBegin;
		rgn.minmax[1]=selEnd;
		silentSegment.push_back(rgn);
	}

	return YSOK;

}

YSRESULT YsWaveEdit::RunCommand_FM7_RepairByte(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	// FM7 REPAIR_BYTE channel ptr
	if(argv.size()<4)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	auto channel=atoi(argv[2].c_str());
	long long wavPtr=atoi(argv[3].c_str());

	YsWave_FM7Util fm7Util;
	return fm7Util.RepairByte(wav,channel,wavPtr,fm7UtilOption);
}


YSRESULT YsWaveEdit::RunCommand_FM7_WaveLengthThr(const YsString &fullCmd,YsConstArrayMask <YsString> argv)
{
	// FM7 WAVELENGTH_THR min max min max
	if(argv.size()<6)
	{
		Error(fullCmd,"Too few arguments.");
		return YSERR;
	}

	fm7UtilOption.zeroMinWaveLength=atoi(argv[2]);
	fm7UtilOption.zeroMaxWaveLength=atoi(argv[3]);
	fm7UtilOption.oneMinWaveLength=atoi(argv[4]);
	fm7UtilOption.oneMaxWaveLength=atoi(argv[5]);
	return YSOK;
}
