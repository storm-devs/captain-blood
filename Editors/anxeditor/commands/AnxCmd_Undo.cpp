//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_Undo
//============================================================================================


#include "AnxCmd_Undo.h"

//============================================================================================

AnxCmd_Undo::AnxCmd_Undo(AnxOptions & options) : Command(options)
{

}

AnxCmd_Undo::~AnxCmd_Undo()
{

}


//============================================================================================

bool AnxCmd_Undo::IsEnable()
{
	if(!opt.project) return false;
	if(opt.project->cmdPointer >= 0) return true;
	return false;
}

void AnxCmd_Undo::Do()
{
	Assert(opt.project);
	Assert(opt.project->cmdPointer >= 0);
	opt.project->commands[opt.project->cmdPointer--]->UnDo();
}

Command * AnxCmd_Undo::CreateThisObject(AnxOptions & opt)
{
	return NEW AnxCmd_Undo(opt);
}


