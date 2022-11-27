//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_ExecuteEvent
//============================================================================================


#include "AnxCmd_ExecuteEvent.h"

//============================================================================================

AnxCmd_ExecuteEvent::AnxCmd_ExecuteEvent(AnxOptions & options, GUIControl * listener, CONTROL_EVENT event) : Command(options)
{
	handler.SetHandler(listener, event);
}

AnxCmd_ExecuteEvent::AnxCmd_ExecuteEvent(AnxOptions & options, const GUIEventHandler & handler) : Command(options)
{
	this->handler = handler;
}

AnxCmd_ExecuteEvent::~AnxCmd_ExecuteEvent()
{

}


//============================================================================================


bool AnxCmd_ExecuteEvent::IsEnable()
{
	return opt.project != null;
}

void AnxCmd_ExecuteEvent::Do()
{
	handler.Execute(null);
}

Command * AnxCmd_ExecuteEvent::CreateThisObject(AnxOptions & opt)
{	
	return NEW AnxCmd_ExecuteEvent(opt, handler);
}



