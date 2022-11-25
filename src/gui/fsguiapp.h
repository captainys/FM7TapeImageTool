/* ////////////////////////////////////////////////////////////

File Name: fsguiapp.h
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

#ifndef FSGUI_APP_IS_INCLUDED
#define FSGUI_APP_IS_INCLUDED
/* { */

#include <fsgui.h>
#include "yswaveedit.h"

class YsHasWaveSound
{
private:
	YsWaveEdit wav;
	int channel;
public:
	YsHasWaveSound();
	const YsWaveEdit &GetCurrentWav(void) const;
	YsWaveEdit &GetCurrentWav(void);
	int GetCurrentChannel(void) const;
	void SetCurrentChannel(int channel);
};

class FsGuiMainCanvas : public FsGuiCanvas, public YsHasWaveSound
{
private:
	class MarkSilentRegionDialog;
	class LowPeakDialog;
	class FM7Dialog;
	class PhaseShiftDialog;
	class JumpToDialog;

	int temporaryPhaseOffset;
public:
	/*! Main menu.  MainMenu is created in MakeMainMenu function, which is called 
	    from the constructor.
	*/
	FsGuiPopUpMenu *mainMenu;

	/*! Low-level interface, FsLazyWindow framework, checks for this value to see
	    if the application run-loop should be terminated.
	*/
	YSBOOL appMustTerminate;

private:
	/*! For convenience, you can use THISCLASS instead of FsGuiMainCanvas 
	    in the member functions.
	*/
	typedef FsGuiMainCanvas THISCLASS;

public:
	// [Core data structure]


	// [GUI support]
	FsGuiRecentFiles recent;
	YsWString lastUsedFileName;
	FsGuiPopUpMenu *fileRecent;
	FsGuiPopUpMenuItem *drawAllChannel;


	// [Modeless dialogs]
	//   (1) Add a pointer in the following chunk.
	//   (2) Add an initialization in the constructor of the aplication.
	//   (3) Add deletion in the destructor of the application.
	//   (4) Add RemoveDialog in Edit_ClearUIIMode


	// [Modal dialogs]


	/*! Constructor is called after OpenGL context is created.
	    It is safe to make OpenGL function calls inside.
	*/
	FsGuiMainCanvas();

	/*! */
	~FsGuiMainCanvas();

	/*! This function is called from the low-level interface to get an
	    application pointer.
	*/
	static FsGuiMainCanvas *GetMainCanvas();

	/*! This funciion is called from the low-level interface for
	    deleting the application.
	*/
	static void DeleteMainCanvas(void);

	/*! Customize this function for adding menus.
	*/
	void MakeMainMenu(void);
	void DeleteMainMenu(void);

	/*! Save data.
	*/
	void Save(YsWString fName,YSBOOL updateRecent);
	/*! Load data.
	*/
	void Open(YsWString fName,YSBOOL updateRecent);

	void SaveCommandLog(YsWString fName,YSBOOL updateRecent);
	void RecallCommandLog(YsWString fName,YSBOOL updateRecent);

	void RunCommand(const YsString &cmd);

public:
	/*! In this function, shared GLSL renderers are created,
	    View-target is set to (-5,-5,-5) to (5,5,5),
	    and view distance is set to 7.5 by default.
	*/
	void Initialize(int argc,char *argv[]);

	/*! This function is called regularly from the low-level interface.
	*/
	void OnInterval(void);


	virtual YSRESULT OnLButtonDown(YSBOOL lb,YSBOOL mb,YSBOOL rb,int mx,int my);
	virtual YSRESULT OnRButtonDown(YSBOOL lb,YSBOOL mb,YSBOOL rb,int mx,int my);

	/*! Phase offset for drawing the current channel.
	*/
	long long int GetTemporaryPhaseOffset(void) const;

	/*! Set phase offset for drawing the current channel.
	*/
	void SetTemporaryPhaseOffset(long long int);

	/*! This function is called from the low-level interface when the
	    window needs to be re-drawn.
	*/
	void Draw(void);
	void DrawHighlightSelection(void) const;
	void DrawWave(void) const;
	void DrawWaveChannel(int channel,YsRect2i waveRect,long long int offset,YsColor col) const;
	YsRect2i GetWaveDrawingRect(void) const;

private:
	// [Menu call-backs]
	/*! Sample call-back functions.
	*/
	void File_Open(FsGuiPopUpMenuItem *);
	void File_Open_FileSelected(FsGuiDialog *dlg,int returnCode);

	void File_Append(FsGuiPopUpMenuItem *);
	void File_Append_FileSelected(FsGuiDialog *dlg,int returnCode);

	void File_Save(FsGuiPopUpMenuItem *);

	void File_SaveAs(FsGuiPopUpMenuItem *);
	void File_SaveAs_FileSelected(FsGuiDialog *dlg,int returnCode);
	void File_SaveAs_OverwriteConfirmed(FsGuiDialog *dlg,int returnCode);

	void File_SaveCommandLog(FsGuiPopUpMenuItem *);
	void File_SaveCommandLog_FileSelected(FsGuiDialog *dlg,int returnCode);
	void File_SaveCommandLog_OverwriteConfirmed(FsGuiDialog *dlg,int returnCode);

	void File_RecallCommandLog(FsGuiPopUpMenuItem *);
	void File_RecallCommandLog_FileSelected(FsGuiDialog *dlg,int returnCode);

	void File_Exit(FsGuiPopUpMenuItem *);
	void File_Exit_ConfirmExitCallBack(FsGuiDialog *,int);
	void File_Exit_ReallyExit(void);

	void File_Recent(FsGuiPopUpMenuItem *);



	void Edit_MakeSilence(FsGuiPopUpMenuItem *);
	void Edit_ReallySilenceSilentSegment(FsGuiPopUpMenuItem *);
	void Edit_DeleteChannel(FsGuiPopUpMenuItem *);

	void Edit_MergeToOneChannel(FsGuiPopUpMenuItem *);

	void Edit_MakeSineWaveHighFirst(FsGuiPopUpMenuItem *);
	void Edit_MakeSineWaveLowFirst(FsGuiPopUpMenuItem *);

	void Edit_ShiftPhase(FsGuiPopUpMenuItem *);

	void Edit_Trim(FsGuiPopUpMenuItem *);



	void Select_Wave(FsGuiPopUpMenuItem *);
	void Select_NextWave(FsGuiPopUpMenuItem *);
	void Select_PrevWave(FsGuiPopUpMenuItem *);



	void View_Zoom(FsGuiPopUpMenuItem *);
	void View_Mooz(FsGuiPopUpMenuItem *);
	void View_MoveLeft(FsGuiPopUpMenuItem *);
	void View_MoveRight(FsGuiPopUpMenuItem *);

	void View_JumpTo(FsGuiPopUpMenuItem *);
	void View_JumpToSelBegin(FsGuiPopUpMenuItem *);
	void View_JumpToSelEnd(FsGuiPopUpMenuItem *);

	void View_Channel0(FsGuiPopUpMenuItem *);
	void View_Channel1(FsGuiPopUpMenuItem *);
	void View_DrawNonCurrent(FsGuiPopUpMenuItem *);


	void Analyze_DetectPeak(FsGuiPopUpMenuItem *);
	void Analyze_FilterShortAndLowPeak(FsGuiPopUpMenuItem *);
public:
	void Analyze_FilterShortAndLowPeak_DoFilter(unsigned int lowThr,unsigned int shortThr);
private:
	void Analyze_InspectPeak(FsGuiPopUpMenuItem *);
	void Analyze_MarkSilentRegion(FsGuiPopUpMenuItem *);
	void Analyze_MarkLongWaveAsSilentFollowedByShortWave(FsGuiPopUpMenuItem *);
	void Analyze_Unsilence(FsGuiPopUpMenuItem *);
	void Analyze_CalculateEnvelope(FsGuiPopUpMenuItem *);
	void Analyze_FM7(FsGuiPopUpMenuItem *);



	void Filter_Median(FsGuiPopUpMenuItem *);
	void Filter_ExpandEnvelope(FsGuiPopUpMenuItem *);



	/*! Customize this function.
	    This function must return a file name where recent file list is saved.
	*/
	YsWString GetRecentFileListFileName(void) const;

	void RefreshRecentlyUsedFileList(void);

	void AddRecentlyUsedFile(YsWString fName);
};

/* } */
#endif
