//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_CreateLink
//============================================================================================

#ifndef _AnxCmd_CreateLink_h_
#define _AnxCmd_CreateLink_h_

#include "Command.h"

class GraphNodeBase;

class AnxCmd_CreateLink : public Command
{
//--------------------------------------------------------------------------------------------
public:
	AnxCmd_CreateLink(AnxOptions & options, GraphNodeBase * from, GraphNodeBase * to);
	virtual ~AnxCmd_CreateLink();

//--------------------------------------------------------------------------------------------
public:
	virtual bool IsEnable();
	virtual bool CanUndo();
	virtual void Do();
	virtual void UnDo();
	virtual void ReDo();

//--------------------------------------------------------------------------------------------
private:
	string fromName;	//От этого нода
	string toName;		//К этому ноду
	string currentName;	//Текущий установленный нод


};

#endif

