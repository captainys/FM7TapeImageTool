#include <ysport.h>

#include "fsguiapp.h"



void FsGuiMainCanvas::Edit_MakeSilence(FsGuiPopUpMenuItem *)
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
	str.Printf("EDIT SILENCE %d %lld %lld",GetCurrentChannel(),selBegin,selEnd);
	waveEdit.RunCommand(str);
	SetNeedRedraw(YSTRUE);
}
void FsGuiMainCanvas::Edit_ReallySilenceSilentSegment(FsGuiPopUpMenuItem *)
{
	auto &waveEdit=GetCurrentWav();

	YsString str;
	str.Printf("EDIT REALLY_SILENCE_SILENT_REGIONS %d",GetCurrentChannel());
	waveEdit.RunCommand(str);
	SetNeedRedraw(YSTRUE);
}
void FsGuiMainCanvas::Edit_DeleteChannel(FsGuiPopUpMenuItem *)
{
	auto &waveEdit=GetCurrentWav();
	YsString str;
	str.Printf("EDIT DELETE_CHANNEL %d",GetCurrentChannel());
	waveEdit.RunCommand(str);
	SetCurrentChannel(0);
	SetNeedRedraw(YSTRUE);
}

void FsGuiMainCanvas::Edit_MergeToOneChannel(FsGuiPopUpMenuItem *)
{
	auto &waveEdit=GetCurrentWav();
	waveEdit.RunCommand("EDIT MERGE_CHANNEL");
	SetCurrentChannel(0);
	SetNeedRedraw(YSTRUE);
}

void FsGuiMainCanvas::Edit_MakeSineWaveHighFirst(FsGuiPopUpMenuItem *)
{
	auto &waveEdit=GetCurrentWav();

	auto sel=waveEdit.GetSelection();
	auto selBegin=sel.minmax[0];
	auto selEnd=sel.minmax[1];

	if(selBegin>selEnd)
	{
		std::swap(selBegin,selEnd);
	}
	if(selBegin<selEnd)
	{
		YsString str;
		str.Printf("EDIT MAKE_SINEWAVE %d 1 %lld %lld 30000",GetCurrentChannel(),selBegin,selEnd);
		waveEdit.RunCommand(str);
		SetNeedRedraw(YSTRUE);
	}
}
void FsGuiMainCanvas::Edit_MakeSineWaveLowFirst(FsGuiPopUpMenuItem *)
{
	auto &waveEdit=GetCurrentWav();

	auto sel=waveEdit.GetSelection();
	auto selBegin=sel.minmax[0];
	auto selEnd=sel.minmax[1];

	if(selBegin>selEnd)
	{
		std::swap(selBegin,selEnd);
	}
	if(selBegin<selEnd)
	{
		YsString str;
		str.Printf("EDIT MAKE_SINEWAVE %d 0 %lld %lld 30000",GetCurrentChannel(),selBegin,selEnd);
		waveEdit.RunCommand(str);
		SetNeedRedraw(YSTRUE);
	}
}

class FsGuiMainCanvas::PhaseShiftDialog : public FsGuiDialog
{
public:
	FsGuiMainCanvas *owner;

	FsGuiTextBox *offsetTxt;
	FsGuiButton *plusBtn,*minusBtn,*applyBtn,*cancelBtn;

	void Make(FsGuiMainCanvas *owner);
	void OnButtonClick(FsGuiButton *btn);
	void OnTextBoxChange(FsGuiTextBox *txt);
};

void FsGuiMainCanvas::PhaseShiftDialog::Make(FsGuiMainCanvas *owner)
{
	this->owner=owner;

	applyBtn=AddTextButton(MkId("ok"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Apply",YSFALSE);
	cancelBtn=AddTextButton(MkId("cancel"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Cancel",YSFALSE);

	offsetTxt=AddTextBox(MkId("offset"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Offset",7,YSFALSE);
	offsetTxt->SetInteger(0);

	plusBtn=AddTextButton(MkId("plus"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"+",YSFALSE);
	minusBtn=AddTextButton(MkId("minus"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"-",YSFALSE);

	SetArrangeType(FSDIALOG_ARRANGE_TOP_LEFT);
	Fit();
}

void FsGuiMainCanvas::PhaseShiftDialog::OnButtonClick(FsGuiButton *btn)
{
	if(applyBtn==btn)
	{
		long long int offset=offsetTxt->GetInteger();
		YsString str;
		str.Printf("EDIT SHIFT_PHASE %d %lld",owner->GetCurrentChannel(),offset);
		owner->GetCurrentWav().RunCommand(str);

		owner->SetTemporaryPhaseOffset(0);
		owner->SetNeedRedraw(YSTRUE);
		owner->RemoveDialog(this);
	}
	if(cancelBtn==btn)
	{
		owner->SetTemporaryPhaseOffset(0);
		owner->SetNeedRedraw(YSTRUE);
		owner->RemoveDialog(this);
	}
	if(plusBtn==btn)
	{
		auto ofst=offsetTxt->GetInteger()+1;
		offsetTxt->SetInteger(ofst);
		owner->SetTemporaryPhaseOffset(ofst);
		owner->SetNeedRedraw(YSTRUE);
	}
	if(minusBtn==btn)
	{
		auto ofst=offsetTxt->GetInteger()-1;
		offsetTxt->SetInteger(ofst);
		owner->SetTemporaryPhaseOffset(ofst);
		owner->SetNeedRedraw(YSTRUE);
	}
}

void FsGuiMainCanvas::PhaseShiftDialog::OnTextBoxChange(FsGuiTextBox *txt)
{
	if(txt==offsetTxt)
	{
		owner->SetTemporaryPhaseOffset(txt->GetInteger());
		owner->SetNeedRedraw(YSTRUE);
	}
}

void FsGuiMainCanvas::Edit_ShiftPhase(FsGuiPopUpMenuItem *)
{
	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<PhaseShiftDialog>();
	dlg->Make(this);
	AddDialog(dlg);
	ArrangeDialog();
	SetNeedRedraw(YSTRUE);
}

void FsGuiMainCanvas::Edit_Trim(FsGuiPopUpMenuItem *)
{
	auto &waveEdit=GetCurrentWav();

	auto sel=waveEdit.GetSelection();
	auto selBegin=sel.minmax[0];
	auto selEnd=sel.minmax[1];

	if(selBegin<selEnd)
	{
		YsString str;
		str.Printf("EDIT TRIM %lld %lld",selBegin,selEnd);
		waveEdit.RunCommand(str);
		SetNeedRedraw(YSTRUE);
	}
	
}
