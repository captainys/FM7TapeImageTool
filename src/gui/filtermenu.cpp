#include "fsguiapp.h"



void FsGuiMainCanvas::Filter_Median(FsGuiPopUpMenuItem *)
{
	auto &waveEdit=GetCurrentWav();

	YsString str;
	str.Printf("FILTER MEDIAN %d",GetCurrentChannel());
	waveEdit.RunCommand(str);
	SetNeedRedraw(YSTRUE);
}



void FsGuiMainCanvas::Filter_ExpandEnvelope(FsGuiPopUpMenuItem *)
{
	auto &waveEdit=GetCurrentWav();

	YsString str;
	str.Printf("FILTER EXPAND_ENVELOPE %d",GetCurrentChannel());
	waveEdit.RunCommand(str);
	SetNeedRedraw(YSTRUE);
}
