#ifndef _XBOX

#include "gui_panel.h"


GUIPanel::GUIPanel (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ClientRect = DrawRect;

	t_OnBeforeDraw = NEW GUIEventHandler;
	t_OnMouseDown = NEW GUIEventHandler;
}

GUIPanel::~GUIPanel ()
{
	delete t_OnMouseDown;
	delete t_OnBeforeDraw;
}

void GUIPanel::Draw ()
{
	t_OnBeforeDraw->Execute(this);
	GUIControl::Draw ();

	int Width = DrawRect.Width;
	int Height = DrawRect.Height;
	GUIRectangle rect = GetDrawRect ();
	this->ClientToScreen (rect);
	GUIHelper::DrawDownBorder(rect.x, rect.y , Width, Height);
}


bool GUIPanel::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	if (message == GUIMSG_LMB_DOWN)	t_OnMouseDown->Execute(this);

	
	return GUIControl::ProcessMessages (message, lparam, hparam);
}



#endif