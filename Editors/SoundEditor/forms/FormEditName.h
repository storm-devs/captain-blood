
#pragma once

#include "..\SndOptions.h"

class FormEditName : public GUIWindow
{
	enum Consts
	{
		c_width = 300,
		c_height = 100,
		c_border = 5,
		c_header_height = 20,
		c_edit_height = 25,
		c_button_width = 130,
		c_button_height = 25,		
	};

public:
	enum PointPivot
	{
		pp_left_top,
		pp_left_bottom,
		pp_right_top,
		pp_right_bottom,
	};


public:
	FormEditName(GUIControl * parent, const GUIPoint & p, PointPivot pp);
	virtual ~FormEditName();

public:
	//Установить текст
	void SetText(const char * str);
	//Получить текст
	const char * GetText();
	//Текущий текст негодиться
	void MarkTextAsIncorrect();

private:
	//Рисование
	virtual void Draw();
	virtual void OnCreate();
	void _cdecl OnOkEdit(GUIControl* sender);
	void _cdecl OnCancelEdit(GUIControl* sender);
	void _cdecl OnEditChange(GUIControl* sender);
	void _cdecl OnEditAccept(GUIControl* sender);

public:
	GUIEventHandler onOk;
	GUIEventHandler onCancel;
	GUIEventHandler onCheck;

private:
	FormButton * buttonOk;
	FormButton * buttonCancel;
	FormEdit * edit;
};

































