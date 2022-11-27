//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// LinkForms
//============================================================================================

#ifndef _LinkForm_h_
#define _LinkForm_h_

#include "..\AnxBase.h"
#include "..\Graph\GraphNodeAnimation.h"
#include "..\EditorControls\AnxToolButton.h"
#include "..\EditorControls\AnxToolEdit.h"
#include "..\EditorControls\AnxAnimationViewer.h"
#include "..\EditorControls\AnxToolListBox.h"
#include "..\Commands\AnxCmd_LinkState.h"

class LinkForm;

//============================================================================================
//LinkFormPanel
//============================================================================================

class LinkFormPanel : public GUIWindow  
{
//--------------------------------------------------------------------------------------------
public:
	LinkFormPanel(GraphLink * _link, bool isFake);
	virtual ~LinkFormPanel();

//--------------------------------------------------------------------------------------------
public:	
	virtual void Draw();

	void _cdecl EvtOk(GUIControl * sender);
	void _cdecl EvtCancel(GUIControl * sender);
	void _cdecl EvtSelFromNode(GUIControl * sender);
	void _cdecl EvtSetSelFromNode(GUIControl * sender);
	void _cdecl EvtSelToNode(GUIControl * sender);
	void _cdecl EvtSetSelToNode(GUIControl * sender);	
	
	void MakeSelFromToNodeList(GUIControl * sender, GraphNodeBase * findNode, AnxNodeTypes type, CONTROL_EVENT event);

//--------------------------------------------------------------------------------------------
private:
	GraphLink * link;
	LinkForm * form;
	GUILabel * lb;
	AnxCmd_LinkState * linkState;
	AnxToolEdit * editFrom;
	AnxToolEdit * editTo;
	array<string> inOutNames;
};

//============================================================================================
//LinkFormGroup
//============================================================================================

class LinkFormGroup : public GUIWindow  
{
//--------------------------------------------------------------------------------------------
public:
	LinkFormGroup(AnxOptions & options, GUIPoint point, LinkData & _data, string & toNode);
	virtual ~LinkFormGroup();

//--------------------------------------------------------------------------------------------
public:	
	virtual void Draw();

//--------------------------------------------------------------------------------------------
private:
	LinkForm * form;
	GUILabel * lb;
};

//============================================================================================
//LinkForm
//============================================================================================

class LinkForm : public GUIControl
{
//--------------------------------------------------------------------------------------------
public:
	LinkForm(GUIWindow * parent, AnxOptions & options, LinkData & _data, GUIPoint pos);
	virtual ~LinkForm();
	void OnCreate();

//--------------------------------------------------------------------------------------------
public:	
	void _cdecl EvtOk(GUIControl * sender);
	void _cdecl EvtCancel(GUIControl * sender);
	void _cdecl EvtChangeSync(GUIControl * sender);
	void _cdecl EvtChangeDef(GUIControl * sender);

	GUIEventHandler eventOk;
	GUIEventHandler eventCancel;

	virtual void Draw();

//--------------------------------------------------------------------------------------------
private:
	AnxOptions & opt;
	LinkData & data;
	GUIComboBox * editName;
	GUICheckBox * flgSync;
	AnxToolEdit * editSync;
	GUICheckBox * flgDef;
	AnxToolEdit * editAct[2];
	AnxToolEdit * editMove[2];
};


#endif

