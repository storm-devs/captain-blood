#ifndef _XBOX

#include "..\..\render.h"
#include "..\..\controls.h"
#include "gui_cursor.h"
#include "gui_helper.h"

GUICursor::GUICursor (const char* FileName) : Cursors (_FL_, 32)
{
	cOffsetX = 0;
	cOffsetY = 0;

	cursor = NULL;
	ctrl = null;
	
	IRender* rs = (IRender*)api->GetService("DX9Render");
	cursor = NEW GUIImage (FileName);
	CurrentCursor = FileName;
	
	
	
	POINT real_pos;
	GetCursorPos(&real_pos);
	ScreenToClient ((HWND)api->Storage().GetLong("system.hwnd"), &real_pos); 
	
	position.X = real_pos.x;
	position.Y = real_pos.y;
	
	
	DeltaX = 0.0f;
	DeltaY = 0.0f;

	IControlsService * ctrlsrv = (IControlsService *)api->GetService("ControlsService");
	Assert(ctrlsrv);
	ctrl = ctrlsrv->CreateInstance(_FL_);
	Assert(ctrl);
	
}

GUICursor::~GUICursor ()
{
	RELEASE(ctrl);
	delete cursor;
}


void GUICursor::Update ()
{
	
	DeltaX = 0.0f;
	DeltaY = 0.0f;
	
	DeltaX = ctrl->GetControlStateFloat ("GUICursor_MoveX");
	DeltaY = ctrl->GetControlStateFloat ("GUICursor_MoveY");

	/*
	//api->Trace("Mouse Delta %3.2f, %3.2f\n", DeltaX, DeltaY);
	
	//if (fabs (DeltaX > 0)) _asm int 3;
	//if (fabs (DeltaY > 0)) _asm int 3;
	
	position.X += (int)DeltaX;
	position.Y += (int)DeltaY;
	*/
	//long x, y;
	//ctrl->GetMouseCursorPos(x, y);

	
	position.X = (int)ctrl->GetControlStateFloat("MouseH");
	position.Y = (int)ctrl->GetControlStateFloat("MouseV");

	IRender* rs = (IRender*)api->GetService("DX9Render");
	int MaxHeight = rs->GetScreenInfo3D().dwHeight-2;
	int MaxWidth = rs->GetScreenInfo3D().dwWidth-2;
	if (position.Y > MaxHeight) position.Y = MaxHeight;
	if (position.Y < 2) position.Y = 2;
	if (position.X > MaxWidth) position.X = MaxWidth;
	if (position.X < 2) position.X = 2;
	
	
}

void GUICursor::Draw ()
{
	if (cursor) GUIHelper::DrawSprite (position.X+cOffsetX, position.Y+cOffsetY, cursor->GetWidth(), cursor->GetHeight(), cursor);
} 

const GUIPoint& GUICursor::GetPosition ()
{
	return position;
}

void GUICursor::SetPosition (const GUIPoint& pos)
{
	position = pos;
}

void GUICursor::Push ()
{
	SavedCursor* nEntry = &Cursors[Cursors.Add ()];
	nEntry->Name = CurrentCursor;
	nEntry->OfsX = cOffsetX;
	nEntry->OfsY = cOffsetY;

}

void GUICursor::Pop ()
{
	int n = Cursors.Size ();
	if (n <= 0) return;
	n--;

	SetCursor (Cursors[n].Name.GetBuffer (), Cursors[n].OfsX, Cursors[n].OfsY);
	Cursors.DelIndex (n);
}

void GUICursor::SetCursor (const char* name, int offsetY, int offsetX)
{
	if (!string::IsEqual (CurrentCursor.GetBuffer (), name) )
	{
		cursor->Load (name);
		CurrentCursor = name;
	}

	cOffsetX = offsetX;
	cOffsetY = offsetY;
}

GUIImage* GUICursor::GetImage ()
{
	return cursor;
}

int GUICursor::GetOffsetX ()
{
	return cOffsetX;
}

int GUICursor::GetOffsetY ()
{
	return cOffsetY;
}

#endif