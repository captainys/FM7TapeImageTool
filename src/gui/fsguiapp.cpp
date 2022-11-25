/* ////////////////////////////////////////////////////////////

File Name: fsguiapp.cpp
Copyright (c) 2017 Soji Yamakawa.  All rights reserved.
http://www.ysflight.com

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//////////////////////////////////////////////////////////// */

#include <ysclass.h>
#include <ysport.h>
#include <yscompilerwarning.h>
#include <ysgl.h>
#include <ysglslcpp.h>

#include "fsguiapp.h"





static FsGuiMainCanvas *appMainCanvas;

////////////////////////////////////////////////////////////

YsHasWaveSound::YsHasWaveSound()
{
	channel=0;
}
const YsWaveEdit &YsHasWaveSound::GetCurrentWav(void) const
{
	return wav;
}
YsWaveEdit &YsHasWaveSound::GetCurrentWav(void)
{
	return wav;
}

int YsHasWaveSound::GetCurrentChannel(void) const
{
	return channel;
}

void YsHasWaveSound::SetCurrentChannel(int channel)
{
	this->channel=channel;
}

////////////////////////////////////////////////////////////

FsGuiMainCanvas::FsGuiMainCanvas()
{
	appMustTerminate=YSFALSE;
	temporaryPhaseOffset=0;

	MakeMainMenu();
}

FsGuiMainCanvas::~FsGuiMainCanvas()
{
	// The following two lines ensure that all self-destructive dialogs are cleaned. 2015/03/18
	RemoveDialogAll();
	PerformScheduledDeletion();

	DeleteMainMenu();
}

void FsGuiMainCanvas::Initialize(int argc,char *argv[])
{
	YsDisregardVariable(argc);
	YsDisregardVariable(argv);
	YsGLSLRenderer::CreateSharedRenderer();
}

void FsGuiMainCanvas::MakeMainMenu(void)
{
	mainMenu=new FsGuiPopUpMenu;
	mainMenu->Initialize();
	mainMenu->SetIsPullDownMenu(YSTRUE);

	{
		FsGuiPopUpMenuItem *fileMenu=mainMenu->AddTextItem(0,FSKEY_F,L"File");
		FsGuiPopUpMenu *fileSubMenu=fileMenu->GetSubMenu();

		fileSubMenu->AddTextItem(mainMenu->MkId("file/open")  ,FSKEY_O,L"Open")->BindCallBack(&THISCLASS::File_Open,this);
		fileSubMenu->AddTextItem(mainMenu->MkId("file/save")  ,FSKEY_S,L"Save")->BindCallBack(&THISCLASS::File_Save,this);
		fileSubMenu->AddTextItem(mainMenu->MkId("file/saveas"),FSKEY_A,L"Save As")->BindCallBack(&THISCLASS::File_SaveAs,this);

		fileSubMenu->AddTextItem(mainMenu->MkId("file/append")  ,FSKEY_NULL,L"Append")->BindCallBack(&THISCLASS::File_Append,this);

		fileSubMenu->AddTextItem(mainMenu->MkId("file/recallCmdLog"),FSKEY_NULL,L"Recall Command Log")->BindCallBack(&THISCLASS::File_RecallCommandLog,this);
		fileSubMenu->AddTextItem(mainMenu->MkId("file/saveCmdLog")  ,FSKEY_NULL,L"Save Command Log")->BindCallBack(&THISCLASS::File_SaveCommandLog,this);

		fileSubMenu->AddTextItem(mainMenu->MkId("file/exit")  ,FSKEY_X,L"Exit")->BindCallBack(&THISCLASS::File_Exit,this);

		fileRecent=fileSubMenu->AddTextItem(0,FSKEY_R,L"Recent Files...")->AddSubMenu();
		RefreshRecentlyUsedFileList();
	}

	{
		auto editMenu=mainMenu->AddTextItem(0,FSKEY_E,L"Edit")->AddSubMenu();
		editMenu->AddTextItem(mainMenu->MkId("edit/makeSilence"),FSKEY_0,L"Make Silence")->BindCallBack(&THISCLASS::Edit_MakeSilence,this);
		editMenu->AddTextItem(mainMenu->MkId("edit/deleteChannel"),FSKEY_NULL,L"Delete Channel")->BindCallBack(&THISCLASS::Edit_DeleteChannel,this);
		editMenu->AddTextItem(mainMenu->MkId("edit/mergeChannel"),FSKEY_NULL,L"Merge Channels to 1 Channel")->BindCallBack(&THISCLASS::Edit_MergeToOneChannel,this);
		editMenu->AddTextItem(mainMenu->MkId("edit/makesinehigh"),FSKEY_NULL,L"Make Sine Wave High First")->BindCallBack(&THISCLASS::Edit_MakeSineWaveHighFirst,this);
		editMenu->AddTextItem(mainMenu->MkId("edit/makesinelow"),FSKEY_NULL,L"Make Sine Wave Low First")->BindCallBack(&THISCLASS::Edit_MakeSineWaveLowFirst,this);
		editMenu->AddTextItem(mainMenu->MkId("edit/shiftphase"),FSKEY_NULL,L"Shift Phase")->BindCallBack(&THISCLASS::Edit_ShiftPhase,this);
		editMenu->AddTextItem(mainMenu->MkId("edit/trim"),FSKEY_NULL,L"Trim to Selection")->BindCallBack(&THISCLASS::Edit_Trim,this);
	}

	{
		auto selectMenu=mainMenu->AddTextItem(0,FSKEY_S,L"Select")->AddSubMenu();
		{
			auto selectWaveMenu=selectMenu->AddTextItem(0,FSKEY_W,L"Wave")->AddSubMenu();
			selectWaveMenu->AddTextItem(mainMenu->MkId("select/wave/wave"),FSKEY_W,L"Wave")->BindCallBack(&THISCLASS::Select_Wave,this);
			selectWaveMenu->AddTextItem(mainMenu->MkId("select/wave/next"),FSKEY_N,L"Next Wave")->BindCallBack(&THISCLASS::Select_NextWave,this);
			selectWaveMenu->AddTextItem(mainMenu->MkId("select/wave/prev"),FSKEY_P,L"Prev Wave")->BindCallBack(&THISCLASS::Select_PrevWave,this);
		}
	}

	{
		auto viewSubMenu=mainMenu->AddTextItem(0,FSKEY_V,L"View")->AddSubMenu();
		viewSubMenu->AddTextItem(mainMenu->MkId("view/zoom"),FSKEY_Z,L"Zoom")->BindCallBack(&THISCLASS::View_Zoom,this);
		viewSubMenu->AddTextItem(mainMenu->MkId("view/mooz"),FSKEY_M,L"Mooz")->BindCallBack(&THISCLASS::View_Mooz,this);
		viewSubMenu->AddTextItem(mainMenu->MkId("view/left"),FSKEY_NULL,L"Move Left")->BindCallBack(&THISCLASS::View_MoveLeft,this);
		viewSubMenu->AddTextItem(mainMenu->MkId("view/right"),FSKEY_NULL,L"Move Right")->BindCallBack(&THISCLASS::View_MoveRight,this);

		BindKeyCallBack(FSKEY_Z,YSFALSE,YSFALSE,YSFALSE,&THISCLASS::View_Zoom,this);
		BindKeyCallBack(FSKEY_M,YSFALSE,YSFALSE,YSFALSE,&THISCLASS::View_Mooz,this);
		BindKeyCallBack(FSKEY_LEFT,YSFALSE,YSFALSE,YSFALSE,&THISCLASS::View_MoveLeft,this);
		BindKeyCallBack(FSKEY_RIGHT,YSFALSE,YSFALSE,YSFALSE,&THISCLASS::View_MoveRight,this);

		viewSubMenu->AddTextItem(mainMenu->MkId("view/channel0"),FSKEY_NULL,L"Channel 0")->BindCallBack(&THISCLASS::View_Channel0,this);
		viewSubMenu->AddTextItem(mainMenu->MkId("view/channel1"),FSKEY_NULL,L"Channel 1")->BindCallBack(&THISCLASS::View_Channel1,this);
		drawAllChannel=viewSubMenu->AddTextItem(mainMenu->MkId("view/allchannel"),FSKEY_NULL,L"Show Non-Current Channel");
		drawAllChannel->BindCallBack(&THISCLASS::View_DrawNonCurrent,this);
		drawAllChannel->SetCheck(YSTRUE);

		viewSubMenu->AddTextItem(mainMenu->MkId("view/jumpTo"),FSKEY_G,L"Go To")->BindCallBack(&THISCLASS::View_JumpTo,this);
		viewSubMenu->AddTextItem(mainMenu->MkId("view/jumpSelBegin"),FSKEY_B,L"Go to Selection-Left")->BindCallBack(&THISCLASS::View_JumpToSelBegin,this);
		viewSubMenu->AddTextItem(mainMenu->MkId("view/jumpSelEnd"),FSKEY_E,L"Go to Selection-Right")->BindCallBack(&THISCLASS::View_JumpToSelEnd,this);
	}

	{
		auto analyzeMenu=mainMenu->AddTextItem(0,FSKEY_A,L"Automatic")->AddSubMenu();
	}

	{
		auto manualMenu=mainMenu->AddTextItem(0,FSKEY_M,L"Manual")->AddSubMenu();

		manualMenu->AddTextItem(mainMenu->MkId("manualanalyze/markSilence"),FSKEY_NULL,L"Mark Silent Segments")->BindCallBack(&THISCLASS::Analyze_MarkSilentRegion,this);
		manualMenu->AddTextItem(mainMenu->MkId("manual/reallySilence"),FSKEY_NULL,L"Make Silent Region Really Silent.")->BindCallBack(&THISCLASS::Edit_ReallySilenceSilentSegment,this);

		manualMenu->AddTextItem(mainMenu->MkId("manualfilter/median"),FSKEY_M,L"Median Filter")->BindCallBack(&THISCLASS::Filter_Median,this);


		manualMenu->AddTextItem(mainMenu->MkId("manual/detectPeak"),FSKEY_P,L"Detect Peaks")->BindCallBack(&THISCLASS::Analyze_DetectPeak,this);
		manualMenu->AddTextItem(mainMenu->MkId("manual/filterPeak"),FSKEY_F,L"Filter Peaks")->BindCallBack(&THISCLASS::Analyze_FilterShortAndLowPeak,this);
		manualMenu->AddTextItem(mainMenu->MkId("manual/analyzePeak"),FSKEY_NULL,L"Analyze Low/Narrow Peaks")->BindCallBack(&THISCLASS::Analyze_InspectPeak,this);
		manualMenu->AddTextItem(mainMenu->MkId("manual/markUnsilence"),FSKEY_NULL,L"Unmark Silent Segments from Selection")->BindCallBack(&THISCLASS::Analyze_Unsilence,this);
		manualMenu->AddTextItem(mainMenu->MkId("manual/silenceLongToWave"),FSKEY_NULL,L"Mark Long Wave as Silent -> Downward Short Wave")->BindCallBack(&THISCLASS::Analyze_MarkLongWaveAsSilentFollowedByShortWave,this);
		manualMenu->AddTextItem(mainMenu->MkId("manual/calculateEnvelope"),FSKEY_E,L"Calculate Envelope")->BindCallBack(&THISCLASS::Analyze_CalculateEnvelope,this);

		manualMenu->AddTextItem(mainMenu->MkId("manual/expand_envelope"),FSKEY_X,L"Expand Envelope")->BindCallBack(&THISCLASS::Filter_ExpandEnvelope,this);
	}

	{
		auto fm7Menu=mainMenu->AddTextItem(0,FSKEY_7,L"FM7")->AddSubMenu();

		fm7Menu->AddTextItem(mainMenu->MkId("fm7/fm7"),FSKEY_NULL,L"FM7 Tape Dialog")->BindCallBack(&THISCLASS::Analyze_FM7,this);
	}

	SetMainMenu(mainMenu);
}

void FsGuiMainCanvas::DeleteMainMenu(void)
{
	delete mainMenu;
}

void FsGuiMainCanvas::Save(YsWString fName,YSBOOL updateRecent)
{
	/* Implement file-saving here. */
	auto &wav=GetCurrentWav();
	if(YSOK!=wav.SaveWav(fName))
	{
		auto msgDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiMessageBoxDialog>();
		msgDialog->Make(L"Error!",L"Failed to Save File.",L"OK.",nullptr,nullptr);
		AttachModalDialog(msgDialog);
	}
	else
	// if no error
	{
		if(YSTRUE==updateRecent)
		{
			AddRecentlyUsedFile(fName);
		}
		lastUsedFileName=fName;

		YsWString msg;
		msg.Append(L"Saved.\n");
		msg.Append(fName);

		auto msgDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiMessageBoxDialog>();
		msgDialog->Make(L"Saved.",msg,L"OK.",nullptr,nullptr);
		AttachModalDialog(msgDialog);

	}
}
void FsGuiMainCanvas::Open(YsWString fName,YSBOOL updateRecent)
{
	/* Implement file-opening here. */
	auto &wav=GetCurrentWav();
	if(YSOK!=wav.LoadWav(fName))
	{
		auto msgDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiMessageBoxDialog>();
		msgDialog->Make(L"Error!",L"Failed to Open File.",L"OK.",nullptr,nullptr);
		AttachModalDialog(msgDialog);
	}
	else
	// if no error
	{
		if(YSTRUE==updateRecent)
		{
			AddRecentlyUsedFile(fName);
		}
		lastUsedFileName=fName;
	}
}

void FsGuiMainCanvas::SaveCommandLog(YsWString fName,YSBOOL updateRecent)
{
	YSRESULT res=YSOK;
	YsFileIO::File fp(fName,"w");
	if(nullptr!=fp)
	{
		auto &wav=GetCurrentWav();
		auto &cmdLog=wav.GetCommandLog();
		for(auto &str : cmdLog)
		{
			fprintf(fp,"%s\n",str.c_str());
		}

		YsWString msg;
		msg.Append(L"Saved.\n");
		msg.Append(fName);

		auto msgDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiMessageBoxDialog>();
		msgDialog->Make(L"Saved.",msg,L"OK.",nullptr,nullptr);
		AttachModalDialog(msgDialog);
	}
	else
	{
		auto msgDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiMessageBoxDialog>();
		msgDialog->Make(L"Error!",L"Failed to Save File.",L"OK.",nullptr,nullptr);
		AttachModalDialog(msgDialog);
		res=YSERR;
	}
	// if no error
	if(YSOK==res)
	{
		if(YSTRUE==updateRecent)
		{
			AddRecentlyUsedFile(fName);
		}
	}
}
void FsGuiMainCanvas::RecallCommandLog(YsWString fName,YSBOOL updateRecent)
{
	YSRESULT res=YSOK;
	YsFileIO::File fp(fName,"r");
	if(nullptr!=fp)
	{
		YsString str;
		while(nullptr!=str.Fgets(fp))
		{
			RunCommand(str);
		}

		auto &wav=GetCurrentWav();
		auto &wavRaw=wav.GetWave();
		auto lastModifiedChannel=wavRaw.GetLastModifiedChannel();
		SetCurrentChannel(lastModifiedChannel);
	}
	else
	{
		auto msgDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiMessageBoxDialog>();
		msgDialog->Make(L"Error!",L"Failed to Open File.",L"OK.",nullptr,nullptr);
		AttachModalDialog(msgDialog);
		res=YSERR;
	}

	// if no error
	if(YSOK==res)
	{
		if(YSTRUE==updateRecent)
		{
			AddRecentlyUsedFile(fName);
		}
	}
}

long long int FsGuiMainCanvas::GetTemporaryPhaseOffset(void) const
{
	return temporaryPhaseOffset;
}

void FsGuiMainCanvas::SetTemporaryPhaseOffset(long long int ofst)
{
	temporaryPhaseOffset=ofst;
}

void FsGuiMainCanvas::RunCommand(const YsString &cmd)
{
	auto &waveEdit=GetCurrentWav();

	waveEdit.RunCommand(cmd);
	SetNeedRedraw(YSTRUE);
}

void FsGuiMainCanvas::OnInterval(void)
{
	FsGuiCanvas::Interval();

	{
		int key;
		while(FSKEY_NULL!=(key=FsInkey()))
		{
			this->KeyIn(key,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
		}
	}

	{
		int charCode;
		while(0!=(charCode=FsInkeyChar()))
		{
			this->CharIn(charCode);
		}
	}

	{
		int lb,mb,rb,mx,my;
		while(FSMOUSEEVENT_NONE!=FsGetMouseEvent(lb,mb,rb,mx,my))
		{
			if(YSOK!=this->SetMouseState((YSBOOL)lb,(YSBOOL)mb,(YSBOOL)rb,mx,my))
			{
			}
		}
	}

	{
		auto nTouch=FsGetNumCurrentTouch();
		auto touch=FsGetCurrentTouch();
		if(YSOK!=this->SetTouchState(nTouch,touch))
		{
		}
	}

	int winWid,winHei;
	FsGetWindowSize(winWid,winHei);
	this->SetWindowSize(winWid,winHei,/*autoArrangeDialog=*/YSTRUE);

	if(0!=FsCheckWindowExposure())
	{
		SetNeedRedraw(YSTRUE);
	}
}

/* virtual */ YSRESULT FsGuiMainCanvas::OnLButtonDown(YSBOOL lb,YSBOOL mb,YSBOOL rb,int mx,int my)
{
	auto &wavEdit=GetCurrentWav();
	auto selection=wavEdit.GetSelection();
	auto viewport=wavEdit.GetViewport();

	auto waveRect=GetWaveDrawingRect();
	if(YSTRUE==waveRect.IsInside(YsVec2i(mx,my)))
	{
		long long dx=mx-waveRect.Min().x();
		long long wid=waveRect.GetWidth();
		selection.minmax[0]=viewport.zero+dx*viewport.wid/wid;
		wavEdit.SetSelection(selection);
		printf("Begin:%lld  End:%lld  Length:%lld\n",selection.minmax[0],selection.minmax[1],selection.GetLength());
		SetNeedRedraw(YSTRUE);
		return YSOK;
	}
	return YSERR;
}
/* virtual */ YSRESULT FsGuiMainCanvas::OnRButtonDown(YSBOOL lb,YSBOOL mb,YSBOOL rb,int mx,int my)
{
	auto &wavEdit=GetCurrentWav();
	auto selection=wavEdit.GetSelection();
	auto viewport=wavEdit.GetViewport();

	auto waveRect=GetWaveDrawingRect();
	if(YSTRUE==waveRect.IsInside(YsVec2i(mx,my)))
	{
		long long dx=mx-waveRect.Min().x();
		long long wid=waveRect.GetWidth();
		selection.minmax[1]=viewport.zero+dx*viewport.wid/wid;
		wavEdit.SetSelection(selection);
		printf("Begin:%lld  End:%lld  Length:%lld\n",selection.minmax[0],selection.minmax[1],selection.GetLength());
		SetNeedRedraw(YSTRUE);
		return YSOK;
	}
	return YSERR;
}



////////////////////////////////////////////////////////////

void FsGuiMainCanvas::File_Exit(FsGuiPopUpMenuItem *)
{
	auto msgDlg=FsGuiDialog::CreateSelfDestructiveDialog <FsGuiMessageBoxDialog>();
	msgDlg->Make(L"Confirm Exit?",L"Confirm Exit?",L"Yes",L"No");
	msgDlg->BindCloseModalCallBack(&THISCLASS::File_Exit_ConfirmExitCallBack,this);
	AttachModalDialog(msgDlg);
}

void FsGuiMainCanvas::File_Exit_ConfirmExitCallBack(FsGuiDialog *,int returnValue)
{
	if(YSOK==(YSRESULT)returnValue)
	{
		File_Exit_ReallyExit();
	}
}

void FsGuiMainCanvas::File_Exit_ReallyExit(void)
{
	this->appMustTerminate=YSTRUE;
}

////////////////////////////////////////////////////////////

