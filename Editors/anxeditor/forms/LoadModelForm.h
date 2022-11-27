//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// LoadModelForm	
//============================================================================================

#ifndef _LoadModelForm_h_
#define _LoadModelForm_h_

#include "..\AnxBase.h"
#include "..\EditorControls\AnxToolButton.h"
#include "..\EditorControls\AnxToolEdit.h"
#include "..\EditorControls\AnxAnimationViewer.h"

class LoadModelForm : public GUIWindow  
{
//--------------------------------------------------------------------------------------------
public:
	LoadModelForm(AnxOptions & options);
	virtual ~LoadModelForm();
	virtual void OnCreate();

//--------------------------------------------------------------------------------------------
public:
	//Рисование
	virtual void Draw();

	void _cdecl EvtOk(GUIControl * sender);
	void _cdecl EvtCancel(GUIControl * sender);
	void _cdecl EvtBrowse(GUIControl * sender);
	void _cdecl EvtBrowseOk(GUIControl * sender);
	void _cdecl EvtBrowseCancel(GUIControl * sender);
	void _cdecl EvtReloadModel(GUIControl * sender);

//--------------------------------------------------------------------------------------------
private:
	AnxOptions & opt;
	AnxToolEdit * editName;
	AnxAnimationViewer * viewer;
	GUIFileOpen * dfo;
};

#endif

