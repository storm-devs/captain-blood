//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxEditor
//============================================================================================
// NodeAnimationConstsForm
//============================================================================================

#ifndef _NodeAnimationConstsForm_h_
#define _NodeAnimationConstsForm_h_

#include "..\AnxBase.h"
#include "..\Graph\GraphNodeAnimation.h"
#include "..\EditorControls\AnxToolButton.h"
#include "..\EditorControls\AnxToolEdit.h"
#include "..\EditorControls\AnxToolListBox.h"

class NodeAnimationConstsForm : public GUIWindow
{
public:
	struct Param
	{
		string name;			//Имя константы
		GUILabel * label;		//Метка, описывающая параметр
		AnxToolEdit * edit;		//Поле редактирования
		GUIComboBox * combo;	//Выбор из заданных
		string svalue;			//Строковое значение параметра
		float fvalue;			//Числовое значение параметра
		float bvalue;			//Блендящееся значение параметра
		GraphNodeAnimation::Const::Type type;
	};

//--------------------------------------------------------------------------------------------
public:
	NodeAnimationConstsForm(AnxOptions & options, GraphNodeAnimation::ConstBlock & _cb);
	~NodeAnimationConstsForm();

//--------------------------------------------------------------------------------------------
public:
	virtual void Draw();
	void _cdecl EvtSelectType(GUIControl * sender);
	void _cdecl EvtOk(GUIControl * sender);
	void _cdecl EvtCancel(GUIControl * sender);
	void _cdecl EvtChangeComboValue(GUIControl * sender);

	void InitStatdartForm(AnxOptions::ConstForm & cf, long & y);
	void InitAttackForm(AnxOptions::ConstForm & cf, long & y);

//--------------------------------------------------------------------------------------------
private:
	AnxOptions & opt;
	GraphNodeAnimation::ConstBlock & cb;
	GUIComboBox * forms;
	AnxToolButton * buttonOk;
	AnxToolButton * buttonCancel;
	GUILabel * errorForm;
	GUIControl * paramsForm;
	array<Param *> params;
};

#endif

