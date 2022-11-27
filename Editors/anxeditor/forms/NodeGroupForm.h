//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// NodeGroupForm	
//============================================================================================

#ifndef _NodeGroupForm_h_
#define _NodeGroupForm_h_

#include "..\AnxBase.h"
#include "..\Graph\GraphNodeGroup.h"
#include "..\EditorControls\AnxToolButton.h"
#include "..\EditorControls\AnxToolEdit.h"
#include "..\EditorControls\AnxItemsList.h"
#include "..\Commands\AnxCmd_NodeState.h"

class NodeGroupForm : public GUIWindow
{
//--------------------------------------------------------------------------------------------
public:
	NodeGroupForm(GraphNodeGroup * _node);
	virtual ~NodeGroupForm();
	virtual void OnCreate();

//--------------------------------------------------------------------------------------------
public:
	virtual void Draw();
	void _cdecl EvtOk(GUIControl * sender);
	void _cdecl EvtCancel(GUIControl * sender);
	void _cdecl EvtAddNewItemToList(GUIControl * sender);
	void _cdecl EvtDeleteItemFromList(GUIControl * sender);
	void _cdecl EvtChangeCheckLoop(GUIControl * sender);

//--------------------------------------------------------------------------------------------
private:
	GraphNodeGroup * node;
	GUIEdit * editName;
	AnxItemsList * list;
	AnxCmd_NodeState * nodeState;
	GUICheckBox * flgLoop;
};

#endif

