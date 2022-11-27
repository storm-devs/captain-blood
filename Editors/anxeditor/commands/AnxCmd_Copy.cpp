//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_Copy
//============================================================================================


#include "AnxCmd_Copy.h"

//============================================================================================

AnxCmd_Copy::AnxCmd_Copy(AnxOptions & options) : Command(options)
{

}

AnxCmd_Copy::~AnxCmd_Copy()
{

}


//============================================================================================

bool AnxCmd_Copy::IsEnable()
{
	if(!opt.project) return false;
	if(opt.project->graph.IsHaveSelNodes()) return true;
	return false;
}

void AnxCmd_Copy::Do()
{
	Assert(opt.project);
	Assert(opt.project->graph.current);
	opt.project->graphCopyBuffer.Reset();
	opt.project->graph.Export(opt.project->graphCopyBuffer);
	opt.project->graphCopyBuffer.EndOfWrite();
}

Command * AnxCmd_Copy::CreateThisObject(AnxOptions & opt)
{
	return NEW AnxCmd_Copy(opt);
}


