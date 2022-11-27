#ifndef _XBOX


#include "gui_manager.h"
#include "..\..\common_h\gui\controls\gui_helper.h"
#include "..\..\common_h\gui\controls\gui_messagebox.h"
#include "..\..\common_h\gui_controls.h"
#include "..\..\Common_h\FileService.h"
#include "..\..\common_h\render.h"

INTERFACE_FUNCTION
CREATE_CLASS(GUIManager)


char sysCurrentDirectory[MAX_PATH];

GUICliper cliper;

GUIManager::GUIManager () : CachedImages(_FL_, 128),
							              DefferedEvents(_FL_, 128),
														ModalWindows (_FL_, 64),
														RegistredWindows (_FL_, 64),
														WindowsToDelete (_FL_, 64)
{

	HintTime = 0.0f;
	MP_CursorPos = GUIPoint (0, 0);
	tFirstMousePressed = NULL;
	dragfrom = NULL;
	bDragIsBegin = false;
	memset (sysCurrentDirectory, 0, MAX_PATH);
	GetCurrentDirectory (MAX_PATH, sysCurrentDirectory);
	pHint.Enabled = false;

	cd_object = NULL;
	cd_custom_draw = NULL;



	active_combobox = NULL;


	modal_mode = false;
	CurrentAllocID = 0;
	ModalModeID = -1;

	GlobalDelete = false;
	message_box_window = NULL;
	
	modal_result = 0;
	element_under_mouse = NULL;
	
	keyboard_focus = NULL;
	pCursor = NULL;
	
	
	click_state = 0;
	
	active_window = NULL;
	


	bShowCursor = true;
	
	
	pTextureVar = NULL;
	ctrl = null;
}

bool GUIManager::Init ()
{
	Enable (true);

	IControlsService * ctrlsrv = (IControlsService *)api->GetService("ControlsService");
	Assert(ctrlsrv);
	ctrl = ctrlsrv->CreateInstance(_FL_);
	Assert(ctrl);
		
	pCursor = NEW GUICursor ("cursor");


	IRender* rs = (IRender*)api->GetService("DX9Render");
	pTextureVar = rs->GetTechniqueGlobalVariable("GUITex", _FL_);
	rs->EnableLoadingScreen(false);

/*
	IVariable* pCliperMatrix = rs->GetTechniqueGlobalVariable("GUI_CliperMatrix", _FL_);

*/
	
	
	return true;
}

GUIManager::~GUIManager ()
{
	pTextureVar = NULL;
	

	FlushCache ();

	GlobalDelete = true;
	delete pCursor;
	
	for (unsigned long n = 0; n < RegistredWindows.Size(); n++)
	{
		GUIControl** wnd = &RegistredWindows [n];
		GUIControl* pWnd = *(wnd);
		
		//api->Trace("GUI MANAGER: Destroy window - '%s'", pWnd->Caption);
		DestroyGUIWindow ((GUIWindow *)pWnd);
	}
	
	CommitDestroy ();
	RELEASE(ctrl);

}

GUIWindow* GUIManager::CreateGUIWindow (int Left, int Top, int Width, int Height)
{
	GUIWindow* nPtr = NEW GUIWindow (NULL, Left,  Top,  Width,  Height);
	
	nPtr->Application = this;
	
	
	GUIControl** newEntry = &RegistredWindows[RegistredWindows.Add ()];
	*(newEntry) = nPtr;
	
	
	click_state++;
	nPtr->draw_level = click_state;
	nPtr->IsActive = true;
	
	if (active_window) 
	{
		((GUIWindow *)active_window)->IsActive = false;
		keyboard_focus = NULL;
	}
	
	active_window = nPtr;
	
	
	return nPtr;
}

void GUIManager::DestroyGUIWindow (GUIWindow* pWnd)
{
	keyboard_focus = NULL;
	if (!GlobalDelete)	pWnd->t_OnClose->Execute (pWnd);
	pHint.Enabled = false;
	GUIControl** newEntry = &WindowsToDelete[WindowsToDelete.Add ()];
	*(newEntry) = pWnd;
}


void GUIManager::Process ()
{
//	float CursorDist = sqrt (pCursor->DeltaX * pCursor->DeltaX +  pCursor->DeltaY * pCursor->DeltaY);




	const GUIPoint& pt = pCursor->GetPosition ();
	new_element_under_mouse = GetElementUnderCursor (pt);
	
	if (element_under_mouse != new_element_under_mouse)
	{
		DestroyHint();
		HintTime = 0.0f;		
		if (element_under_mouse)     
		element_under_mouse->SendMessage (GUIMSG_MOUSELEAVE);
		
		
		if (new_element_under_mouse) 
		new_element_under_mouse->SendMessage (GUIMSG_MOUSEENTER);
		
		element_under_mouse = new_element_under_mouse;
	} else
		{
			HintTime += api->GetDeltaTime();
		}
	


	if ((HintTime > 1.0f) && (element_under_mouse != NULL))
	{
		bool res = element_under_mouse->BeforeHintShow();
		if (res == true)
		{
			if (HintTime < 4.0f)
			{
				BornHint (element_under_mouse->Hint);
				pHint.HintElement = element_under_mouse;
			}
		} else
			{
				DestroyHint();
				HintTime = 0.0f;		
			}

		//(Max comment) if (HintTime > 4.0f) DestroyHint();
	}

	
/*
	GUIMSG_LMB_DBLCLICK,
	GUIMSG_RMB_DBLCLICK,
	GUIMSG_WHEEL_UP,
	GUIMSG_WHEEL_DOWN,
*/


	if (!ctrl)
	{
		pCursor->Update ();
		CommitDestroy ();

		return;
	}



		int mwheel  = (int)(ctrl->GetControlStateFloat("MWheel") * 20.0f);

		if (mwheel != 0)
		{
			int dir = mwheel / abs(mwheel);
			if (dir < 0)
				for (int n =0; n < abs(mwheel); n++)
									SendMessage (GUIMSG_WHEEL_DOWN);

			if (dir > 0)
				for (int n =0; n < abs(mwheel); n++)
									SendMessage (GUIMSG_WHEEL_UP);

		}
		
		

		//CST_INACTIVATED
		if (ctrl->GetControlStateType("LeftMouseButton") == CST_ACTIVATED)
		{
			MP_CursorPos = pt;
			SendMessage (GUIMSG_LMB_DOWN);
			tFirstMousePressed = element_under_mouse;
		}

		if (ctrl->GetControlStateType("LeftMouseButton") == CST_INACTIVATED)
		{
			tFirstMousePressed = NULL;
			SendMessage (GUIMSG_LMB_UP);
			// Завершение драга...
			if (bDragIsBegin == true)
			{
				bDragIsBegin = false;
				pCursor->Pop ();
				
				if (element_under_mouse)
				{
					bool CanDrop = element_under_mouse->DragOver (pt.X, pt.Y, dragfrom);
					if (CanDrop)
					{
						element_under_mouse->DragDrop (pt.X, pt.Y, dragfrom);
					} else
					{
						element_under_mouse->DragDrop (pt.X, pt.Y, NULL);
					}
				}

				dragfrom = NULL;
			}

		}

		if (ctrl->GetControlStateType("LeftMouseDbl") == CST_ACTIVATED)
		{
			SendMessage (GUIMSG_LMB_DBLCLICK);
		}
		
		if (ctrl->GetControlStateType("RightMouseDbl") == CST_ACTIVATED)
		{
			SendMessage (GUIMSG_RMB_DBLCLICK);
		}

		if (ctrl->GetControlStateType("RightMouseButton") == CST_ACTIVATED)
		{
			SendMessage (GUIMSG_RMB_DOWN);
		}


		if (ctrl->GetControlStateType("RightMouseButton") == CST_INACTIVATED)
		{
			SendMessage (GUIMSG_RMB_UP);
		}


		if (ctrl->GetControlStateType("MiddleMouseButton") == CST_ACTIVATED)
		{
			SendMessage (GUIMSG_MMB_DOWN);
		}


		if (ctrl->GetControlStateType("MiddleMouseButton") == CST_INACTIVATED)
		{
			SendMessage (GUIMSG_MMB_UP);
		}


	
	
	
	if ((old_mouse_pos.X !=  pt.X) || (old_mouse_pos.Y !=  pt.Y))
	{
		SendMessage (GUIMSG_MOUSEMOVE);
		old_mouse_pos = pt;

		//MP_CursorPos = pt;
		float mDX = (float)(MP_CursorPos.x - pt.x);
		float mDY = (float)(MP_CursorPos.y - pt.y);
		float CursorDist = sqrtf (mDX * mDX +  mDY * mDY);
			// Если двигаем мышу и зажата левая кнопка мыши...
		if (ctrl->GetControlStateType("LeftMouseButton") == CST_ACTIVE)
		{
				// Процесс драга
				if (bDragIsBegin == true)
				{
					if (element_under_mouse)
					{
						bool CanDrop = element_under_mouse->DragOver (pt.X, pt.Y, dragfrom);
						if (CanDrop == false)
						{
							pCursor->SetCursor ("ndragcursor", -8, -8);
						} else
							{
								pCursor->SetCursor ("cdragcursor", -8, 0);
							}
					} else
						{
							pCursor->SetCursor ("ndragcursor", -8, -8);
						}
				} // Drag is begin
			

			if (CursorDist >= 5)
			{
				if ((bDragIsBegin == false) && (element_under_mouse) && 
						(element_under_mouse->bDragAndDrop == true) && 
						(tFirstMousePressed == element_under_mouse))
				{
					if (element_under_mouse->DragBegin ())
					{
						// Начало драга
						dragfrom = element_under_mouse;

						bDragIsBegin = true;
						pCursor->Push ();
						pCursor->SetCursor ("ndragcursor", -8, -8);
					}
				}
			} // Cursor Dist
			
		} // LMB - pressed
	}
	
	long KeyBufLen = ctrl->GetKeyBufferLength();
	const KeyDescr* KeyBuf = ctrl->GetKeyBuffer();
	for (long k = 0; k < KeyBufLen; k++)
	{
		if (KeyBuf[k].bSystem)
		{
			SendMessage (GUIMSG_KEYPRESSED, (dword)KeyBuf[k].ucVKey, 1);
		} else
			{
				SendMessage (GUIMSG_KEYPRESSED, (dword)KeyBuf[k].ucVKey, 0);
			}
	}
/*	
	int cur_key = GetCurrentKey ();
	
	if (cur_key >= 0)
	{
		DWORD lparam = (DWORD)cur_key;
		SendMessage (GUIMSG_KEYPRESSED, lparam);
	}
*/
	
	pCursor->Update ();
	
	CommitDestroy ();
}  


bool _cdecl SortFunc (GUIControl *const &wnd1, GUIControl *const &wnd2)
{
//	GUIControl* pWindow1 = *(wnd1);
//	GUIControl* pWindow2 = *(wnd2);
	
	if (wnd1->GetDrawLevel() > wnd2->GetDrawLevel()) return false;
	
	return true;
}

bool _cdecl SortFunc2 (GUIControl *const &wnd1, GUIControl *const &wnd2)
{
	
	if (wnd1->GetDrawLevel() < wnd2->GetDrawLevel()) return false;
	
	return true;
}


void GUIManager::Draw ()
{
	cliper.SetFullScreenRect ();
	IRender* rs = (IRender*)api->GetService("DX9Render");
	RENDERVIEWPORT tmp = rs->GetViewport ();

	Matrix ident;
	rs->SetWorld (ident);
	
	RegistredWindows.QSort (SortFunc);
	
	for (unsigned long n = 0; n < RegistredWindows.Size(); n++)
	{
		GUIControl** wnd = &RegistredWindows [n];
		GUIControl* pWnd = *(wnd);
		if (pWnd->Visible)
		{
			pWnd->Draw ();
		}
		
	}
	
	
	Matrix m;

	IVariable* pVar = rs->GetTechniqueGlobalVariable("GUI_CliperMatrix", _FL_);
	if (pVar)
	{
		pVar->SetMatrix(m);
		pVar = NULL;
	}

	/*
	m.Transposition ();
	DWORD ffc = rs->GetFirstFreeVertexShaderConstant ();
	rs->SetVertexShaderConstant (ffc, m, 4);
*/

	
	rs->SetViewport (tmp);
	RenderHint ();
	if (bShowCursor)
	{
		bool NeedSTDDraw = true;
		if (bDragIsBegin)
		{
			// Надо вызвать евент ручной отрисовки...
			if ((cd_object != NULL) && (cd_custom_draw != NULL))
			{
				GUIPoint mouse_point = pCursor->GetPosition ();
				NeedSTDDraw = (cd_object->*cd_custom_draw) (mouse_point.x, mouse_point.y);
			}
		}
		if (NeedSTDDraw) pCursor->Draw ();
	}
		

	
}




void GUIManager::ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam)
{
	const GUIPoint& pt = pCursor->GetPosition ();
	
	int under_cursor_only = GUIHelper::MessageSpreadOnlyForUnderCursorControls(message);
	
	if (under_cursor_only)
	{
		lparam = pt.X;
		hparam = pt.Y;
	}
	

	for (DWORD u = 0; u < RegistredWindows.Size(); u++)
	{
		GUIControl** wnd = &RegistredWindows [u];
		GUIControl* pWnd = *(wnd);

		if (message == GUIMSG_LMB_DOWN) pWnd->MouseDown (message, pt);
		if (message == GUIMSG_RMB_DOWN) pWnd->MouseDown (message, pt);
		if (message == GUIMSG_MMB_DOWN) pWnd->MouseDown (message, pt);
		
		if (message == GUIMSG_MOUSEMOVE) pWnd->MouseMove (message, pt);
		
		if (message == GUIMSG_LMB_UP) pWnd->MouseUp (message, pt);
		if (message == GUIMSG_RMB_UP) pWnd->MouseUp (message, pt);
		if (message == GUIMSG_MMB_UP) pWnd->MouseUp (message, pt);
		if (message == GUIMSG_KEYPRESSED) pWnd->KeyPressed ((int)lparam, hparam==1);

	}

	
	GUIControl* pWnd = NULL;
	int need_send = true;
	
	// Выбираем какое окошко сделать активным...
	if (active_combobox == NULL)
	{
		if ((message == GUIMSG_LMB_DOWN) || (message == GUIMSG_RMB_DOWN))
		{
			RegistredWindows.QSort (SortFunc2);
			int current_active_clicked = false;

			GUIControl** wnd = &RegistredWindows [0];
			if (!wnd) return;
			GUIControl* pWnd = *(wnd);
			GUIRectangle rct = pWnd->GetDrawRect ();
			pWnd->ClientToScreen(rct);
			if (GUIHelper::PointInRect(pt, rct)) 
			{
				if (pWnd == active_window) current_active_clicked = true;
			}

			if (!current_active_clicked)
			{
				for (unsigned long x = 0; x < RegistredWindows.Size(); x++)
				{
					GUIControl** wnd = &RegistredWindows [x];
					GUIControl* pWnd = *(wnd);

					// Если в модальном режим и выбрали окошко
					// созданное до модального
					if (modal_mode)
					{
						// то все пропустить, такое окно нельзя выбрать...
						int WndAllocID = pWnd->GetAllocID ();
						if (WndAllocID < ModalModeID)
						{
							continue;
						}
					}

					GUIRectangle rct = pWnd->GetDrawRect ();
					pWnd->ClientToScreen(rct);
					if (GUIHelper::PointInRect(pt, rct)) 
					{
						// Делаем окошко активным...
						click_state++;
						pWnd->draw_level = click_state;
						
						((GUIWindow *)pWnd)->IsActive = true;
						if (active_window) 
						{
							((GUIWindow *)active_window)->IsActive = false;
							if (keyboard_focus) keyboard_focus->SendMessage (GUIMSG_LOSTKEYBOARDFOCUS);
							keyboard_focus = NULL;
						}
						active_window = pWnd;
						break;
					}
				}
			}
		}
		// Выбрали...
		
	//} //active_combobox == NULL
	
	if (active_window == NULL) return;
	// Отсылаем ему массадж...
	//RegistredWindows.Sort (SortFunc);
	//unsigned long n = 0;
	//for (unsigned long n = 0;n < RegistredWindows.Size(); n++)
	//{
		//GUIControl** wnd = RegistredWindows (n);
		//GUIControl* pWnd = *(wnd);
		pWnd = active_window;
		need_send = true;
		if (under_cursor_only)
		{
			GUIRectangle rct = pWnd->GetDrawRect ();
			pWnd->ClientToScreen(rct);
			if (GUIHelper::PointInRect(pt, rct) == false) need_send = false;
		}


		
	} else //active_combobox == NULL
		{
			pWnd = active_combobox;
			need_send = true;


			if (message == GUIMSG_LMB_DOWN)
			{
				GUIRectangle rct = active_combobox->GetClientRect ();
				active_combobox->ClientToScreen (rct);
				if (!GUIHelper::PointInRect(pt, rct)) 
				{
					((GUIComboBox*)active_combobox)->ListBox->Visible = false;
					need_send = false;
					SetActiveComboBox (NULL);
				}
			}
		

		}

		
		if (!pWnd->Visible) need_send = false;
		if (need_send)
		{
			if (message != GUIMSG_KEYPRESSED)
			{
				pWnd->SendMessage (message, lparam, hparam);
			}	else
				{
					if (keyboard_focus) 
							keyboard_focus->SendMessage (message, lparam, hparam);
				}

			
		}


		
		
	// }
	
}

void GUIManager::SendMessage (GUIMessage message, DWORD lparam, DWORD hparam)
{
	ProcessMessages (message, lparam, hparam);
}



GUIControl* GUIManager::GetElementUnderCursor (const GUIPoint& pt)
{
	
	unsigned long t = RegistredWindows.Size();
	for (unsigned long n = 0; n < t; n++)
	{
		GUIControl** wnd = &RegistredWindows [t-n-1];
		GUIControl* pWnd = *(wnd);
		
		GUIControl* element = pWnd->GetElementUnderCursor (pt);
		if (element && element->Visible) return element;
	}
	
	
	return NULL;
}


void GUIManager::CommitDestroy ()
{
	FocusWasLost = false;
	for (unsigned long n = 0; n < WindowsToDelete.Size(); n++)
	{
		GUIControl** wnd = &WindowsToDelete [n];
		GUIControl* pWnd = *(wnd);
		
		// Если в модальном режиме...
		if (modal_mode)
		{

			int mdlwind_index = 0;
			for (DWORD j = 0; j < ModalWindows.Size(); j++)				
			{
				if (ModalWindows[j] == pWnd)
				{
					mdlwind_index = j;
					break;
				}

			}

			
			if (mdlwind_index >= 0)
			{
				GUIControl** tmpwnd = &ModalWindows [mdlwind_index];
				GUIControl* mdlWnd = *(tmpwnd);
				// Окно которе мы удаляем было модальное...
				if (mdlWnd == pWnd)
				{
					//Удаляем окно из списка модальных окон
					ModalWindows.DelIndex (mdlwind_index);
					// Если модальных окон не осталось
					// откулюаем модальный режим....
					if (ModalWindows.Size() <= 0)
					{
						FocusWasLost = true;
						modal_mode = false;
						active_window = NULL;
						ModalModeID = 0;
					} else
					{
						// Делаем самое последнее оконо в списке модальных активным
						int mcount = ModalWindows.Size ();
						GUIControl** tmpwnd2 = &ModalWindows [mcount-1];
						GUIControl* mdlWnd2 = *(tmpwnd2);
						ModalModeID = mdlWnd2->GetAllocID ();

						// Делаем окошко активным...
						click_state++;
						pWnd->draw_level = click_state;
						
						((GUIWindow *)mdlWnd2)->IsActive = true;
						if (active_window) 
						{
							((GUIWindow *)active_window)->IsActive = false;
							if (keyboard_focus) keyboard_focus->SendMessage (GUIMSG_LOSTKEYBOARDFOCUS);
							keyboard_focus = NULL;
						}
						active_window = mdlWnd2;
						active_window->IsActive = true;

					}
				}
			}
		}

		if (message_box_window == pWnd)
		{
			message_box_window = NULL;
		}
		
		
		if (pWnd == active_window)  
		{
			FocusWasLost = true;
			active_window = NULL;
			keyboard_focus = NULL;
		}
		
		if (element_under_mouse)
		{
			GUIControl* gc = element_under_mouse->GetMainControl ();
			if (gc == pWnd) element_under_mouse = NULL;
		}
		
		int num = FindWindowByPtr (pWnd);
		if (num != -1) 
		{
			GUIControl** w1 = &RegistredWindows [num];
      GUIControl* pWindowToDelete = *(w1);
     
			if (GlobalDelete)
			{
				GUIWindow* delWnd = (GUIWindow*)pWindowToDelete; 
				delWnd->t_OnClose->SetHandler (NULL, (CONTROL_EVENT)NULL);
			}
      
      //t_OnClose
			delete pWindowToDelete;
			RegistredWindows.DelIndex (num);
		}
	}
	
	WindowsToDelete.DelAll ();



	
	//FocusWasLost = false;
	if (FocusWasLost)
	{
		DWORD max_num = 0;
		int index = -1;
		for (DWORD y =0; y < RegistredWindows.Size(); y++)
		{
			if (RegistredWindows[y]->GetDrawLevel() >= max_num)
			{
				index = y;
				max_num = RegistredWindows[y]->GetDrawLevel();
			}
		}

		if (index >= 0)
		{
			RegistredWindows[index]->SetFocus();
		}
	}
}


int GUIManager::FindWindowByPtr (GUIControl* pWnd)
{
	for (unsigned long n = 0; n < RegistredWindows.Size(); n++)
	{
		GUIControl** wnd = &RegistredWindows [n];
		GUIControl* xpWnd = *(wnd);
		
		if (xpWnd == pWnd) return n;
	}
	
	return -1; // Unregistred window
}


int GUIManager::RegisterForm (GUIWindow* form)
{
	form->Application = this;
	
	
	GUIControl** newEntry = &RegistredWindows[RegistredWindows.Add ()];
	*(newEntry) = form;
	
	
	click_state++;
	form->draw_level = click_state;
	form->IsActive = true;
	
	if (active_window) 
	{
		((GUIWindow *)active_window)->IsActive = false;
		if(active_window->InTree(keyboard_focus))
		{
			keyboard_focus = NULL;
		}
	}
	
	active_window = form;
	
	return true;
}


//GetAsyncKeyState
int GUIManager::GetCurrentKey ()
{
	for (int n = 0; n < 255; n++)
	{
		if (n == VK_SHIFT) continue;
		if (n == 161) continue;  // RSHIFT
		if (n == 160) continue; // LSHIFT
		if (n == 17) continue; // RCTRL
		if (n == 162) continue; // LCTRL
		if (n == 1) continue; // LMB
		if (n == 2) continue; // RMB
		if (n == 3) continue; // На всякий случай :)
		if (GetAsyncKeyState(n) < 0) return n;
	}
	
	
	return -1;
}


void GUIManager::SetKeyboardFocus (GUIControl* kfocus)
{
	if (keyboard_focus == kfocus) return;
	
	if (keyboard_focus) keyboard_focus->SendMessage (GUIMSG_LOSTKEYBOARDFOCUS);
	keyboard_focus = kfocus;
}


void GUIManager::Show (GUIWindow* form)
{
	if (keyboard_focus) keyboard_focus->SendMessage (GUIMSG_LOSTKEYBOARDFOCUS);
	keyboard_focus = NULL;

// Если в модальном режиме, то не заводить новые окна...
	form->SetAllocID (CurrentAllocID);
	CurrentAllocID++;
	RegisterForm (form);
	ExecuteCreate (form);


//	form->t_OnCreate->Execute (NULL);
}

void GUIManager::ShowModal (GUIWindow* form)
{
	if(!form->InTree(keyboard_focus))
	{
		if (keyboard_focus) keyboard_focus->SendMessage (GUIMSG_LOSTKEYBOARDFOCUS);
		keyboard_focus = NULL;
	}


	GUIControl** newEntry = &ModalWindows[ModalWindows.Add ()];
	*(newEntry) = form;

	ModalModeID = CurrentAllocID;
	modal_mode = true;
	form->SetAllocID (CurrentAllocID);
	CurrentAllocID++;
	RegisterForm (form);
	modal_result = 0;
	ExecuteCreate (form);


//	form->t_OnCreate->Execute (NULL);
}

GUIControl* GUIManager::GetTopModal ()
{
	if(ModalWindows.Size() > 0)
	{
		return ModalWindows[ModalWindows.Last()];
	}
	return 0;
}


int GUIManager::InitForm (GUIWindow* form)
{
	form->Application = this;
	
	return true;
}

void GUIManager::ArrangeCascade ()
{
	int px = 0;
	int py = 0;
	
	
	for (unsigned long n = 0; n < RegistredWindows.Size(); n++)
	{
		GUIControl** wnd = &RegistredWindows [n];
		GUIControl* xpWnd = *(wnd);
		
		GUIWindow* xW = (GUIWindow*)xpWnd;
		
		xW->SetPosition (px, py);
		px+= 12;
		py+= 12;
		
		
	}
	
	
}


GUIMessageBox* GUIManager::MessageBox (const char* Text, const char* Caption, DWORD flags, bool bFlat)
{
	
	
	GUIMessageBox* mb = NEW GUIMessageBox (10, 10, Caption, Text, flags, bFlat);
	
	int the_width = mb->GetDrawRect().Width;
	
	IRender* rs = (IRender*)api->GetService("DX9Render");
	
	int dX = (rs->GetScreenInfo3D().dwWidth - the_width) / 2;
	
	int the_height = mb->GetDrawRect().Height;
	
	int dY = (rs->GetScreenInfo3D().dwHeight - the_height) / 2;
	
	mb->SetPosition (dX, dY);
	
	
	message_box_window = mb;
	ShowModal (mb);
	
	/*
	for (;;)
	{
		this->Draw ();
		this->Process ();
		
		if (message_box_window == NULL) break;
	}
	*/
	
	
	
	return mb;
}


void _fastcall GUIManager::DrawGUI(float dltTime)
{
	Process ();

	for (int n =0; n < DefferedEvents; n++)
	{
		DefferedEvents[n].frame_to_execute--;
		if (DefferedEvents[n].frame_to_execute <= 0)
		{
			DefferedEvents[n].event.Execute(NULL);
			DefferedEvents.ExtractNoShift(n);
			n--;
		}
	}

	Draw ();
}

void GUIManager::Close (GUIWindow* pWnd)
{
	keyboard_focus = NULL;
	DestroyGUIWindow (pWnd);
}

GUIWindow* GUIManager::FindWindow (const char* caption)
{
	for (unsigned long n = 0; n < RegistredWindows.Size(); n++)
	{
		GUIControl** wnd = &RegistredWindows [n];
		GUIControl* xpWnd = *(wnd);
		
		GUIWindow* xW = (GUIWindow*)xpWnd;

		// Нашли такое окошко
		if (strcmp (xW->Caption.GetBuffer(), caption) == 0) 
		{
			return xW;
		}
		
	}

	return NULL;
}



void GUIManager::GetCursor (int& x, int &y)
{
	const GUIPoint& pt = pCursor->GetPosition ();
	x = pt.X;
	y = pt.Y;
}


void GUIManager::HideAll ()
{
	for (unsigned long n = 0; n < RegistredWindows.Size(); n++)
	{
		GUIControl** wnd = &RegistredWindows [n];
		GUIControl* xpWnd = *(wnd);
		
		GUIWindow* xW = (GUIWindow*)xpWnd;
		xW->Visible = false;
	}

}

void GUIManager::ShowAll ()
{
	for (unsigned long n = 0; n < RegistredWindows.Size(); n++)
	{
		GUIControl** wnd = &RegistredWindows [n];
		GUIControl* xpWnd = *(wnd);
		
		GUIWindow* xW = (GUIWindow*)xpWnd;
		xW->Visible = true;
	}

}


void GUIManager::ShowCursor (bool bShow)
{
	bShowCursor = bShow;
}

void GUIManager::SetActiveComboBox (GUIControl* combobox)
{
	active_combobox = combobox;
}

GUIControl* GUIManager::GetActiveComboBox ()
{
	return active_combobox;
}

void GUIManager::GetDeltaMouse (float& dX, float& dY)
{
	dX = pCursor->DeltaX;
	dY = pCursor->DeltaY;
}

GUIControl* GUIManager::GetElementUnderCursor ()
{
	return element_under_mouse;
}
//

//Устанавливает фокус для всего...
void GUIManager::SetFocus (GUIControl* element)
{
	//GUIPoint old_cursor_pos = pCursor->GetPosition ();

	GUIRectangle el_rect = element->GetClientRect ();
	element->ClientToScreen (el_rect);
	GUIPoint new_pos;
	new_pos.x = el_rect.x+4;
	new_pos.y = el_rect.y+4;
	//pCursor->SetPosition (new_pos);
	//const GUIPoint& pt = pCursor->GetPosition ();
	DWORD lparam = new_pos.X;
	DWORD hparam = new_pos.Y;
	element->SendMessage(GUIMSG_LMB_DOWN, lparam, hparam);

	if (element->Is("GUIWindow")) active_window = element;
	element->IsActive = true;

	//pCursor->SetPosition (old_cursor_pos);


}

const char* GUIManager::GetCurrentDir ()
{
	return sysCurrentDirectory;
}

void GUIManager::SetCurrentDir (const char* cDir)
{
	crt_strncpy (sysCurrentDirectory, MAX_PATH-1, cDir, MAX_PATH-2);
}


void GUIManager::ExecuteCreate (GUIControl* window)
{
 window->sysOnCreate();
 window->OnCreate ();


 for (DWORD n = 0; n < window->Childs.Size (); n++)
 {
	 ExecuteCreate (window->Childs[n]);
 }
}

GUICursor* GUIManager::GetCursor ()
{
	return pCursor;
}


void GUIManager::SetDragAndDropCursorCustomDraw (GUIControl* object, CUSTOM_CURSORDRAW custom_draw)
{
	cd_object = object;
	cd_custom_draw = custom_draw;

}

void GUIManager::BornHint (const string& str)
{
	if (pHint.Enabled == false)
	{
		int addY = pCursor->GetImage()->GetHeight()+pCursor->GetOffsetY();
		const GUIPoint& pt = pCursor->GetPosition ();
		pHint.posX = pt.x + pCursor->GetOffsetX();
		pHint.posY = pt.y + addY - 8;
	}
	
	if (str.IsEmpty())
	{
		pHint.Enabled = false;
		return;
	}

	pHint.Enabled = true;
	pHint.HintText = str;

}

void GUIManager::DestroyHint ()
{
	pHint.Enabled = false;
}

void GUIManager::RenderHint ()
{
	if (!pHint.Enabled) return;

	pHint.HintElement->DrawHint (pHint.posX, pHint.posY, pHint.HintText);
}

void GUIManager::AddDefferedEvent (const IGUIManager::DefferedEvent& event)
{
	DefferedEvents.Add(event);
}


void GUIManager::PrecacheImages (const char* dir)
{
	IRender* pRS = (IRender*)api->GetService("DX9Render");
	string filename, basePath;
	filename = "resource\\textures\\gui\\";
	filename += dir;
	IFileService* fs = (IFileService*)api->GetService("FileService");
	fs->BuildPath("resource\\textures\\", basePath);
	IFinder* finder = fs->CreateFinder(filename, "*.txx", find_all_files | find_no_recursive, _FL_);
	for (dword i = 0; i < finder->Count(); i++)
	{		
		filename = finder->FilePath(i);
		filename.GetRelativePath(basePath);
		IBaseTexture* nTexture = pRS->CreateTexture(_FL_, filename);
		CachedImages.Add(nTexture);
	}
	finder->Release();	
}

void GUIManager::FlushCache ()
{
	for (int n = 0; n < CachedImages; n++)
	{
		if (CachedImages[n])	CachedImages[n]->Release();
	}
	CachedImages.DelAll();
}

void GUIManager::DeleteControl (GUIControl* ctrl)
{
	if (ctrl == element_under_mouse) element_under_mouse = NULL;
	if (ctrl == keyboard_focus) keyboard_focus = NULL;
	if(pHint.HintElement == ctrl)
	{
		DestroyHint();
		pHint.HintElement = null;
	}
}

void GUIManager::Enable (bool bVal)
{
	if (bVal)
	{
		api->SetObjectExecution(this, "gui", 0x100, &GUIManager::DrawGUI);
	} else
	{
		api->DelObjectExecutions(this);
	}
	
}

int GUIManager::GetNumRegWindows()
{
	return RegistredWindows.Size();
}

GUIWindow* GUIManager::GetRegWindow(int i)
{
	if(i >= 0 && i < RegistredWindows)
	{
		return (GUIWindow*)RegistredWindows[i];
	}
	return 0;
}

#endif