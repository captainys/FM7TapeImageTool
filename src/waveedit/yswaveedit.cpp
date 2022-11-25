#include "yswaveedit.h"



const std::vector <YsString> &YsWaveEdit::GetCommandLog(void) const
{
	return cmdLog;
}

YSRESULT YsWaveEdit::LoadWav(YsWString fName)
{
	CleanUp();
	return YsWaveKernel::LoadWav(fName);
}

YSRESULT YsWaveEdit::RunCommand(YsString cmd)
{
	auto argv=cmd.Argv();
	if(0==argv.size() || '#'==argv[0][0])
	{
		return YSOK;
	}

	cmdLog.push_back(cmd);

	if(0==argv[0].STRCMP("ANALYZE"))
	{
		return RunCommand_Analyze(cmd,argv);
	}
	else if(0==argv[0].STRCMP("EDIT"))
	{
		return RunCommand_Edit(cmd,argv);
	}
	else if(0==argv[0].STRCMP("FILTER"))
	{
		return RunCommand_Filter(cmd,argv);
	}
	else if(0==argv[0].STRCMP("FM7"))
	{
		return RunCommand_FM7(cmd,argv);
	}

	Error(cmd,"Unknown command.");
	return YSERR;
}

void YsWaveEdit::Error(const YsString &fullCmd,YsString msg)
{
	fprintf(stderr,"Command Error\n");
	fprintf(stderr,"Command: %s\n",fullCmd.c_str());
	fprintf(stderr,"Reason: %s\n",msg.c_str());
	errString=msg;
}

