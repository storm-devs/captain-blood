#ifndef _XBOX
#ifndef MESSAGE_BOX
#define MESSAGE_BOX


#include "gui_window.h"
#include "gui_label.h"
#include "gui_edit.h"


extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnOK \
t_OnOK->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

// Для навешивания обработчика приравниванием :)
#define OnCancel \
t_OnCancel->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


#define GUIMB_OKCANCEL 0
#define GUIMB_OK       1
#define GUIMB_YESNO       2
#define GUIMB_ABORTIGNORE       3


class GUIMessageBox : public GUIWindow
{
	friend class IGUIManager;
	
	
	DWORD flags;
	GUIButton* OKButton;
	GUIButton* CancelButton;
	GUILabel* MessageLabel;
	
public:
	
	bool bHotKeys;
	GUIMessageBox (int X, int Y, const char* caption, const char* message, DWORD flags, bool bFlat = false);
	~GUIMessageBox ();
	
	void _cdecl OKClick (GUIControl* sender);
	
	void _cdecl CloseClick (GUIControl* sender);
	
	GUIEventHandler* t_OnOK;  
	
	GUIEventHandler* t_OnCancel;  

	virtual void Draw ();

	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	
	void KeyPressed( int key, bool bSysKey);
	
	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIMessageBox") == 0) return true;
		return GUIWindow::Is (className);
	}

};



#endif

#endif