//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxEditor
//============================================================================================
// NodeAnimationEventsForm
//============================================================================================

#ifndef _NodeAnimationEventsForm_h_
#define _NodeAnimationEventsForm_h_

#include "..\AnxBase.h"
#include "..\Graph\GraphNodeAnimation.h"
#include "..\EditorControls\AnxToolButton.h"
#include "..\EditorControls\AnxToolEdit.h"
#include "..\EditorControls\AnxToolListBox.h"

class NodeAnimationEventsForm : public GUIWindow
{
public:
	struct Param
	{
		GUILabel * label;		//Метка, описывающая параметр
		AnxToolEdit * edit;		//Поле редактирования
		GUIComboBox * combo;	//Выбор из заданных
		string value;			//Значение параметры
	};

//--------------------------------------------------------------------------------------------
public:
	NodeAnimationEventsForm(AnxOptions & options, GraphNodeAnimation::Event & _event);
	~NodeAnimationEventsForm();

//--------------------------------------------------------------------------------------------
public:
	virtual void Draw();
	void _cdecl EvtSelectType(GUIControl * sender);
	void _cdecl EvtOk(GUIControl * sender);
	void _cdecl EvtCancel(GUIControl * sender);
	void _cdecl EvtChangeComboValue(GUIControl * sender);

//--------------------------------------------------------------------------------------------
private:
	AnxOptions & opt;
	GraphNodeAnimation::Event & event;
	GUIComboBox * forms;
	AnxToolButton * buttonOk;
	AnxToolButton * buttonCancel;
	string name;
	array<Param *> params;	
};

#endif

