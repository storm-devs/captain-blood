//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormEdit
//============================================================================================

#pragma once

#include "..\SndOptions.h"

class FormEdit : public GUIEdit
{
//--------------------------------------------------------------------------------------------
public:
	FormEdit(GUIControl* parent, const GUIRectangle & rect);
	virtual ~FormEdit();

//--------------------------------------------------------------------------------------------
private:
	virtual void DrawEditBox (long nLeft, long nTop, long Width, long Height);

	void _cdecl ChangeText(GUIControl * c);
	void _cdecl AcceptText(GUIControl * c);

public:
	void UpdateText();



public:
	bool isAccept;
	GUIEventHandler onAccept;
	GUIEventHandler onChange;

	//Автоматические значение для строки
	string * stringValue;
	//Автоматические значение для чисел
	float * floatValue;
	long * longValue;
	float min;
	float max;
	bool isLimitedNumber;
};

