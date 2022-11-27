//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================
//
//===========================================================================================================================
// AnxCmd_OpenNodesList
//============================================================================================


#include "AnxCmd_OpenNodesList.h"

//============================================================================================

AnxCmd_OpenNodesList::AnxCmd_OpenNodesList(AnxOptions & options, GUIControl * listener, CONTROL_EVENT event) : Command(options)
{
	handler.SetHandler(listener, event);
}

AnxCmd_OpenNodesList::AnxCmd_OpenNodesList(AnxOptions & options, const GUIEventHandler & handler) : Command(options)
{
	this->handler = handler;
}

AnxCmd_OpenNodesList::~AnxCmd_OpenNodesList()
{

}


//============================================================================================


bool AnxCmd_OpenNodesList::IsEnable()
{
	if(!opt.project) return false;
	if(!opt.project->graph.root) return false;
	return (opt.project->graph.root->child > 0);
}

void AnxCmd_OpenNodesList::Do()
{
	handler.Execute(null);
}

Command * AnxCmd_OpenNodesList::CreateThisObject(AnxOptions & opt)
{	
	return NEW AnxCmd_OpenNodesList(opt, handler);
}

