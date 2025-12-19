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


class FsGuiMainCanvas::ForceWaveLengthDialog : public FsGuiDialog
{
public:
	FsGuiButton *okBtn,*cancelBtn,*withinSelectedRegionBtn;
	FsGuiButton *dontCareBtn,*lowFirstBtn,*highFirstBtn;
	FsGuiTextBox *srcMinLenTxt,*srcMaxLenTxt,*newLenTxt;
	void Make(void);
	void OnButtonClick(FsGuiButton *btn) override;
};

void FsGuiMainCanvas::ForceWaveLengthDialog::Make(void)
{
	okBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"OK",YSTRUE);
	cancelBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"Cancel",YSTRUE);
	withinSelectedRegionBtn=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,"Within Selected Region",YSTRUE);

	dontCareBtn=AddTextButton(0,FSKEY_NULL,FSGUI_RADIOBUTTON,"Don't Care",YSTRUE);
	lowFirstBtn=AddTextButton(0,FSKEY_NULL,FSGUI_RADIOBUTTON,"Low First",YSFALSE);
	highFirstBtn=AddTextButton(0,FSKEY_NULL,FSGUI_RADIOBUTTON,"High First",YSFALSE);

	srcMinLenTxt=AddTextBox(0,FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Source Wave Len Min",10,YSTRUE);
	srcMaxLenTxt=AddTextBox(0,FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"Source Wave Len Max",10,YSTRUE);
	newLenTxt=AddTextBox(0,FSKEY_NULL,FsGuiTextBox::HORIZONTAL,L"New Wave Len",10,YSTRUE);

	FsGuiButton *radioBtnGrp[3]=
	{
		dontCareBtn,
		lowFirstBtn,
		highFirstBtn
	};
	SetRadioButtonGroup(3,radioBtnGrp);
	lowFirstBtn->SetCheck(YSTRUE);

	SetArrangeType(FSDIALOG_ARRANGE_TOP_LEFT);
	Fit();
}

void FsGuiMainCanvas::ForceWaveLengthDialog::OnButtonClick(FsGuiButton *btn)
{
	if(okBtn==btn)
	{
		CloseModalDialog(1);
	}
	else if(cancelBtn==btn)
	{
		CloseModalDialog(0);
	}
}

void FsGuiMainCanvas::Filter_ForceWaveLength(FsGuiPopUpMenuItem *)
{
	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<ForceWaveLengthDialog>();
	dlg->Make();
	AttachModalDialog(dlg);
	ArrangeDialog();

	dlg->CallOnCloseModal([=](int returnCode)
		{
			if(0!=returnCode)
			{
				printf("Do it!\n");

				int sourceWaveType=0;
				if(YSTRUE==dlg->lowFirstBtn->GetCheck())
				{
					sourceWaveType=1;
				}
				else if(YSTRUE==dlg->highFirstBtn->GetCheck())
				{
					sourceWaveType=2;
				}

				auto &wav=GetCurrentWav();
				auto &wavRaw=wav.GetWave();
				auto channel=GetCurrentChannel();

				size_t i0=0,i1=wavRaw.GetNumSamplePerChannel();
				if(YSTRUE==dlg->withinSelectedRegionBtn->GetCheck())
				{
					auto sel=wav.GetSelection();
					i0=sel.minmax[0];
					i1=sel.minmax[1];
				}

				auto srcLenMin=dlg->srcMinLenTxt->GetInteger();
				auto srcLenMax=dlg->srcMaxLenTxt->GetInteger();
				auto newLen=dlg->newLenTxt->GetInteger();

				YsString cmd;
				cmd.Printf("EDIT FORCELENGTH %d %d %lld %lld %d %d %d",
				   channel,
				   sourceWaveType,
				   i0,
				   i1,
				   srcLenMin,
				   srcLenMax,
				   newLen);

				printf("%s\n",cmd.c_str());
				RunCommand(cmd);
				SetNeedRedraw(YSTRUE);
			}
		}
	);
}
