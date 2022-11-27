//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_LinkState
//============================================================================================

#ifndef _AnxCmd_LinkState_h_
#define _AnxCmd_LinkState_h_

#include "Command.h"

class AnxCmd_LinkState : public Command
{
//--------------------------------------------------------------------------------------------
public:
	AnxCmd_LinkState(GraphLink * link);
	virtual ~AnxCmd_LinkState();

//--------------------------------------------------------------------------------------------
public:
	virtual bool IsEnable();
	virtual bool CanUndo();
	virtual void Do();
	virtual void UnDo();
	virtual void ReDo();

//--------------------------------------------------------------------------------------------
private:
	GraphLink * GetLink();

private:
	AnxStreamMemory undoData;
	AnxStreamMemory redoData;
	string fromNode, toNode;
	GraphLink * doLink;
};

#endif

