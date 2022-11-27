
#pragma once

#include "..\SndOptions.h"

class ProjectSoundBank;

class FormSoundBankOptions : public GUIWindow
{
	enum Consts
	{
		c_width = 600,
		c_height = 180,
		c_border = 5,
		c_space = 15,
		c_header_height = 20,
		c_edit_height = 25,
		c_nav_button_w = 30,
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
	FormSoundBankOptions(GUIControl * parent, const GUIPoint & p, ProjectSoundBank * bank);
	virtual ~FormSoundBankOptions();

public:


private:
	//Рисование
	virtual void Draw();
	void _cdecl OnOkEdit(GUIControl* sender);
	void _cdecl OnCancelEdit(GUIControl* sender);
	void _cdecl OnEditChange(GUIControl* sender);
	void _cdecl OnEditAccept(GUIControl* sender);
	void _cdecl OnChoisePath(GUIControl* sender);

public:
	FormButton * buttonOk;
	FormButton * buttonCancel;
	FormButton * buttonExpPath;
	FormEdit * edit;
	UniqId bankId;
};
















