#ifndef _XBOX

#ifndef GUI_WINDOWS_MANAGER_INTERFACE
#define GUI_WINDOWS_MANAGER_INTERFACE

#include <stdio.h>
#include "Controls\gui_window.h"
#include "Controls\gui_messagebox.h"

class GUIControl;
class GUICursor;

typedef bool (_cdecl GUIControl::*CUSTOM_CURSORDRAW)(int MouseX, int MouseY);


class IGUIManager : public RegObject
{
public:
	
	struct DefferedEvent
	{
		GUIEventHandler event;
		int frame_to_execute;
	};

	virtual void AddDefferedEvent (const IGUIManager::DefferedEvent& event) = 0;

	IGUIManager () {};
	virtual ~IGUIManager () {};
	
	virtual GUIWindow* CreateGUIWindow (int Left, int Top, int Width, int Height) = 0;
	virtual void DestroyGUIWindow (GUIWindow* pWnd) = 0;
	
	virtual void Show (GUIWindow* form) = 0;
	virtual void ShowModal (GUIWindow* form) = 0;
	virtual GUIControl* GetTopModal () = 0;
	virtual void ArrangeCascade () = 0;
	virtual GUIMessageBox* MessageBox (const char* Text, const char* Caption, DWORD flags, bool bFlat = false) = 0;
	
	//------------ FOR SYSTEM USE ------------------------------------------------
	
	virtual void Process () = 0;
	virtual void Draw () = 0;
	virtual void SendMessage (GUIMessage message, DWORD lparam = 0, DWORD hparam  = 0) = 0;
	virtual GUIControl* GetElementUnderCursor (const GUIPoint& pt) = 0;
	virtual void CommitDestroy () = 0;
	virtual int FindWindowByPtr (GUIControl* pWnd) = 0;
	virtual int RegisterForm (GUIWindow* form) = 0;
	virtual int GetCurrentKey () = 0;
	virtual void SetKeyboardFocus (GUIControl* kfocus) = 0;
	virtual int InitForm (GUIWindow* form) = 0;

	virtual void Close (GUIWindow* pWnd) = 0;

	virtual GUIWindow* FindWindow (const char* caption) = 0;

	virtual void GetCursor (int& x, int &y) = 0;


	virtual void HideAll () = 0;
	virtual void ShowAll () = 0;

	virtual void ShowCursor (bool bShow) = 0;

	virtual void SetActiveComboBox (GUIControl* combobox) = 0;
	virtual GUIControl* GetActiveComboBox () = 0;


	virtual void GetDeltaMouse (float& dX, float& dY) = 0;


	virtual GUIControl* GetElementUnderCursor () = 0;

	virtual void SetFocus (GUIControl* element) = 0;


	virtual const char* GetCurrentDir () = 0;
	virtual void SetCurrentDir (const char* cDir) = 0;

	virtual GUICursor* GetCursor () = 0;

	virtual void SetDragAndDropCursorCustomDraw (GUIControl* object, CUSTOM_CURSORDRAW custom_draw) = 0;

	virtual void PrecacheImages (const char* dir) = 0;
	virtual void FlushCache () = 0;

	virtual void DeleteControl (GUIControl* ctrl) = 0;


	virtual void Enable (bool bVal) = 0;

	virtual int GetNumRegWindows() = 0;
	virtual GUIWindow* GetRegWindow(int i) = 0;

//element_under_mouse

};


#endif


#endif