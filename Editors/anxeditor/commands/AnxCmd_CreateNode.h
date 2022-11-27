//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_CreateNode
//============================================================================================

#ifndef _AnxCmd_CreateNode_h_
#define _AnxCmd_CreateNode_h_

#include "Command.h"

class AnxCmd_CreateNode : public Command
{
//--------------------------------------------------------------------------------------------
public:
	AnxCmd_CreateNode(AnxNodeTypes t, AnxOptions & options);
	virtual ~AnxCmd_CreateNode();

//--------------------------------------------------------------------------------------------
public:
	virtual bool IsEnable();
	virtual bool CanUndo();
	virtual void Do();
	virtual void UnDo();
	virtual void ReDo();
	virtual Command * CreateThisObject(AnxOptions & opt);

	//Создать объект нода по типу
	static GraphNodeBase * CreateNodeByType(AnxNodeTypes type, AnxOptions & options);

//--------------------------------------------------------------------------------------------
private:
	string nodeName;
	AnxNodeTypes type;
};

#endif

