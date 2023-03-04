#include <ysport.h>

#include "fsguiapp.h"
#include "fsguifiledialog.h"



void FsGuiMainCanvas::View_Zoom(FsGuiPopUpMenuItem *)
{
	int wid,hei;
	FsGetWindowSize(wid,hei);

	auto &wavEdit=GetCurrentWav();
	auto viewport=wavEdit.GetViewport();

	auto vpCenter=viewport.zero+viewport.wid/2;

	viewport.wid/=2;
	if(viewport.wid<wid/8)
	{
		viewport.wid=wid/8;
	}

	viewport.zero=vpCenter-viewport.wid/2;
	printf("%lld %lld\n",viewport.zero,viewport.wid);

	wavEdit.SetViewport(viewport);

	SetNeedRedraw(YSTRUE);
}
void FsGuiMainCanvas::View_Mooz(FsGuiPopUpMenuItem *)
{
	int wid,hei;
	FsGetWindowSize(wid,hei);

	auto &wavEdit=GetCurrentWav();
	auto &wavRaw=wavEdit.GetWave();
	auto viewport=wavEdit.GetViewport();

	auto vpCenter=viewport.zero+viewport.wid/2;

	viewport.wid*=2;
	if(wavRaw.GetNumSamplePerChannel()<viewport.wid)
	{
		viewport.wid=wavRaw.GetNumSamplePerChannel();
	}

	viewport.zero=vpCenter-viewport.wid/2;
	printf("%lld %lld\n",viewport.zero,viewport.wid);

	wavEdit.SetViewport(viewport);

	SetNeedRedraw(YSTRUE);
}
void FsGuiMainCanvas::View_MoveLeft(FsGuiPopUpMenuItem *)
{
	int wid,hei;
	FsGetWindowSize(wid,hei);

	auto &wavEdit=GetCurrentWav();
	auto viewport=wavEdit.GetViewport();
	viewport.zero-=viewport.wid/10;

	printf("%lld %lld\n",viewport.zero,viewport.wid);

	wavEdit.SetViewport(viewport);

	SetNeedRedraw(YSTRUE);
}
void FsGuiMainCanvas::View_MoveRight(FsGuiPopUpMenuItem *)
{
	int wid,hei;
	FsGetWindowSize(wid,hei);

	auto &wavEdit=GetCurrentWav();
	auto viewport=wavEdit.GetViewport();
	viewport.zero+=viewport.wid/10;

	printf("%lld %lld\n",viewport.zero,viewport.wid);

	wavEdit.SetViewport(viewport);

	SetNeedRedraw(YSTRUE);
}

void FsGuiMainCanvas::View_Channel0(FsGuiPopUpMenuItem *)
{
	SetCurrentChannel(0);
	SetNeedRedraw(YSTRUE);
}

void FsGuiMainCanvas::View_Channel1(FsGuiPopUpMenuItem *)
{
	auto &wavEdit=GetCurrentWav();
	auto &wavRaw=wavEdit.GetWave();
	if(2<=wavRaw.GetNumChannel())
	{
		SetCurrentChannel(1);
		SetNeedRedraw(YSTRUE);
	}
}

void FsGuiMainCanvas::View_DrawNonCurrent(FsGuiPopUpMenuItem *itm)
{
	auto check=itm->GetCheck();
	YsFlip(check);
	itm->SetCheck(check);
	SetNeedRedraw(YSTRUE);
}

class FsGuiMainCanvas::JumpToDialog : public FsGuiDialog
{
private:
	FsGuiMainCanvas *owner;

	FsGuiButton *jumpBtn,*closeBtn;

public:
	FsGuiTextBox *locationTxt;
	void Make(FsGuiMainCanvas *owner);
	void OnButtonClick(FsGuiButton *btn);
};

void FsGuiMainCanvas::JumpToDialog::Make(FsGuiMainCanvas *owner)
{
	this->owner=owner;

	jumpBtn=AddTextButton(MkId("jump"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Jump",YSFALSE);
	closeBtn=AddTextButton(MkId("close"),FSKEY_NULL,FSGUI_PUSHBUTTON,L"Close",YSFALSE);

	locationTxt=AddTextBox(MkId("location"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Location",10,YSFALSE);
	SetArrangeType(FSDIALOG_ARRANGE_TOP_LEFT);
	Fit();
}

void FsGuiMainCanvas::JumpToDialog::OnButtonClick(FsGuiButton *btn)
{
	if(jumpBtn==btn)
	{
		auto &wavEdit=owner->GetCurrentWav();
		auto viewport=wavEdit.GetViewport();
		auto ptr=locationTxt->GetInteger();
		viewport.zero=ptr-viewport.wid/2;
		wavEdit.SetViewport(viewport);
		owner->SetNeedRedraw(YSTRUE);
	}
	else if(closeBtn==btn)
	{
		owner->RemoveDialog(this);
	}
}

void FsGuiMainCanvas::View_JumpTo(FsGuiPopUpMenuItem *)
{
	auto dlg=FindTypedModelessDialog<JumpToDialog>();
	if(nullptr==dlg)
	{
		dlg=FsGuiDialog::CreateSelfDestructiveDialog<JumpToDialog>();
		dlg->Make(this);
		AddDialog(dlg);
		ArrangeDialog();
	}

	auto &wavEdit=GetCurrentWav();
	auto viewport=wavEdit.GetViewport();
	auto currentPtr=(long long int)(viewport.zero+viewport.wid/2);
	dlg->locationTxt->SetInteger(currentPtr);
	SetNeedRedraw(YSTRUE);
}

void FsGuiMainCanvas::View_JumpToSelBegin(FsGuiPopUpMenuItem *)
{
	auto &wavEdit=GetCurrentWav();
	auto viewport=wavEdit.GetViewport();
	auto sel=wavEdit.GetSelection();

	viewport.zero=sel.Min()-viewport.wid/2;
	wavEdit.SetViewport(viewport);
	SetNeedRedraw(YSTRUE);
}
void FsGuiMainCanvas::View_JumpToSelEnd(FsGuiPopUpMenuItem *)
{
	auto &wavEdit=GetCurrentWav();
	auto viewport=wavEdit.GetViewport();
	auto sel=wavEdit.GetSelection();

	viewport.zero=sel.Max()-viewport.wid/2;
	wavEdit.SetViewport(viewport);
	SetNeedRedraw(YSTRUE);
}
