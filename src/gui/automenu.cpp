#include <ysport.h>
#include <t77.h>
#include <fsguifiledialog.h>

#include "yswave_fm7util.h"

#include "fsguiapp.h"





class AutoFilterDialog : public FsGuiDialog
{
public:
	FsGuiButton *okBtn,*cancelBtn;

	// For marking silent segments
	FsGuiTextBox *levelThrTxt,*durationThrTxt;

	// For filtering peaks
	FsGuiTextBox *lowPeakThrTxt,*shortPeakThrTxt;

	FsGuiMainCanvas *owner;

	void Make(FsGuiMainCanvas *canvas);
	void OnButtonClick(FsGuiButton *btn);
};

void AutoFilterDialog::Make(FsGuiMainCanvas *owner)
{
	this->owner=owner;

	FsGuiDialog::Initialize();
	okBtn=AddTextButton(MkId("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,L"OK",YSTRUE);
	cancelBtn=AddTextButton(MkId("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,L"Cancel",YSFALSE);

	AddStaticText(0,FSKEY_NULL,"Mark Silent Segments",YSTRUE);
	levelThrTxt=AddTextBox(MkId("levelThr"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Level Threshold",20,YSTRUE);
	durationThrTxt=AddTextBox(MkId("durationThr"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Duration Threshold",20,YSTRUE);

	levelThrTxt->SetInteger(1638); // 5% of spectrum
	durationThrTxt->SetRealNumber(0.1833,1);   // 600bps, 1 byte=11-pulse long -> 0.1833 sec


	AddStaticText(0,FSKEY_NULL,"Filter Peaks",YSTRUE);
	lowPeakThrTxt=AddTextBox(MkId("lowPeakThr"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Low Peak Threshold",20,YSTRUE);
	shortPeakThrTxt=AddTextBox(MkId("shortPeakThr"),FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Short Peak Threshold",20,YSTRUE);

	lowPeakThrTxt->SetInteger(1638); // 5%
	shortPeakThrTxt->SetInteger(5);


	SetArrangeType(FSDIALOG_ARRANGE_TOP_LEFT);
	Fit();
}

void AutoFilterDialog::OnButtonClick(FsGuiButton *btn)
{
	if(okBtn==btn)
	{
		owner->DoAutomaticFiltering(levelThrTxt->GetInteger(),durationThrTxt->GetRealNumber(),lowPeakThrTxt->GetInteger(),shortPeakThrTxt->GetInteger());
	}
	CloseModalDialog(0);
}



void FsGuiMainCanvas::AutomaticFiltering(FsGuiPopUpMenuItem *)
{
	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<AutoFilterDialog>();
	dlg->Make(this);
	AttachModalDialog(dlg);
}
void FsGuiMainCanvas::DoAutomaticFiltering(int silenceLevelThr,double silenceDurationThr,int lowPeakThr,int shortPeakThr)
{
	YsString str;

	str.Printf("ANALYZE MARK_SILENT %d %d %lf",
		GetCurrentChannel(),
		silenceLevelThr,
		silenceDurationThr);
	RunCommand(str);


	str.Printf("EDIT REALLY_SILENCE_SILENT_REGIONS %d",GetCurrentChannel());
	RunCommand(str);


	str.Printf("FILTER MEDIAN %d",GetCurrentChannel());
	RunCommand(str);


	str.Printf("ANALYZE DETECT_PEAK %d",GetCurrentChannel());
	RunCommand(str);


	str.Printf("FILTER LOW_AND_SHORT_PEAK %d %d %d",GetCurrentChannel(),lowPeakThr,shortPeakThr);
	RunCommand(str);


	str.Printf("ANALYZE CALCULATE_ENVELOPE %d",GetCurrentChannel());
	RunCommand(str);


	str.Printf("FILTER EXPAND_ENVELOPE %d",GetCurrentChannel());
	RunCommand(str);


	SetNeedRedraw(YSTRUE);
}
