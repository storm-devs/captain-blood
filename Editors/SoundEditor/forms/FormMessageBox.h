
#pragma once

#include "..\SndOptions.h"

class FormMessageBox : public GUIWindow
{
	enum Consts
	{
		c_width = 700,
		c_space = 5,
		c_header_height = 20,
		c_button_width = 130,
		c_button_height = 25,		
		c_height = c_header_height + c_button_height + c_space*4,
	};

public:
	enum PointPivot
	{
		pp_screenCenter,
		pp_left_top,
		pp_left_bottom,
		pp_right_top,
		pp_right_bottom,		
	};

	enum Mode
	{
		m_yesno,
		m_okcancel,
		m_ok,
	};


public:
	FormMessageBox(const char * caption, const char * message, Mode mode = m_okcancel, GUIControl * parent = null, PointPivot pp = pp_screenCenter, const GUIPoint * p = null);
	virtual ~FormMessageBox();

private:
	//Рисование
	virtual void Draw();
	void _cdecl DoOk(GUIControl* sender);
	void _cdecl DoCancel(GUIControl* sender);


public:
	GUIEventHandler onOk;
	GUIEventHandler onCancel;
	dword headerColor;

private:
	FormButton * buttonOk;
	FormButton * buttonCancel;
	ExtNameStr captionText;
	GUIPoint captionPos;
	ExtNameStr messageText;
	GUIPoint messagePos;
};
