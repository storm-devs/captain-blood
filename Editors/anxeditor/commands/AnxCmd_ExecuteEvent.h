//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_ExecuteEvent
//============================================================================================

#ifndef _AnxCmd_ExecuteEvent_h_
#define _AnxCmd_ExecuteEvent_h_

#include "Command.h"

class AnxCmd_ExecuteEvent : public Command
{
//--------------------------------------------------------------------------------------------
public:
	AnxCmd_ExecuteEvent(AnxOptions & options, GUIControl * listener, CONTROL_EVENT event);
	AnxCmd_ExecuteEvent(AnxOptions & options, const GUIEventHandler & handler);
	virtual ~AnxCmd_ExecuteEvent();

//--------------------------------------------------------------------------------------------
public:
	virtual bool IsEnable();
	virtual void Do();
	virtual Command * CreateThisObject(AnxOptions & opt);

//--------------------------------------------------------------------------------------------
private:
	GUIEventHandler handler;
};

#endif

