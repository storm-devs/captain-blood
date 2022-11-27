//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_OpenNodesList
//============================================================================================

#ifndef _AnxCmd_OpenNodesList_h_
#define _AnxCmd_OpenNodesList_h_

#include "Command.h"

class AnxCmd_OpenNodesList : public Command
{
//--------------------------------------------------------------------------------------------
public:
	AnxCmd_OpenNodesList(AnxOptions & options, GUIControl * listener, CONTROL_EVENT event);
	AnxCmd_OpenNodesList(AnxOptions & options, const GUIEventHandler & handler);
	virtual ~AnxCmd_OpenNodesList();

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

