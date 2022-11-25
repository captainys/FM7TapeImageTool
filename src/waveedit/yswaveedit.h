#ifndef YSWAVEEDIT_IS_INCLUDED
#define YSWAVEEDIT_IS_INCLUDED
/* { */

#include <vector>
#include <ysclass.h>

#include "yswavekernel.h"
#include "yswave_fm7util.h"


class YsWaveEdit : public YsWaveKernel
{
private:
	std::vector <YsString> cmdLog;
	YsString errString;

public:
	const std::vector <YsString> &GetCommandLog(void) const;
	YSRESULT RunCommand(YsString cmd);

	YSRESULT LoadWav(YsWString fName);

private:
	YSRESULT RunCommand_Analyze(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Analyze_DetectPeak(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Analyze_MarkSilentSegment(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Analyze_CalculateEnvelope(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Analyze_MarkLongWaveAsSilent(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Analyze_Unsilence(const YsString &fullCmd,YsConstArrayMask <YsString> argv);

	YSRESULT RunCommand_Edit(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Edit_Silence(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Edit_DeleteChannel(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Edit_MakeSineWave(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Edit_ShiftPhase(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Edit_Trim(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Edit_MergeChannel(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Edit_ReallySilenceSilentRegion(const YsString &fullCmd,YsConstArrayMask <YsString> argv);

	YSRESULT RunCommand_Filter(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Filter_Median(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Filter_EvenOut(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Filter_ExpandEnvelope(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_Filter_LowAndShortPeak(const YsString &fullCmd,YsConstArrayMask <YsString> argv);

public:
	YsWave_FM7Util::Option fm7UtilOption;
private:
	YSRESULT RunCommand_FM7(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_FM7_RepairPrecedingAndTrailingFF(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_FM7_ClearFileBlockGap(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_FM7_RepairByte(const YsString &fullCmd,YsConstArrayMask <YsString> argv);
	YSRESULT RunCommand_FM7_WaveLengthThr(const YsString &fullCmd,YsConstArrayMask <YsString> argv);

	void Error(const YsString &fullCmd,YsString msg);
};



/* } */
#endif
