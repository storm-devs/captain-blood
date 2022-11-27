//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_CopyEvent
//============================================================================================

#ifndef _AnxCmd_CopyEvent_h_
#define _AnxCmd_CopyEvent_h_

#include "Command.h"

class NodeAnimationForm;

class AnxCmd_CopyPasteEvent : public Command
{
//--------------------------------------------------------------------------------------------
public:
	AnxCmd_CopyPasteEvent(AnxOptions & options, NodeAnimationForm * _form, bool isCopy);
	virtual ~AnxCmd_CopyPasteEvent();

//--------------------------------------------------------------------------------------------
public:
	virtual bool IsEnable();
	virtual void Do();
	virtual Command * CreateThisObject(AnxOptions & opt);

//--------------------------------------------------------------------------------------------
private:
	NodeAnimationForm * form;
	bool isCopyEvent;
};

#endif

