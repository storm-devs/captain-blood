#ifndef _XBOX

//****************************************************************

#include "gui_viewport.h"
#include "..\arcball\ArcBall.h"
#include "..\..\gizmo\gizmo.h"
#include "..\EditorMain.h"
#include "..\Forms\MainWindow.h"



GUIViewPort::GUIViewPort (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIControl (pParent)
{
	//pFont = NEW GUIFont ("SansSerif");
	pFont = NULL;
	
	DrawRect.Left = Left;
	DrawRect.Top = Top;
	DrawRect.Width = Width;
	DrawRect.Height = Height;
	
	ClientRect = DrawRect;

	t_OnRender = NEW GUIEventHandler;

}

GUIViewPort::~GUIViewPort ()
{
	delete t_OnRender;
	//delete pFont;
	GUIControl::~GUIControl ();
}

void GUIViewPort::Draw ()
{
	if (Visible == false) return;
	GUIControl::Draw ();

	int Width = DrawRect.Width;
	int Height = DrawRect.Height;
	
	GUIRectangle rect = GetDrawRect ();
	this->ClientToScreen (rect);
	

	GUIHelper::DrawDownBorder(rect.x, rect.y , Width, Height);

	cliper.Push();
	GUIRectangle r;
	r = rect;
	r.x += 2;
	r.y += 2;
	r.w = Width - 4;
	r.h = Height - 4;
	cliper.SetRectangle(r);
	t_OnRender->Execute(this);
	cliper.Pop();

	
}


bool GUIViewPort::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	if (pEditor->ArcBall()->GetMode() == ABM_DISABLED)
	{
		GUIPoint MousePos = GetMainControl()->Application->GetCursor()->GetPosition();
		GUIPoint LocalMousePos = MousePos;
		ScreenToClient(LocalMousePos);

		if (message == GUIMSG_LMB_DOWN) 
		{
			//GetMainControl()->Application->GetCursor()->Push();
			//GetMainControl()->Application->GetCursor()->SetCursor("peditor\\currotate");

			if (!pEditor->GetGizmo()->IsActiveMode())
			{
				pEditor->ArcBall()->HandleMessages(WM_LBUTTONDOWN, MousePos.x, MousePos.y);
			}
			pEditor->GetGizmo()->HandleMessages(WM_LBUTTONDOWN, LocalMousePos.x, LocalMousePos.y);
		}

		if (message == GUIMSG_RMB_DOWN) 
		{
			//GetMainControl()->Application->GetCursor()->Push();
			//GetMainControl()->Application->GetCursor()->SetCursor("peditor\\curscale");
			pEditor->ArcBall()->HandleMessages(WM_RBUTTONDOWN, MousePos.x, MousePos.y);
			pEditor->GetGizmo()->HandleMessages(WM_RBUTTONDOWN, LocalMousePos.x, LocalMousePos.y);
		}

		if (message == GUIMSG_MMB_DOWN) 
		{
			//GetMainControl()->Application->GetCursor()->Push();
			//GetMainControl()->Application->GetCursor()->SetCursor("peditor\\curmove");
			pEditor->ArcBall()->HandleMessages(WM_MBUTTONDOWN, MousePos.x, MousePos.y);
			pEditor->GetGizmo()->HandleMessages(WM_MBUTTONDOWN, LocalMousePos.x, LocalMousePos.y);
		}
	}
	

	return GUIControl::ProcessMessages (message, lparam, hparam);
}

void GUIViewPort::MouseUp (int button, const GUIPoint& pt)
{
	if (pEditor->ArcBall()->GetMode() != ABM_DISABLED)
	{
		//GetMainControl()->Application->GetCursor()->Pop();
	}
	
	GUIPoint MousePos = GetMainControl()->Application->GetCursor()->GetPosition();
	pEditor->ArcBall()->HandleMessages(WM_LBUTTONUP, MousePos.x, MousePos.y);
	pEditor->ArcBall()->HandleMessages(WM_RBUTTONUP, MousePos.x, MousePos.y);
	pEditor->ArcBall()->HandleMessages(WM_MBUTTONUP, MousePos.x, MousePos.y);


	


	GUIPoint LocalMousePos = MousePos;
	ScreenToClient(LocalMousePos);

	pEditor->GetGizmo()->HandleMessages(WM_LBUTTONUP, LocalMousePos.x, LocalMousePos.y);
	pEditor->GetGizmo()->HandleMessages(WM_RBUTTONUP, LocalMousePos.x, LocalMousePos.y);
	pEditor->GetGizmo()->HandleMessages(WM_MBUTTONUP, LocalMousePos.x, LocalMousePos.y);

	pEditor->FormMain->SaveGizmoMatrix();

}

void GUIViewPort::MouseMove (int button, const GUIPoint& pt)
{
	GUIPoint MousePos = GetMainControl()->Application->GetCursor()->GetPosition();
	pEditor->ArcBall()->HandleMessages (WM_MOUSEMOVE, MousePos.x, MousePos.y);

	GUIPoint LocalMousePos = MousePos;
	ScreenToClient(LocalMousePos);

	pEditor->GetGizmo()->HandleMessages (WM_MOUSEMOVE, LocalMousePos.x, LocalMousePos.y);
}


#endif