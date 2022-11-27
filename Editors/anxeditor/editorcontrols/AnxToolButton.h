//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxToolButton	
//============================================================================================

#ifndef _AnxToolButton_h_
#define _AnxToolButton_h_

#include "..\AnxProject.h"

class AnxMenu;

class AnxToolButton : public GUIButton
{
//--------------------------------------------------------------------------------------------
public:
	AnxToolButton(Command * _cmd, GUIControl* pParent, int Left, int Top, int Width, int Height, AnxMenu * _menu = null);
	virtual ~AnxToolButton();

//--------------------------------------------------------------------------------------------
private:
	virtual void Draw();
	virtual void DrawButtonDown(int Left, int Top, int Width, int Height);	
	virtual void DrawButtonUp(int Left, int Top, int Width, int Height);
	void DrawFlash(int Left, int Top, int Width, int Height);
	virtual void OnMDown(int mouseKey, const GUIPoint & p);
	virtual void OnPressed ();
	//Исполнить команду
	void _cdecl Execute(GUIControl * sender);


public:
	GUIRectangle Rect();
	AnxToolButton * NoShadow();

//--------------------------------------------------------------------------------------------
protected:
	Command * cmd;
	dword color;
	float flashTimer;
	AnxMenu * menu;
	bool isShadow;	
};

inline AnxToolButton * AnxToolButton::NoShadow()
{
	isShadow = false;
	return this;
}

#endif

