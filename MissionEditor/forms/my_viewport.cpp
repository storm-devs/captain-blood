#include "my_viewport.h"

#include "..\..\Common_h\InputSrvCmds.h"
#include "..\..\Common_h\corecmds.h"
#include "..\movecontroller.h"
#include "..\rotatecontroller.h"
#include "..\selector.h"
extern MoveController* MoveControl;
extern RotateController* RotateControl;
extern ObjectSelector* SelectControl; 
extern IMission* miss;

extern int AdditionalWidth;
extern bool bBigPanel;

float TimeFromLastPress = 100.0f;
extern bool bWasPressed;
extern IRender * pRS; // Рендер
	
TViewPort::TViewPort (GUIControl* pParent, int Left, int Top, int Width, int Height) : GUIPanel (pParent, Left, Top, Width, Height)
{
	initialX = Left;
	initialWidth = Width;
	bActive = false;
	isFly = false;
}
	
TViewPort::~TViewPort ()
{
}

bool TViewPort::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	//GUIMSG_RMB_DOWN
 

 GUIPoint ptCursor;
 GUIHelper::ExtractCursorPos (message, lparam, hparam, ptCursor);
 ScreenToClient(ptCursor);

 GUIRectangle sc_rect = ClientRect;
 ClientToScreen(sc_rect);
 RENDERVIEWPORT viewport;
 viewport.Width = sc_rect.w;
 viewport.Height = sc_rect.h;
 viewport.X = sc_rect.x;
 viewport.Y = sc_rect.y;
 viewport.MinZ = 0.0f;
 viewport.MaxZ = 1.0f;

 if (message == GUIMSG_MOUSEMOVE)
 {
	 MoveControl->MouseMove(ptCursor, viewport);
	 RotateControl->MouseMove(ptCursor, viewport);
	 SelectControl->MouseMove(ptCursor, viewport);
 }

 if (message == GUIMSG_LMB_DOWN)
 {
		bool need1 = MoveControl->ButtonIsPressed(true, ptCursor, viewport);
		bool need2 = RotateControl->ButtonIsPressed(true, ptCursor, viewport);
		if (GetAsyncKeyState(VK_CONTROL) < 0)
		{
			SelectControl->ButtonIsPressed(true, ptCursor, viewport);

			// Если в режиме move или rotate нужно выделить объет...
			if ((need1==false) || (need2==false))
			{
				bool saveflag = SelectControl->GetIsActive();
				SelectControl->Activate(true);
				SelectControl->ButtonIsPressed(true, ptCursor, viewport);
				SelectControl->Activate(saveflag);
			}
		}


 }

 

 return GUIPanel::ProcessMessages (message, lparam, hparam);
}

  
void TViewPort::MouseUp (int button, const GUIPoint& pt)
{
	if (button != GUIMSG_LMB_UP) return;

	GUIPoint ptCursor;
	RENDERVIEWPORT viewport;

  MoveControl->ButtonIsPressed(false, ptCursor, viewport);
  RotateControl->ButtonIsPressed(false, ptCursor, viewport);
  SelectControl->ButtonIsPressed(false, ptCursor, viewport);

}
//
void TViewPort::Draw ()
{/*
	if (!bBigPanel)
	{
		DrawRect.Left = initialX;
		DrawRect.Width = initialWidth;
	} else*/
	{
		DrawRect.Left = initialX + AdditionalWidth;
		DrawRect.Width = initialWidth - AdditionalWidth;
	}

	ClientRect = DrawRect;

 //TimeFromLastPress += api->GetDeltaTime();

 // Enter to fly mode...
	if ((miss->Controls().GetControlStateType("ExitFromFreeFly") == CST_ACTIVATED)/* && (TimeFromLastPress > 0.3f)*/)
	{
		isFly = !isFly;
		if(isFly)
		{
			api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(false));
			//miss->Controls().LockDebugKeys(false);
			//api->Controls->EnableControlGroup ("mission", true, true, true);
			miss->Controls().EnableControlGroup ("FreeCamera", true);
			miss->Controls().EnableControlGroup ("GUI", false);
			GetMainControl()->Application->ShowCursor (false);
			bActive = true;
			bWasPressed = true;
			TimeFromLastPress = 0.0f;
			//miss->Controls().LockMouseCursorPos(true);
			miss->Controls().ExecuteCommand(InputSrvLockMouse(true));
		}else{
			TimeFromLastPress = 0.0f;
			miss->Controls().EnableControlGroup ("GUI", true);
			//api->Controls->EnableControlGroup ("mission", false, true, true);
			miss->Controls().EnableControlGroup ("FreeCamera", false);
			GetMainControl()->Application->ShowCursor (true);

			bActive = false;
			//miss->Controls().LockMouseCursorPos(false);
			miss->Controls().ExecuteCommand(InputSrvLockMouse(false));
		}

	}
	
	
	
 




	DWORD color = 0xFF363F4B;
	if (bActive) color = 0xFFFFFF00;

	int Width = DrawRect.Width;
	int Height = DrawRect.Height;
	GUIRectangle rect = GetDrawRect ();
	this->ClientToScreen (rect);
	GUIHelper::DrawWireRect(rect.x, rect.y , Width, Height, color);
	GUIHelper::DrawWireRect(rect.x-1, rect.y-1 , Width+2, Height+2, color);

}


void TViewPort::KeyPressed (int key, bool bSysKey)
{
  if (!bSysKey) return;

	/*
	if (key == 27 && bSysKey)
	{
		if (bActive)
		{
			//api->Controls->LockDebugKeys(true);
			miss->Controls().EnableControlGroup ("GUI", true, true, true);
			//api->Controls->EnableControlGroup ("mission", false, true, true);
			miss->Controls().EnableControlGroup ("FreeCamera", false, true, true);
			GetMainControl()->Application->ShowCursor (true);

			bActive = false;
		}
	}*/

}
