//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// Command
//============================================================================================


#include "Command.h"

//============================================================================================

Command::Command(AnxOptions & options) : opt(options)
{
}

Command::~Command()
{

}

//============================================================================================

void Command::Process()
{
	if(!opt.project) return;
	if(!IsEnable()) return;
	Command * cmd = CreateThisObject(opt);
	if(cmd) opt.project->Execute(cmd);
}

bool Command::IsEnable()
{
	return false;
}

bool Command::CanUndo()
{
	return false;
}

void Command::Do()
{

}

void Command::UnDo()
{
}

void Command::ReDo()
{

}

Command * Command::CreateThisObject(AnxOptions & opt)
{
	Assert(false);
	return null;
}
