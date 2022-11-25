#include "fsguiapp.h"
#include "yswave_waveutil.h"



void FsGuiMainCanvas::Select_Wave(FsGuiPopUpMenuItem *)
{
	auto &wav=GetCurrentWav();
	auto &wavRaw=wav.GetWave();
	auto sel=wav.GetSelection();

	YsWave_WaveUtil waveUtil;
	
	if(YSOK==waveUtil.DetectWave(wavRaw,GetCurrentChannel(),sel.minmax[0]))
	{
		auto rgn=waveUtil.GetRegion();
		sel.minmax[0]=rgn.minmax[0];
		sel.minmax[1]=rgn.minmax[1];
		wav.SetSelection(sel);
		SetNeedRedraw(YSTRUE);
	}
}
void FsGuiMainCanvas::Select_NextWave(FsGuiPopUpMenuItem *)
{
	auto &wav=GetCurrentWav();
	auto &wavRaw=wav.GetWave();
	auto sel=wav.GetSelection();
	auto viewport=wav.GetViewport();

	YsWave_WaveUtil waveUtil;
	if(YSOK==waveUtil.DetectWave(wavRaw,GetCurrentChannel(),sel.Max()+1))
	{
		auto rgn=waveUtil.GetRegion();
		sel.minmax[0]=rgn.minmax[0];
		sel.minmax[1]=rgn.minmax[1];
		wav.SetSelection(sel);

		viewport.zero=sel.Min()-viewport.wid/2;
		wav.SetViewport(viewport);

		SetNeedRedraw(YSTRUE);
	}
}
void FsGuiMainCanvas::Select_PrevWave(FsGuiPopUpMenuItem *)
{
	auto &wav=GetCurrentWav();
	auto &wavRaw=wav.GetWave();
	auto sel=wav.GetSelection();
	auto viewport=wav.GetViewport();

	YsWave_WaveUtil waveUtil;
	if(YSOK==waveUtil.DetectWaveBackward(wavRaw,GetCurrentChannel(),sel.Min()-1))
	{
		auto rgn=waveUtil.GetRegion();
		sel.minmax[0]=rgn.minmax[0];
		sel.minmax[1]=rgn.minmax[1];
		wav.SetSelection(sel);

		viewport.zero=sel.Min()-viewport.wid/2;
		wav.SetViewport(viewport);

		SetNeedRedraw(YSTRUE);
	}
}
