

#pragma once

#include "..\SndOptions.h"


class FormNewAnimation : public GUIWindow
{
	enum Consts
	{
		c_width = 600,
		c_height = 150,
		c_border = 5,
		c_space = 15,
		c_header_height = 30,
		c_edit_height = 25,
		c_nav_button_w = 30,
		c_button_width = 130,
		c_button_height = 25,
	};

public:
	FormNewAnimation(GUIControl * parent, const GUIPoint & p);
	virtual ~FormNewAnimation();

public:
	//Получить имя анимации, которую добавили
	const char * GetAnimationName();

private:
	//Рисование
	virtual void Draw();
	void _cdecl OnOkEdit(GUIControl* sender);
	void _cdecl OnMessageBoxOk(GUIControl* sender);
	void _cdecl OnCancelEdit(GUIControl* sender);
	void _cdecl OnEditChange(GUIControl* sender);
	void _cdecl OnEditAccept(GUIControl* sender);
	void _cdecl OnAnxChoisePath(GUIControl* sender);

public:
	GUIEventHandler onOk;

private:
	FormButton * buttonOk;
	FormButton * buttonCancel;
	FormButton * buttonAnxPath;	
	FormEdit * anxEdit;
	bool needAcceptAnx;
	string anxName;
};

