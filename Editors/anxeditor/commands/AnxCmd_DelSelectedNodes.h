//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_DelSelectedNodes
//============================================================================================

#ifndef _AnxCmd_DelSelectedNodes_h_
#define _AnxCmd_DelSelectedNodes_h_

#include "Command.h"

class AnxCmd_DelSelectedNodes : public Command  
{
//--------------------------------------------------------------------------------------------
public:
	AnxCmd_DelSelectedNodes(AnxOptions & options);
	virtual ~AnxCmd_DelSelectedNodes();

//--------------------------------------------------------------------------------------------
public:
	virtual bool IsEnable();
	virtual bool CanUndo();
	virtual void Do();
	virtual void UnDo();
	virtual void ReDo();
	virtual Command * CreateThisObject(AnxOptions & opt);



//--------------------------------------------------------------------------------------------
private:
	AnxStreamMemory data;
};

#endif

