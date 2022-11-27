//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxToolButton	
//============================================================================================
			
#include "..\Commands\Command.h"

#include "AnxToolButton.h"
#include "AnxMenu.h"

//============================================================================================

AnxToolButton::AnxToolButton(Command * _cmd, GUIControl* pParent, int Left, int Top, int Width, int Height, AnxMenu * _menu) : GUIButton(pParent, Left, Top, Width, Height)
{
	cmd = _cmd;
	if(cmd || _menu)
	{
		this->OnMousePressed = (CONTROL_EVENT)&AnxToolButton::Execute;
	}
	color = 0xcfe0e0f0;
	flashTimer = 0.0f;
	menu = _menu;
	Align = GUIAL_Left;
	isShadow = true;
}

AnxToolButton::~AnxToolButton()
{
	if(cmd) delete cmd;
}


//============================================================================================

void AnxToolButton::Draw()
{
	GUIButton::Draw();
	if(cmd && !cmd->IsEnable())
	{		
		GUIRectangle rect = DrawRect;
		ClientToScreen(rect);
		if(!menu)
		{
			GUIHelper::Draw2DRectAlpha(rect.x + 1, rect.y + 1, rect.w - 1, rect.h - 1, (color & 0x00ffffff) | 0xcf000000);
		}else{
			GUIHelper::Draw2DRectAlpha(rect.x, rect.y, rect.w, rect.h, (color & 0x00ffffff) | 0xcf000000);
		}
	}else
	if(menu && menu->Application->GetElementUnderCursor() == this)
	{
		GUIRectangle rect = DrawRect;
		ClientToScreen(rect);
		GUIHelper::Draw2DRectAlpha(rect.x, rect.y, rect.w, rect.h, 0x2f0f0f2f);
	}
	if(flashTimer > 0.0f)
	{
		if(flashTimer > 1.0f) flashTimer = 1.0f;
		flashTimer -= 2.0f*api->GetDeltaTime();
		if(flashTimer < 0.0f) flashTimer = 0.0f;
	}
}

void AnxToolButton::DrawButtonDown(int Left, int Top, int Width, int Height)
{
	if(menu) return;
	GUIHelper::Draw2DRectAlpha(Left + 1, Top + 1, Width, Height, 0x1f000000);
	GUIHelper::Draw2DRectAlpha(Left + 1, Top + 1, Width, Height, (color & 0x00ffffff) | 0xdf000000);
	GUIHelper::DrawLinesBox(Left + 1, Top + 1, Width, Height, 0x8f000000);
	DrawFlash(Left + 1, Top + 1, Width, Height);
}

void AnxToolButton::DrawButtonUp(int Left, int Top, int Width, int Height)
{
	if(menu) return;
	if(!cmd || cmd->IsEnable())
	{
		if(isShadow) GUIHelper::Draw2DRectAlpha(Left + 4, Top + 4, Width, Height, 0x1f000000);
	}
	GUIHelper::Draw2DRectAlpha(Left, Top, Width, Height, (color & 0x00ffffff) | 0xcf000000);
	GUIHelper::DrawLinesBox(Left, Top, Width, Height, 0x8f000000);
	DrawFlash(Left, Top, Width, Height);
}

void AnxToolButton::DrawFlash(int Left, int Top, int Width, int Height)
{
	if(flashTimer > 0.0f)
	{
		if(flashTimer > 1.0f) flashTimer = 1.0f;
		float k = flashTimer*flashTimer;
		dword clr = 0x00ffffff | dword(k*255.0f*0.8f) << 24;
		GUIHelper::DrawWireRect(Left, Top, Width, Height, clr);
	}
}

void AnxToolButton::OnMDown(int mouseKey, const GUIPoint & p)
{
	if(!cmd || cmd->IsEnable()) GUIButton::OnMDown(mouseKey, p);
}

void AnxToolButton::OnPressed ()
{
	GUIButton::OnPressed ();
	flashTimer = 1.0f;
}

//Исполнить команду
void _cdecl AnxToolButton::Execute(GUIControl * sender)
{
	if(cmd) cmd->Process();
	if(menu) menu->Close(this);
}

GUIRectangle AnxToolButton::Rect()
{
	GUIRectangle rect = DrawRect;
	ClientToScreen(rect);
	return rect;
}