//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// NodeAnimationForm	
//============================================================================================

#ifndef _NodeAnimationForm_h_
#define _NodeAnimationForm_h_

#include "..\AnxBase.h"
#include "..\Graph\GraphNodeAnimation.h"
#include "..\EditorControls\AnxToolButton.h"
#include "..\EditorControls\AnxToolEdit.h"
#include "..\EditorControls\AnxAnimationViewer.h"
#include "..\Commands\AnxCmd_NodeState.h"

class NodeAnimationForm : public GUIWindow
{
//--------------------------------------------------------------------------------------------
public:
	NodeAnimationForm(GraphNodeAnimation * _node);
	virtual ~NodeAnimationForm();
	virtual void OnCreate();

//--------------------------------------------------------------------------------------------
public:
	virtual void Draw();
	void _cdecl EvtOk(GUIControl * sender);
	void _cdecl EvtCancel(GUIControl * sender);
	void _cdecl EvtAddNewClip(GUIControl * sender);
	void _cdecl EvtAddNewClipOk(GUIControl * sender);
	void _cdecl EvtAddNewClipCancel(GUIControl * sender);
	void _cdecl EvtReloadClip(GUIControl * sender);
	void _cdecl EvtReloadClipOk(GUIControl * sender);
	void _cdecl EvtReloadClipCancel(GUIControl * sender);
	void _cdecl EvtDeleteClip(GUIControl * sender);
	void _cdecl EvtMoveClip(GUIControl * sender);
	void _cdecl EvtReverseClip(GUIControl * sender);
	void _cdecl EvtReduceClip(GUIControl * sender);
	void _cdecl EvtSelectClip(GUIControl * sender);
	void _cdecl EvtClipNameChangle(GUIControl * sender);
	void _cdecl EvtUpdateFPS(GUIControl * sender);
	void _cdecl EvtUpdateMinFrame(GUIControl * sender);
	void _cdecl EvtUpdateMaxFrame(GUIControl * sender);
	void _cdecl EvtUpdatePrecisions(GUIControl * sender);
	void _cdecl EvtAddNewEvent(GUIControl * sender);
	void _cdecl EvtDeleteEvent(GUIControl * sender);
	void _cdecl EvtSelectEvent(GUIControl * sender);
	void _cdecl EvtEventFrameUpdate(GUIControl * sender);
	void _cdecl EvtEventInfoChangle(GUIControl * sender);
	void _cdecl EvtShowEventParams(GUIControl * sender);
	void _cdecl EvtShowChangeCheck(GUIControl * sender);
	void _cdecl EvtChangeStartNode(GUIControl * sender);
	void _cdecl EvtChangeStopNode(GUIControl * sender);
	void _cdecl EvtChangeMovement(GUIControl * sender);
	void _cdecl EvtChangeGlobalPos(GUIControl * sender);
	void _cdecl EvtConstNameChangle(GUIControl * sender);
	void _cdecl EvtSelectConst(GUIControl * sender);
	void _cdecl EvtAddNewConst(GUIControl * sender);
	void _cdecl EvtDeleteConst(GUIControl * sender);
	void _cdecl EvtEditConst(GUIControl * sender);
	void _cdecl EvtUpdateSkipFPSChange(GUIControl * sender);

	bool IsEnableEventCopy();
	bool CopyEvent(AnxStream & stream);
	void PasteEvent(AnxStream & stream);

//--------------------------------------------------------------------------------------------
private:
	//Обновить список линков
	void UpdateClipsList();
	//Обновить список событий
	void UpdateEventsList();
	//Обновить список событий
	void UpdateConstsList();

private:
	GraphNodeAnimation * node;
	AnxToolEdit * editName;
	GUIComboBox * clips;
	GUIComboBox * events;
	GUIComboBox * consts;
	GUICheckBox * flgStart;
	GUICheckBox * flgStop;
	GUICheckBox * flgLoop;
	GUICheckBox * flgChange;
	GUICheckBox * flgMovement;
	GUICheckBox * flgGlobalPos;
	AnxCmd_NodeState * nodeState;
	GUIFileOpen * dfo;
	AnxAnimationViewer * viewer;
	long selectedClip;
	AnxToolEdit * editProb;
	AnxToolEdit * editFPS;
	AnxToolEdit * editMin;
	AnxToolEdit * editMax;
	AnxToolEdit * editQP;
	AnxToolEdit * editPP;
	AnxToolEdit * editSP;
	long selectEvent;
	AnxToolEdit * editEvt;
	string tmp;
	long minFrame, maxFrame, maxFrames;
	float vQP, vPP, vSP;

	GUIEventHandler* t_OnOK;
	GUIControl * t_OnOK_Sender;
};

#endif

