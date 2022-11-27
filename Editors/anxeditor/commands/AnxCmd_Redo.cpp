//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_Redo
//============================================================================================


#include "AnxCmd_Redo.h"

//============================================================================================

AnxCmd_Redo::AnxCmd_Redo(AnxOptions & options) : Command(options)
{

}

AnxCmd_Redo::~AnxCmd_Redo()
{

}


//============================================================================================


bool AnxCmd_Redo::IsEnable()
{
	if(!opt.project) return false;
	if(opt.project->cmdPointer + 1 < opt.project->commands) return true;
	return false;
}

void AnxCmd_Redo::Do()
{
	Assert(opt.project);
	Assert(opt.project->cmdPointer + 1 < opt.project->commands);
	opt.project->commands[++opt.project->cmdPointer]->ReDo();
}

Command * AnxCmd_Redo::CreateThisObject(AnxOptions & opt)
{
	return NEW AnxCmd_Redo(opt);
}



