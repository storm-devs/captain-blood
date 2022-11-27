//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_NodeState
//============================================================================================

#ifndef _AnxCmd_NodeState_h_
#define _AnxCmd_NodeState_h_

#include "Command.h"

class AnxCmd_NodeState : public Command
{
//--------------------------------------------------------------------------------------------
public:
	AnxCmd_NodeState(GraphNodeBase * node);
	virtual ~AnxCmd_NodeState();

//--------------------------------------------------------------------------------------------
public:
	virtual bool IsEnable();
	virtual bool CanUndo();
	virtual void Do();
	virtual void UnDo();
	virtual void ReDo();

private:
	void WriteDataToStream(GraphNodeBase * node, AnxStreamMemory & stream);
	void ReadDataFromStream(GraphNodeBase * node, AnxStreamMemory & stream);


//--------------------------------------------------------------------------------------------
private:
	AnxStreamMemory undoData;
	AnxStreamMemory redoData;
	string undoNodeName;
	string redoNodeName;
	GraphNodeBase * doNode;
};

#endif

