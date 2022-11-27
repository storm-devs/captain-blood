//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// NodeInOutForm	
//============================================================================================

#ifndef _NodeInOutForm_h_
#define _NodeInOutForm_h_

#include "..\AnxBase.h"
#include "..\Graph\GraphNodeInOut.h"
#include "..\EditorControls\AnxToolButton.h"
#include "..\EditorControls\AnxToolEdit.h"
#include "..\Commands\AnxCmd_NodeState.h"

class NodeInOutForm : public GUIWindow  
{
//--------------------------------------------------------------------------------------------
public:
	NodeInOutForm(GraphNodeInOut * _node);
	virtual ~NodeInOutForm();
	virtual void OnCreate();

//--------------------------------------------------------------------------------------------
public:
	//Рисование
	virtual void Draw();

	void _cdecl EvtOk(GUIControl * sender);
	void _cdecl EvtCancel(GUIControl * sender);

//--------------------------------------------------------------------------------------------
private:
	GraphNodeInOut * node;
	GUIEdit * editName;
	GUIEdit * editLink;
	AnxCmd_NodeState * nodeState;
};

#endif

