#include <ysport.h>
#include <t77.h>
#include <fsguifiledialog.h>

#include "yswave_fm7util.h"

#include "fsguiapp.h"



void FsGuiMainCanvas::Analyze_DetectPeak(FsGuiPopUpMenuItem *)
{
	auto &waveEdit=GetCurrentWav();

	YsString str;
	str.Printf("ANALYZE DETECT_PEAK %d",GetCurrentChannel());
	waveEdit.RunCommand(str);
	SetNeedRedraw(YSTRUE);
}


////////////////////////////////////////////////////////////



class Analyze_FilterPeakDialog : public FsGuiDialog
{
public:
	FsGuiButton *okBtn,*cancelBtn;
	FsGuiTextBox *lowPeakThrTxt,*shortPeakThrTxt;
	FsGuiMainCanvas *owner;

	void Make(FsGuiMainCanvas *canvas);
	void OnButtonClick(FsGuiButton *btn);
};

void Analyze_FilterPeakDialog::Make(FsGuiMainCanvas *owner)
{
	FsGuiDialog::Initialize();

	this->owner=owner;

	okBtn=AddTextButton(MkId("OK"),FSKEY_ENTER,FSGUI_PUSHBUTTON,L"OK",YSTRUE);
	cancelBtn=AddTextButton(MkId("Cancel"),FSKEY_ENTER,FSGUI_PUSHBUTTON,L"Cancel",YSTRUE);

	lowPeakThrTxt=AddTextBox(MkId("lowPeakThr"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Low Peak Threshold",20,YSTRUE);
	shortPeakThrTxt=AddTextBox(MkId("shortPeakThr"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Short Peak Threshold",20,YSTRUE);

	lowPeakThrTxt->SetInteger(1638); // 5%
	shortPeakThrTxt->SetInteger(5);

	SetArrangeType(FSDIALOG_ARRANGE_TOP_LEFT);
	Fit();
}

void Analyze_FilterPeakDialog::OnButtonClick(FsGuiButton *btn)
{
	if(okBtn==btn)
	{
		owner->Analyze_FilterShortAndLowPeak_DoFilter(lowPeakThrTxt->GetInteger(),shortPeakThrTxt->GetInteger());
	}
	CloseModalDialog(0);
}

void FsGuiMainCanvas::Analyze_FilterShortAndLowPeak(FsGuiPopUpMenuItem *)
{
	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<Analyze_FilterPeakDialog>();
	dlg->Make(this);
	AttachModalDialog(dlg);
}

void FsGuiMainCanvas::Analyze_FilterShortAndLowPeak_DoFilter(unsigned int lowThr,unsigned int shortThr)
{
	auto &waveEdit=GetCurrentWav();

	YsString str;
	str.Printf("FILTER LOW_AND_SHORT_PEAK %d %d %d",GetCurrentChannel(),lowThr,shortThr);
	waveEdit.RunCommand(str);
	SetNeedRedraw(YSTRUE);
}


////////////////////////////////////////////////////////////



class FsGuiMainCanvas::LowPeakDialog : public FsGuiDialog
{
private:
	class PeakReference
	{
	public:
		YSSIZE_T peakIdx;
		int score;
	};
public:
	FsGuiMainCanvas *owner;

	std::vector <PeakReference> sortedPeak;
	bool sortedByHeight;
	int currentRank;

	FsGuiButton *closeBtn;
	FsGuiStatic *rankTxt;

	FsGuiButton *sortByHeightBtn,*sortByWidthBtn;

	FsGuiButton *prevBtn,*nextBtn;
	FsGuiButton *prev10Btn,*next10Btn;
	FsGuiButton *prev100Btn,*next100Btn;
	FsGuiButton *prev1000Btn,*next1000Btn;
	FsGuiButton *prev10000Btn,*next10000Btn;

	FsGuiButton *erasePeakBtn;

	void Make(FsGuiMainCanvas *owner);
	void SortByHeight(void);
	void SortByWidth(void);
	void Jump(YSSIZE_T sortedPeakIdx);
	void OnButtonClick(FsGuiButton *btn);
};

void FsGuiMainCanvas::LowPeakDialog::Make(FsGuiMainCanvas *owner)
{
	this->owner=owner;

	FsGuiDialog::Initialize();
	closeBtn=AddTextButton(MkId("close"),FSKEY_ENTER,FSGUI_PUSHBUTTON,L"Close",YSTRUE);

	rankTxt=AddStaticText(0,FSKEY_NULL,"Rank",YSTRUE);

	sortByHeightBtn=AddTextButton(MkId("sortByHeight"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Sort by Height",YSTRUE);
	sortByWidthBtn=AddTextButton(MkId("sortByWidth"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Sort by Width",YSFALSE);

	prev10000Btn=AddTextButton(MkId("prev10000"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"<<<<<<",YSTRUE);
	prev1000Btn=AddTextButton(MkId("prev1000"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"<<<<",YSFALSE);
	prev100Btn=AddTextButton(MkId("prev100"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"<<<",YSFALSE);
	prev10Btn=AddTextButton(MkId("prev10"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"<<",YSFALSE);
	prevBtn=AddTextButton(MkId("prev"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"<",YSFALSE);
	nextBtn=AddTextButton(MkId("next"),FSKEY_NULL,FSGUI_PUSHBUTTON,L">",YSFALSE);
	next10Btn=AddTextButton(MkId("next10"),FSKEY_NULL,FSGUI_PUSHBUTTON,L">>",YSFALSE);
	next100Btn=AddTextButton(MkId("next100"),FSKEY_NULL,FSGUI_PUSHBUTTON,L">>>",YSFALSE);
	next1000Btn=AddTextButton(MkId("next1000"),FSKEY_NULL,FSGUI_PUSHBUTTON,L">>>>",YSFALSE);
	next10000Btn=AddTextButton(MkId("next10000"),FSKEY_NULL,FSGUI_PUSHBUTTON,L">>>>>>",YSFALSE);

	erasePeakBtn=AddTextButton(MkId("erasePeak"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Erase Smaller Peaks",YSTRUE);

	SortByHeight();

	SetArrangeType(FSDIALOG_ARRANGE_TOP_LEFT);
	Fit();
}

void FsGuiMainCanvas::LowPeakDialog::SortByHeight(void)
{
	auto &wav=owner->GetCurrentWav();
	auto &wavRaw=wav.GetWave();
	auto &peak=wav.GetPeak();
	auto channel=owner->GetCurrentChannel();

	std::vector <int> score;
	sortedPeak.clear();

	for(YSSIZE_T idx=0; idx+1<peak.size(); ++idx)
	{
		if(peak[idx].isHigh!=peak[idx+1].isHigh)
		{
			auto level0=wavRaw.GetSignedValue16(channel,peak[idx].idx);
			auto level1=wavRaw.GetSignedValue16(channel,peak[idx+1].idx);
			auto diff=YsAbs(level1-level0);

			PeakReference ref;
			ref.peakIdx=idx;
			ref.score=diff;

			sortedPeak.push_back(ref);
			score.push_back(diff);
		}
	}

	YsSimpleMergeSort<int,PeakReference>(score.size(),score.data(),sortedPeak.data());
	sortedByHeight=true;
	currentRank=0;
	Jump(0);
}

void FsGuiMainCanvas::LowPeakDialog::SortByWidth(void)
{
	auto &wav=owner->GetCurrentWav();
	auto &wavRaw=wav.GetWave();
	auto &peak=wav.GetPeak();
	auto channel=owner->GetCurrentChannel();

	std::vector <int> score;
	sortedPeak.clear();

	for(YSSIZE_T idx=0; idx+1<peak.size(); ++idx)
	{
		if(peak[idx].isHigh!=peak[idx+1].isHigh)
		{
			auto diff=peak[idx+1].idx-peak[idx].idx;

			PeakReference ref;
			ref.peakIdx=idx;
			ref.score=diff;

			sortedPeak.push_back(ref);
			score.push_back(diff);
		}
	}

	YsSimpleMergeSort<int,PeakReference>(score.size(),score.data(),sortedPeak.data());
	sortedByHeight=false;
	currentRank=0;
	Jump(0);
}

void FsGuiMainCanvas::LowPeakDialog::Jump(YSSIZE_T sortedPeakIdx)
{
	if(0<sortedPeak.size())
	{
		sortedPeakIdx=YsBound<long long>(sortedPeakIdx,0,sortedPeak.size());
		auto peakIdx=sortedPeak[sortedPeakIdx].peakIdx;

		auto &wav=owner->GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto &peak=wav.GetPeak();

		auto viewport=wav.GetViewport();
		wav.SetViewportZero(peak[peakIdx].idx-viewport.wid/2);

		currentRank=sortedPeakIdx;

		YsString str;
		str.Printf("Rank:%lld  Diff:%lld",currentRank,sortedPeak[sortedPeakIdx].score);
		rankTxt->SetText(str);

		if(peakIdx+1<peak.size())
		{
			YsWaveKernel::Selection sel;
			sel.minmax[0]=peak[peakIdx].idx;
			sel.minmax[1]=peak[peakIdx+1].idx;
			wav.SetSelection(sel);
		}

		owner->SetNeedRedraw(YSTRUE);
	}
}

void FsGuiMainCanvas::LowPeakDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==closeBtn)
	{
		owner->RemoveDialog(this);
	}
	else if(btn==sortByHeightBtn)
	{
		SortByHeight();
	}
	else if(btn==sortByWidthBtn)
	{
		SortByWidth();
	}
	else if(btn==prevBtn)
	{
		Jump(currentRank-1);
	}
	else if(btn==nextBtn)
	{
		Jump(currentRank+1);
	}
	else if(btn==prev10Btn)
	{
		Jump(currentRank-10);
	}
	else if(btn==next10Btn)
	{
		Jump(currentRank+10);
	}
	else if(btn==prev100Btn)
	{
		Jump(currentRank-100);
	}
	else if(btn==next100Btn)
	{
		Jump(currentRank+100);
	}
	else if(btn==prev1000Btn)
	{
		Jump(currentRank-1000);
	}
	else if(btn==next1000Btn)
	{
		Jump(currentRank+1000);
	}
	else if(btn==prev10000Btn)
	{
		Jump(currentRank-10000);
	}
	else if(btn==next10000Btn)
	{
		Jump(currentRank+10000);
	}
	else if(btn==erasePeakBtn)
	{
		auto channel=owner->GetCurrentChannel();
		if(0<=currentRank && currentRank<sortedPeak.size())
		{
			YsString cmd;
			if(true==sortedByHeight)
			{
				cmd.Printf("FILTER EVENOUT %d HEIGHT %d",channel,sortedPeak[currentRank].score);
			}
			else
			{
				cmd.Printf("FILTER EVENOUT %d WIDTH %d",channel,sortedPeak[currentRank].score);
			}
			owner->RunCommand(cmd);
		}
	}
}

void FsGuiMainCanvas::Analyze_InspectPeak(FsGuiPopUpMenuItem *)
{
	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<LowPeakDialog>();
	dlg->Make(this);
	AddDialog(dlg);
	ArrangeDialog();
	SetNeedRedraw(YSTRUE);
}

////////////////////////////////////////////////////////////



class FsGuiMainCanvas::MarkSilentRegionDialog : public FsGuiDialog
{
public:
	FsGuiMainCanvas *owner;

	FsGuiButton *okBtn,*cancelBtn;
	FsGuiTextBox *levelThrTxt;
	FsGuiTextBox *durationThrTxt;

	void Make(FsGuiMainCanvas *owner);
	void OnButtonClick(FsGuiButton *btn);
};

void FsGuiMainCanvas::MarkSilentRegionDialog::Make(FsGuiMainCanvas *owner)
{
	this->owner=owner;

	FsGuiDialog::Initialize();
	okBtn=AddTextButton(MkId("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,L"OK",YSTRUE);
	cancelBtn=AddTextButton(MkId("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,L"Cancel",YSFALSE);

	levelThrTxt=AddTextBox(MkId("levelThr"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Level Threshold",20,YSTRUE);
	durationThrTxt=AddTextBox(MkId("durationThr"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Duration Threshold",20,YSTRUE);

	levelThrTxt->SetInteger(1638); // 5% of spectrum
	durationThrTxt->SetRealNumber(0.1833,1);   // 600bps, 1 byte=11-pulse long -> 0.1833 sec

	SetArrangeType(FSDIALOG_ARRANGE_TOP_LEFT);
	Fit();
}

void FsGuiMainCanvas::MarkSilentRegionDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==cancelBtn)
	{
		owner->RemoveDialog(this);
	}
	else if(btn==okBtn)
	{
		YsString str;

		str.Printf("ANALYZE MARK_SILENT %d %s %s",
			owner->GetCurrentChannel(),
			levelThrTxt->GetString().c_str(),
			durationThrTxt->GetString().c_str());
		owner->RunCommand(str);

		owner->RemoveDialog(this);
	}
}

void FsGuiMainCanvas::Analyze_MarkSilentRegion(FsGuiPopUpMenuItem *)
{
	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<MarkSilentRegionDialog>();
	dlg->Make(this);
	AddDialog(dlg);
	ArrangeDialog();
	SetNeedRedraw(YSTRUE);
}



////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Analyze_Unsilence(FsGuiPopUpMenuItem *)
{
	auto &waveEdit=GetCurrentWav();

	auto sel=waveEdit.GetSelection();
	auto selBegin=sel.minmax[0];
	auto selEnd=sel.minmax[1];

	if(selBegin>selEnd)
	{
		std::swap(selBegin,selEnd);
	}

	YsString str;
	str.Printf("ANALYZE UNSILENCE %d %lld %lld",GetCurrentChannel(),selBegin,selEnd);
	waveEdit.RunCommand(str);
	SetNeedRedraw(YSTRUE);
}

////////////////////////////////////////////////////////////



void FsGuiMainCanvas::Analyze_MarkLongWaveAsSilentFollowedByShortWave(FsGuiPopUpMenuItem *)
{
	// ANALYZE MARK_LONG_WAVE_SILENT ch longHumpThr shortHumpThr silenceThr

	unsigned int longHumpThr=80;
	unsigned int shortHumpThr=10;
	unsigned int trailingSilenceThr=2000;
	unsigned int followedByDownWave=1;
	YsString str;
	str.Printf("ANALYZE MARK_LONG_WAVE_SILENT %d %d %d %d %d",
		GetCurrentChannel(),
		longHumpThr,
		shortHumpThr,
		trailingSilenceThr,
		followedByDownWave); // Followed by Down Wave
	RunCommand(str);
	SetNeedRedraw(YSTRUE);
}


////////////////////////////////////////////////////////////



void FsGuiMainCanvas::Analyze_CalculateEnvelope(FsGuiPopUpMenuItem *)
{
	auto &waveEdit=GetCurrentWav();

	YsString str;
	str.Printf("ANALYZE CALCULATE_ENVELOPE %d",GetCurrentChannel());
	waveEdit.RunCommand(str);
	SetNeedRedraw(YSTRUE);
}



////////////////////////////////////////////////////////////



class FsGuiMainCanvas::FM7Dialog : public FsGuiDialog
{
public:
	typedef FM7Dialog THISCLASS;

	FsGuiMainCanvas *owner;
	FsGuiButton *okBtn;

	FsGuiTextBox *searchFromTxt;
	FsGuiButton *selBeginToSearchFrom;

	int lastErrorCode;
	long long lastErrorPtr;
	FsGuiStatic *lastErrorLocTxt;

	FsGuiTextBox *zeroMinThrTxt,*zeroMaxThrTxt,*oneMinThrTxt,*oneMaxThrTxt;

	FsGuiButton *getByteBtn;
	FsGuiButton *findLeadBtn;
	FsGuiButton *readBlockBtn;
	FsGuiButton *readFileBtn;
	FsGuiButton *skipFileBtn;
	FsGuiButton *readRawByteSequence;
	FsGuiButton *skipRawByteSequence;

	FsGuiButton *readSynclessRawByteSequence;
	FsGuiButton *skipSynclessRawByteSequence;

	FsGuiButton *jumpToLastErrorBtn;

	FsGuiButton *repairFFinFileBtn;
	FsGuiButton *cleanBlockGapBtn;
	FsGuiButton *repairNextByteBtn;

	FsGuiButton *saveT77Btn;
	FsGuiButton *forceSaveT77StdBtn;
	FsGuiButton *forceSaveT77NonStdBtn;
	FsGuiButton *saveRawBinaryBtn;

	FsGuiButton *silenceNonBIOSBytes;

	bool forceSave,standardWaveLength=true;

	void SetLastError(int errorCode,long long errorPtr);

	void Make(FsGuiMainCanvas *owner);
	void OnButtonClick(FsGuiButton *btn);

	void SendThresholdCommand(void);
	void SaveT77(void);
	void SaveT77_FileSelected(FsGuiDialog *dlg,int returnCode);
	void SaveT77_OverwriteConfirmed(FsGuiDialog *dlg,int returnCode);
	void SaveT77_Save(YsWString fName);
	void SaveT77_ForceSave(YsWString fName);

	void SaveRawBinary(void);
	void SaveRawBinary_FileSelected(FsGuiDialog *dlg,int returnCode);

	void SilenceNonBIOSBytes(void);
};

void FsGuiMainCanvas::FM7Dialog::Make(FsGuiMainCanvas *owner)
{
	lastErrorCode=0;
	lastErrorPtr=0;

	this->owner=owner;
	auto &wav=owner->GetCurrentWav();

	FsGuiDialog::Initialize();
	okBtn=AddTextButton(MkId("ok"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Close",YSFALSE);
	lastErrorLocTxt=AddStaticText(0,FSKEY_NULL,"Last Err at:0000000",YSFALSE);
	jumpToLastErrorBtn=AddTextButton(MkId("jumpToLastError"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Jump to Last Err",YSFALSE);

	AddStaticText(0,FSKEY_NULL,"Zero Bit Length",YSFALSE);
	zeroMinThrTxt=AddTextBox(MkId("zeroMinThr"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Min:",3,YSFALSE);
	zeroMaxThrTxt=AddTextBox(MkId("zeroMinThr"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Max:",3,YSFALSE);
	AddStaticText(0,FSKEY_NULL,"One Bit ",YSFALSE);
	oneMinThrTxt=AddTextBox(MkId("oneMinThr"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Min:",3,YSFALSE);
	oneMaxThrTxt=AddTextBox(MkId("oneMinThr"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Max:",3,YSFALSE);

	zeroMinThrTxt->SetInteger(wav.fm7UtilOption.zeroMinWaveLength);
	zeroMaxThrTxt->SetInteger(wav.fm7UtilOption.zeroMaxWaveLength);
	oneMinThrTxt->SetInteger(wav.fm7UtilOption.oneMinWaveLength);
	oneMaxThrTxt->SetInteger(wav.fm7UtilOption.oneMaxWaveLength);

	searchFromTxt=AddTextBox(0,FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Search From",10,YSTRUE);
	searchFromTxt->SetInteger(0);
	selBeginToSearchFrom=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,L"<<SelBegin",YSFALSE);

	findLeadBtn=AddTextButton(MkId("findLead"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Find Lead",YSFALSE);
	readBlockBtn=AddTextButton(MkId("readBlock"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Read Block",YSFALSE);
	readFileBtn=AddTextButton(MkId("readFile"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Read File",YSFALSE);
	skipFileBtn=AddTextButton(MkId("skipFile"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Skip File",YSFALSE);
	readRawByteSequence=AddTextButton(MkId("readRawByteSequence"),FSKEY_NULL,FSGUI_PUSHBUTTON,"Read Lead+Raw Bytes",YSFALSE);
	skipRawByteSequence=AddTextButton(MkId("skipRawByteSequence"),FSKEY_NULL,FSGUI_PUSHBUTTON,"Skip Lead+Raw Bytes",YSFALSE);

	readSynclessRawByteSequence=AddTextButton(MkId("readSynclessRawByteSequence"),FSKEY_NULL,FSGUI_PUSHBUTTON,"Read Raw Bytes without Sync",YSTRUE);
	skipSynclessRawByteSequence=AddTextButton(MkId("readSynclessRawByteSequence"),FSKEY_NULL,FSGUI_PUSHBUTTON,"Skip Raw Bytes without Sync",YSFALSE);

	getByteBtn=AddTextButton(MkId("getByte"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Get Byte (SEL)",YSTRUE);
	repairFFinFileBtn=AddTextButton(MkId("repairFFinFile"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Repair FF in File(SEL)",YSFALSE);
	cleanBlockGapBtn=AddTextButton(MkId("cleanBlockGap"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Clean Block Gap",YSFALSE);
	repairNextByteBtn=AddTextButton(MkId("repairNextByte"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Try Repair Next Byte",YSFALSE);

	saveT77Btn=AddTextButton(MkId("saveT77"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Save T77 file",YSFALSE);
	saveRawBinaryBtn=AddTextButton(MkId("saveRaw"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Save RAW Binary Streams",YSFALSE);

	forceSaveT77StdBtn=AddTextButton(MkId("forceSaveT77"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Force Save T77 file (Standard Wave Length)",YSTRUE);
	forceSaveT77NonStdBtn=AddTextButton(MkId("forceSaveT77NS"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Force Save T77 file (Non-Standard Wave Length)",YSFALSE);

	silenceNonBIOSBytes=AddTextButton(MkId("silenceNonBIOSBytes"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Silence Non-BIOS Bytes",YSTRUE);

	SetTransparency(YSTRUE);
	SetBackgroundAlpha(0.0);
	SetArrangeType(FSDIALOG_ARRANGE_TOP_LEFT);
	Fit();
}

void FsGuiMainCanvas::FM7Dialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==okBtn)
	{
		owner->RemoveDialog(this);
	}
	else if(btn==selBeginToSearchFrom)
	{
		auto &wav=owner->GetCurrentWav();
		auto sel=wav.GetSelection();
		searchFromTxt->SetInteger(sel.minmax[0]);
		owner->SetNeedRedraw(YSTRUE);
	}
	else if(btn==getByteBtn)
	{
		auto &wav=owner->GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto &peak=wav.GetPeak();
		auto channel=owner->GetCurrentChannel();
		auto sel=wav.GetSelection();

		SendThresholdCommand();

		YsWave_FM7Util fm7Util;
		auto byteData=fm7Util.ReadByte(wavRaw,channel,sel.minmax[0],wav.fm7UtilOption);
		if(YSOK==byteData.res)
		{
			printf("%d (%02xH)\n",byteData.byteData,byteData.byteData);
			sel.minmax[0]=byteData.minmax[0];
			sel.minmax[1]=byteData.minmax[1];
			wav.SetSelection(sel);
			owner->SetNeedRedraw(YSTRUE);
		}
	}
	else if(btn==findLeadBtn)
	{
		auto &wav=owner->GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto &peak=wav.GetPeak();
		auto channel=owner->GetCurrentChannel();
		auto fromPtr=searchFromTxt->GetInteger();

		SendThresholdCommand();

		YsWave_FM7Util fm7Util;
		auto lead=fm7Util.FindLead(wavRaw,channel,fromPtr,wav.fm7UtilOption);
		if(0<lead.numFF)
		{
			auto sel=wav.GetSelection();
			sel.minmax[0]=lead.minmax[0];
			sel.minmax[1]=lead.minmax[1];
			wav.SetSelection(sel);
			owner->SetNeedRedraw(YSTRUE);
		}
	}
	else if(btn==readBlockBtn)
	{
		auto &wav=owner->GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto &peak=wav.GetPeak();
		auto channel=owner->GetCurrentChannel();
		auto fromPtr=searchFromTxt->GetInteger();

		SendThresholdCommand();

		YsWave_FM7Util fm7Util;
		auto lead=fm7Util.FindLead(wavRaw,channel,fromPtr,wav.fm7UtilOption);
		if(0<lead.numFF)
		{
			auto blk=fm7Util.ReadBlock(wavRaw,channel,lead,wav.fm7UtilOption);
			if(YsWave_FM7Util::ERROR_NOERROR!=blk.errorCode)
			{
				SetLastError(blk.errorCode,blk.errorPtr);
			}
			auto sel=wav.GetSelection();
			sel.minmax[0]=blk.minmax[0];
			sel.minmax[1]=blk.minmax[1];
			wav.SetSelection(sel);
			owner->SetNeedRedraw(YSTRUE);
		}
	}
	else if(btn==readFileBtn)
	{
		auto &wav=owner->GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto &peak=wav.GetPeak();
		auto channel=owner->GetCurrentChannel();
		auto fromPtr=searchFromTxt->GetInteger();

		SendThresholdCommand();

		YsWave_FM7Util fm7Util;
		auto f=fm7Util.ReadFile(wavRaw,channel,fromPtr,wav.fm7UtilOption);
		if(0<f.block.size())
		{
			auto sel=wav.GetSelection();
			sel.minmax[0]=f.minmax[0];
			sel.minmax[1]=f.minmax[1];
			wav.SetSelection(sel);
			owner->SetNeedRedraw(YSTRUE);
			if(f.block.back().errorCode!=YsWave_FM7Util::ERROR_NOERROR)
			{
				auto &blk=f.block.back();
				SetLastError(blk.errorCode,blk.errorPtr);
			}
		}
		printf("----\n");
	}
	else if(btn==skipFileBtn)
	{
		auto &wav=owner->GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto &peak=wav.GetPeak();
		auto channel=owner->GetCurrentChannel();
		auto fromPtr=searchFromTxt->GetInteger();

		SendThresholdCommand();

		YsWave_FM7Util fm7Util;
		auto skip=fm7Util.ReadFile(wavRaw,channel,fromPtr,wav.fm7UtilOption);
		fromPtr=skip.minmax[1];
		searchFromTxt->SetInteger(fromPtr);

		auto f=fm7Util.ReadFile(wavRaw,channel,fromPtr,wav.fm7UtilOption);
		printf("%lld blocks.\n",f.block.size());
		if(0<f.block.size())
		{
			auto sel=wav.GetSelection();
			sel.minmax[0]=f.minmax[0];
			sel.minmax[1]=f.minmax[1];
			wav.SetSelection(sel);
			owner->SetNeedRedraw(YSTRUE);
			if(f.block.back().errorCode!=YsWave_FM7Util::ERROR_NOERROR)
			{
				auto &blk=f.block.back();
				SetLastError(blk.errorCode,blk.errorPtr);
			}
		}
		printf("----\n");
	}
	else if(btn==readRawByteSequence)
	{
		auto &wav=owner->GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto &peak=wav.GetPeak();
		auto channel=owner->GetCurrentChannel();
		auto fromPtr=searchFromTxt->GetInteger();

		SendThresholdCommand();

		YsWave_FM7Util fm7Util;
		auto lead=fm7Util.FindLead(wavRaw,channel,fromPtr,wav.fm7UtilOption);
		if(0<lead.numFF)
		{
			auto blk=fm7Util.ReadRawByteSequence(wavRaw,channel,lead,wav.fm7UtilOption);
			if(YsWave_FM7Util::ERROR_NOERROR!=blk.errorCode)
			{
				SetLastError(blk.errorCode,blk.errorPtr);
			}
			auto sel=wav.GetSelection();
			sel.minmax[0]=blk.minmax[0];
			sel.minmax[1]=blk.minmax[1];
			wav.SetSelection(sel);
			owner->SetNeedRedraw(YSTRUE);
		}
	}
	else if(btn==skipRawByteSequence)
	{
		auto &wav=owner->GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto &peak=wav.GetPeak();
		auto channel=owner->GetCurrentChannel();
		auto fromPtr=searchFromTxt->GetInteger();

		SendThresholdCommand();

		YsWave_FM7Util fm7Util;
		auto lead=fm7Util.FindLead(wavRaw,channel,fromPtr,wav.fm7UtilOption);
		if(0<lead.numFF)
		{
			auto blk=fm7Util.ReadRawByteSequence(wavRaw,channel,lead,wav.fm7UtilOption);

			fromPtr=blk.minmax[1];
			searchFromTxt->SetInteger(fromPtr);
			auto lead=fm7Util.FindLead(wavRaw,channel,fromPtr,wav.fm7UtilOption);
			if(0<lead.numFF)
			{
				auto blk=fm7Util.ReadRawByteSequence(wavRaw,channel,lead,wav.fm7UtilOption);
				if(YsWave_FM7Util::ERROR_NOERROR!=blk.errorCode)
				{
					SetLastError(blk.errorCode,blk.errorPtr);
				}

				searchFromTxt->SetInteger(blk.minmax[1]);

				auto sel=wav.GetSelection();
				sel.minmax[0]=blk.minmax[0];
				sel.minmax[1]=blk.minmax[1];
				wav.SetSelection(sel);
				owner->SetNeedRedraw(YSTRUE);
			}
		}
	}
	else if(btn==readSynclessRawByteSequence)
	{
		auto &wav=owner->GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto &peak=wav.GetPeak();
		auto channel=owner->GetCurrentChannel();
		auto fromPtr=searchFromTxt->GetInteger();

		SendThresholdCommand();

		YsWave_FM7Util fm7Util;

		auto blk=fm7Util.ReadBareByteSequence(wavRaw,channel,fromPtr,wav.fm7UtilOption);
		if(YsWave_FM7Util::ERROR_NOERROR!=blk.errorCode)
		{
			SetLastError(blk.errorCode,blk.errorPtr);
		}

		printf("%zd bytes\n",blk.dump.size());

		auto sel=wav.GetSelection();
		sel.minmax[0]=blk.minmax[0];
		sel.minmax[1]=blk.minmax[1];
		wav.SetSelection(sel);
		owner->SetNeedRedraw(YSTRUE);
	}
	else if(btn==skipSynclessRawByteSequence)
	{
		auto &wav=owner->GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto &peak=wav.GetPeak();
		auto channel=owner->GetCurrentChannel();
		auto fromPtr=searchFromTxt->GetInteger();

		SendThresholdCommand();

		YsWave_FM7Util fm7Util;

		auto blk=fm7Util.ReadBareByteSequence(wavRaw,channel,fromPtr,wav.fm7UtilOption);

		if(YsWave_FM7Util::ERROR_NOERROR!=blk.errorCode)
		{
			SetLastError(blk.errorCode,blk.errorPtr);
		}
		auto sel=wav.GetSelection();
		sel.minmax[0]=blk.minmax[0];
		sel.minmax[1]=blk.minmax[1];
		wav.SetSelection(sel);
		owner->SetNeedRedraw(YSTRUE);

		searchFromTxt->SetInteger(blk.minmax[1]);
	}
	else if(btn==repairFFinFileBtn)
	{
		auto channel=owner->GetCurrentChannel();
		auto &wav=owner->GetCurrentWav();
		auto sel=wav.GetSelection();
		YsString str;
		str.Printf("FM7 REPAIR_PREPOSTFF %d %lld",channel,sel.Min());
		wav.RunCommand(str);
		owner->SetNeedRedraw(YSTRUE);
	}
	else if(btn==cleanBlockGapBtn)
	{
		auto channel=owner->GetCurrentChannel();
		auto &wav=owner->GetCurrentWav();
		auto sel=wav.GetSelection();
		YsString str;
		str.Printf("FM7 CLEAR_BLOCKGAP %d %lld",channel,sel.Min());
		wav.RunCommand(str);
		owner->SetNeedRedraw(YSTRUE);
	}
	else if(btn==jumpToLastErrorBtn)
	{
		auto &wav=owner->GetCurrentWav();
		auto viewport=wav.GetViewport();
		viewport.zero=lastErrorPtr-viewport.wid/2;
		wav.SetViewport(viewport);
		owner->SetNeedRedraw(YSTRUE);
	}
	else if(btn==repairNextByteBtn)
	{
		auto &wav=owner->GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto channel=owner->GetCurrentChannel();
		auto sel=wav.GetSelection();

		YsWave_FM7Util fm7Util;
		auto opt=wav.fm7UtilOption;
		auto next11=fm7Util.GetNext11Wave(wavRaw,channel,sel.Max(),opt);
		for(int i=0; i<11; ++i)
		{
			printf("[%d] %lld ",i,next11.wavePtr[i+1]-next11.wavePtr[i]);
			if(true==next11.errorBit[i])
			{
				printf("Error");
			}
			printf("\n");
		}

		SendThresholdCommand();

		YsString str;
		str.Printf("FM7 REPAIR_BYTE %d %lld\n",channel,sel.Max());

		wav.RunCommand(str);
		owner->SetNeedRedraw(YSTRUE);
	}
	else if(btn==saveT77Btn)
	{
		forceSave=false;
		SaveT77();
	}
	else if(btn==forceSaveT77StdBtn)
	{
		forceSave=true;
		standardWaveLength=true;
		SaveT77();
	}
	else if(btn==forceSaveT77NonStdBtn)
	{
		forceSave=true;
		standardWaveLength=false;
		SaveT77();
	}
	else if(btn==saveRawBinaryBtn)
	{
		SaveRawBinary();
	}
	else if(btn==silenceNonBIOSBytes)
	{
		SilenceNonBIOSBytes();
	}
}

void FsGuiMainCanvas::FM7Dialog::SetLastError(int errorCode,long long errorPtr)
{
	lastErrorCode=errorCode;
	lastErrorPtr=errorPtr;
	if(YsWave_FM7Util::ERROR_NOERROR!=errorCode)
	{
		YsString msg;
		msg.Printf("Last Err at:%lld",errorPtr);
		lastErrorLocTxt->SetText(msg);
	}
	else
	{
		lastErrorLocTxt->SetText("");
	}
}


void FsGuiMainCanvas::FM7Dialog::SendThresholdCommand(void)
{
	YsString cmd;
	cmd.Printf("FM7 WAVELENGTH_THR %d %d %d %d\n",
		(int)zeroMinThrTxt->GetInteger(),
		(int)zeroMaxThrTxt->GetInteger(),
		(int)oneMinThrTxt->GetInteger(),
		(int)oneMaxThrTxt->GetInteger());
	owner->GetCurrentWav().RunCommand(cmd);
}

void FsGuiMainCanvas::FM7Dialog::SaveT77(void)
{
	auto fdlg=FsGuiDialog::CreateSelfDestructiveDialog <FsGuiFileDialog> ();

	YsWString lastPath,lastFile;
	owner->lastUsedFileName.SeparatePathFile(lastPath,lastFile);

	if(0==lastPath.Strlen())
	{
		lastPath=YsSpecialPath::GetUserDocDirW();
	}
	if(0==lastFile.Strlen())
	{
		lastFile=L"newfile.t77";
	}
	lastFile.ReplaceExtension(L".t77");

	YsWString def;
	def.MakeFullPathName(lastPath,lastFile);

	fdlg->Initialize();
	fdlg->mode=FsGuiFileDialog::MODE_SAVE;
	fdlg->multiSelect=YSFALSE;
	fdlg->title.Set(L"Save T77");
	fdlg->fileExtensionArray.Append(L".t77");
	fdlg->defaultFileName=def;
	fdlg->BindCloseModalCallBack(&THISCLASS::SaveT77_FileSelected,this);
	AttachModalDialog(fdlg);
}
void FsGuiMainCanvas::FM7Dialog::SaveT77_FileSelected(FsGuiDialog *dlg,int returnCode)
{
	auto fdlg=dynamic_cast <FsGuiFileDialog *> (dlg);
	if(nullptr!=fdlg && (int)YSOK==returnCode && 1==fdlg->selectedFileArray.size())
	{
		YsWString fName=fdlg->selectedFileArray[0];
		printf("Selected %ls\n",fName.c_str());

		YsFileIO::File fp(fName,"rb");
		if(nullptr!=fp)
		{
			auto msgDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiMessageBoxDialogWithPayload <YsWString> >();
			msgDialog->payload=fName;

			YsWString msg;
			msg.Append(fName);
			msg.Append('\n');
			msg.Append(L"File Exists. Overwrite?");
			msgDialog->Make(L"Confirm Overwrite?",msg,L"Yes, Overwrite.",L"No, don't overwrite.");
			AttachModalDialog(msgDialog);
			msgDialog->BindCloseModalCallBack(&THISCLASS::SaveT77_OverwriteConfirmed,this);
		}
		else
		{
			if(true!=forceSave)
			{
				this->SaveT77_Save(fName);
			}
			else
			{
				this->SaveT77_ForceSave(fName);
			}
		}
	}
}
void FsGuiMainCanvas::FM7Dialog::SaveT77_OverwriteConfirmed(FsGuiDialog *dlg,int returnCode)
{
	auto msgDialog=dynamic_cast <FsGuiMessageBoxDialogWithPayload <YsWString> *>(dlg);
	if(nullptr!=msgDialog && (int)YSOK==returnCode)
	{
		if(true!=forceSave)
		{
			this->SaveT77_Save(msgDialog->payload);
		}
		else
		{
			this->SaveT77_ForceSave(msgDialog->payload);
		}
	}
}
void FsGuiMainCanvas::FM7Dialog::SaveT77_Save(YsWString fName)
{
	auto &wav=owner->GetCurrentWav();
	auto &wavRaw=wav.GetWave();
	auto &peak=wav.GetPeak();
	auto channel=owner->GetCurrentChannel();

	T77Encoder encoder;

	encoder.StartT77Header();

	long long ptr=0;
	while(ptr<wavRaw.GetNumSamplePerChannel())
	{
		YsWave_FM7Util fm7Util;
		auto opt=wav.fm7UtilOption;
		auto f=fm7Util.ReadFile(wavRaw,channel,ptr,opt);
		printf("%lld blocks.\n",f.block.size());
		if(0<f.block.size() && true==f.IsFBASICFile())
		{
			printf("*Save as an F-BASIC file\n");

			for(auto &blk : f.block)
			{
				for(int i=0; i<blk.nLeadFF; ++i)
				{
					encoder.AddByte(0xff);
				}
				for(auto c : blk.dump)
				{
					encoder.AddByte(c);
				}
				for(int i=0; i<blk.nTrailFF; ++i)
				{
					encoder.AddByte(0xff);
				}
			}
			ptr=f.block.back().minmax[1];
		}
		else
		{
		#if 1
			// Try reading as a bare (sync-less) data block.
			YsWave_FM7Util fm7Util;
			auto blk=fm7Util.ReadBareByteSequence(wavRaw,channel,ptr,wav.fm7UtilOption);
			if(0==blk.dump.size())
			{
				break;
			}

			printf("* Save as Raw Data Block  %lld bytes\n",(long long int)blk.dump.size());

			for(auto c : blk.dump)
			{
				encoder.AddByte(c);
			}

			ptr=blk.minmax[1];
		#else
			// Try reading as a raw data block.
			auto lead=fm7Util.FindLead(wavRaw,channel,ptr,wav.fm7UtilOption);
			if(0<lead.numFF)
			{
				auto opt=wav.fm7UtilOption;
				auto blk=fm7Util.ReadRawByteSequence(wavRaw,channel,lead,wav.fm7UtilOption);

				printf("* Save as Raw Data Block %lld FFs and %lld bytes\n",
				    (long long int)blk.nLeadFF,
				    (long long int)blk.dump.size());

				for(int i=0; i<blk.nLeadFF; ++i)
				{
					encoder.AddByte(0xff);
				}
				for(auto c : blk.dump)
				{
					encoder.AddByte(c);
				}

				ptr=blk.minmax[1];
			}
			else
			{
				break;
			}
		#endif
		}

		encoder.AddGapBetweenFile();
	}

	YsFileIO::File fp(fName,"wb");
	if(nullptr!=fp)
	{
		fwrite(encoder.t77.data(),1,encoder.t77.size(),fp);
	}
}

void FsGuiMainCanvas::FM7Dialog::SaveT77_ForceSave(YsWString fName)
{
	auto &wav=owner->GetCurrentWav();
	auto &wavRaw=wav.GetWave();
	auto &peak=wav.GetPeak();
	auto channel=owner->GetCurrentChannel();

	YsWave_FM7Util fm7Util;
	auto opt=wav.fm7UtilOption;
	opt.standardWaveLength=standardWaveLength;

	auto t77=fm7Util.EncodeT77BitWise(wavRaw,channel,opt);
	YsFileIO::File fp(fName,"wb");
	if(nullptr!=fp)
	{
		fwrite(t77.data(),1,t77.size(),fp);
	}
}

void FsGuiMainCanvas::Analyze_FM7(FsGuiPopUpMenuItem *)
{
	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog <FM7Dialog>();
	dlg->Make(this);
	AddDialog(dlg);
	ArrangeDialog();
	SetNeedRedraw(YSTRUE);
}

void FsGuiMainCanvas::FM7Dialog::SaveRawBinary(void)
{
	auto fdlg=FsGuiDialog::CreateSelfDestructiveDialog <FsGuiFileDialog> ();

	YsWString lastPath,lastFile;
	owner->lastUsedFileName.SeparatePathFile(lastPath,lastFile);

	if(0==lastPath.Strlen())
	{
		lastPath=YsSpecialPath::GetUserDocDirW();
	}
	if(0==lastFile.Strlen())
	{
		lastFile=L"newfile.bin";
	}
	lastFile.ReplaceExtension(L".bin");

	YsWString def;
	def.MakeFullPathName(lastPath,lastFile);

	fdlg->Initialize();
	fdlg->mode=FsGuiFileDialog::MODE_SAVE;
	fdlg->multiSelect=YSFALSE;
	fdlg->title.Set(L"Save Raw Binary Byte Streams");
	fdlg->fileExtensionArray.Append(L".bin");
	fdlg->defaultFileName=def;
	fdlg->BindCloseModalCallBack(&THISCLASS::SaveRawBinary_FileSelected,this);
	AttachModalDialog(fdlg);
}
void FsGuiMainCanvas::FM7Dialog::SaveRawBinary_FileSelected(FsGuiDialog *dlg,int returnCode)
{
	auto fdlg=dynamic_cast <FsGuiFileDialog *> (dlg);
	if(nullptr!=fdlg && (int)YSOK==returnCode && 1==fdlg->selectedFileArray.size())
	{
		YsWString fName=fdlg->selectedFileArray[0];
		printf("Selected %ls\n",fName.c_str());

		fName.RemoveExtension();

		int fileCtr=0;
		auto &wav=owner->GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto &peak=wav.GetPeak();
		auto channel=owner->GetCurrentChannel();

		long long ptr=0;
		while(ptr<wavRaw.GetNumSamplePerChannel())
		{
			YsArray <unsigned char> bin;

			YsWave_FM7Util fm7Util;
			auto opt=wav.fm7UtilOption;
			auto f=fm7Util.ReadFile(wavRaw,channel,ptr,opt);
			printf("%lld blocks.\n",f.block.size());
			if(0<f.block.size() && true==f.IsFBASICFile())
			{
				printf("*Save as an F-BASIC file\n");

				for(auto &blk : f.block)
				{
					for(int i=0; i<blk.nLeadFF; ++i)
					{
						bin.push_back(0xff);
					}
					for(auto c : blk.dump)
					{
						bin.push_back(c);
					}
					for(int i=0; i<blk.nTrailFF; ++i)
					{
						bin.push_back(0xff);
					}
				}
				ptr=f.block.back().minmax[1];
			}
			else
			{
				// Try reading as a raw data block.
				YsWave_FM7Util fm7Util;
				auto lead=fm7Util.FindLead(wavRaw,channel,ptr,wav.fm7UtilOption);
				if(0<lead.numFF)
				{
					auto opt=wav.fm7UtilOption;
					auto blk=fm7Util.ReadRawByteSequence(wavRaw,channel,lead,wav.fm7UtilOption);

					printf("* Save as Raw Data Block %lld FFs and %lld bytes\n",
					    (long long int)blk.nLeadFF,
					    (long long int)blk.dump.size());

					for(int i=0; i<blk.nLeadFF; ++i)
					{
						bin.push_back(0xff);
					}
					for(auto c : blk.dump)
					{
						bin.push_back(c);
					}

					ptr=blk.minmax[1];
				}
				else
				{
					break;
				}
			}

			if(0<bin.size())
			{
				YsString numStr;
				numStr.Printf("_%d.bin",fileCtr++);
				YsWString wNumStr,ful;
				wNumStr.SetUTF8String(numStr);
				ful=fName+wNumStr;
				printf("%ls\n",ful.c_str());

				YsFileIO::File ofp(ful,"wb");
				if(nullptr!=ofp)
				{
					fwrite(bin.data(),1,bin.size(),ofp);
				}
			}
		}
	}
}

void FsGuiMainCanvas::FM7Dialog::SilenceNonBIOSBytes(void)
{
	auto &wav=owner->GetCurrentWav();
	auto &wavRaw=wav.GetWave();
	auto &peak=wav.GetPeak();
	auto channel=owner->GetCurrentChannel();
	auto sel=wav.GetSelection();

	SendThresholdCommand();

	YsLoopCounter ctr;
	ctr.BeginCounter(wavRaw.GetNumSamplePerChannel());

	long long ptr=0;
	while(ptr<wavRaw.GetNumSamplePerChannel())
	{
		ctr.ShowCounter(ptr);

		YsWave_FM7Util fm7Util;
		auto blk=fm7Util.ReadBareByteSequence(wavRaw,channel,ptr,wav.fm7UtilOption);
		if(0!=blk.dump.size())
		{
			if(ptr+1<blk.minmax[0])
			{
				YsString str;
				str.Printf("EDIT SILENCE %d %lld %lld",channel,ptr,blk.minmax[0]);
				printf("%s\n",str.Txt());
				wav.RunCommand(str);
			}
			ptr=blk.minmax[1]+1;
		}
		else
		{
			// Nothing is found
			if(ptr+1<wavRaw.GetNumSamplePerChannel())
			{
				YsString str;
				str.Printf("EDIT SILENCE %d %lld %lld",channel,ptr,wavRaw.GetNumSamplePerChannel());
				printf("%s\n",str.Txt());
				wav.RunCommand(str);
			}
			break;
		}
	}

	ctr.EndCounter();

	YsString str;
	str.Printf("EDIT REALLY_SILENCE_SILENT_REGIONS %d",channel);
	wav.RunCommand(str);
}
