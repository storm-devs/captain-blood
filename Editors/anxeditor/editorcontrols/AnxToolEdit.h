//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxToolEdit	
//============================================================================================

#ifndef _AnxToolEdit_h_
#define _AnxToolEdit_h_

#include "..\AnxProject.h"

// Для навешивания обработчика приравниванием :)
#define OnEditUpdate \
	t_OnEditUpdate->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

class AnxToolEdit : public GUIEdit  
{
//--------------------------------------------------------------------------------------------
public:
	AnxToolEdit(GUIControl* pParent, int Left, int Top, int Width, int Height);
	virtual ~AnxToolEdit();

//--------------------------------------------------------------------------------------------
public:
	virtual void DrawEditBox (long nLeft, long nTop, long Width, long Height);

	void _cdecl ChangeText(GUIControl * c);
	void _cdecl AcceptText(GUIControl * c);

	void UpdateText();

	bool isAccept;
	GUIEventHandler * t_OnEditUpdate;

	//Автоматические значение для строки
	string * stringValue;
	//Автоматические значение для чисел
	float * floatValue;
	long * longValue;
	float min;
	float max;
	bool isLimitedNumber;
};

#endif

