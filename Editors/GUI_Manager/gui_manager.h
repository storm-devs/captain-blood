#ifndef _XBOX


#ifndef GUI_WINDOWS_MANAGER
#define GUI_WINDOWS_MANAGER

//#include "..\..\common_h\gui\gui_IManager.h"
#include "..\..\common_h\gui.h"
#include "..\..\common_h\gui\Controls\gui_window.h"
#include "..\..\common_h\gui\Controls\gui_cursor.h"
#include "..\..\common_h\templates.h"




class GUIManager : public IGUIManager
{
	IVariable* pTextureVar;

	bool bSysKey;
	array <IBaseTexture*> CachedImages;
	bool FocusWasLost;
	int CurrentAllocID;
	int ModalModeID;

	GUIPoint MP_CursorPos;
	friend class GUIMessageBox;

	IControls * ctrl;
	


	GUIControl* active_combobox;

	int GlobalDelete;
	bool bShowCursor;
	
	int modal_result;
	bool modal_mode;
	
	
	int click_state;
	GUIControl* active_window;
	
	GUIControl* message_box_window;
	
	
	GUIControl* keyboard_focus;
	
	
	
	GUIControl* element_under_mouse;
	GUIControl* new_element_under_mouse;
	
	GUIPoint old_mouse_pos;
	
	GUICursor* pCursor;
	
  array <GUIControl*> ModalWindows;

	array <GUIControl*> RegistredWindows;
	
	array <GUIControl*> WindowsToDelete;
	
	void ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);

	
	GUIControl* tFirstMousePressed;
	GUIControl* dragfrom;
	bool bDragIsBegin;

GUIControl* cd_object;
CUSTOM_CURSORDRAW cd_custom_draw;


float HintTime;

 struct GUIHint
 {
	 GUIControl* HintElement;
	 string HintText;
	 int posX;
	 int posY;
	 bool Enabled;
 };

 GUIHint pHint;
	
public:
	


	array<IGUIManager::DefferedEvent> DefferedEvents;

	void AddDefferedEvent (const IGUIManager::DefferedEvent& event);

	GUIManager ();
	~GUIManager ();
	
	
	GUIWindow* CreateGUIWindow (int Left, int Top, int Width, int Height);
	void DestroyGUIWindow (GUIWindow* pWnd);
	
	
	void Process ();  
	
	void Draw ();  
	
	
	
	void SendMessage (GUIMessage message, DWORD lparam = 0, DWORD hparam  = 0);
	
	
	GUIControl* GetElementUnderCursor (const GUIPoint& pt);
	
	void CommitDestroy ();
	
	int FindWindowByPtr (GUIControl* pWnd);
	
	
	int RegisterForm (GUIWindow* form);
	
	//GetAsyncKeyState
	int GetCurrentKey ();
	
	
	void SetKeyboardFocus (GUIControl* kfocus);
	
	
	void Show (GUIWindow* form);
	
	void ShowModal (GUIWindow* form);

	GUIControl* GetTopModal ();
	
	int InitForm (GUIWindow* form);
	
	
	void ArrangeCascade ();
	
	GUIMessageBox* MessageBox (const char* Text, const char* Caption, DWORD flags, bool bFlat = false);
	
	
	void _fastcall DrawGUI(float dltTime);	
	
	bool Init ();

	virtual void Close (GUIWindow* pWnd);

	virtual GUIWindow* FindWindow (const char* caption);

	virtual void GetCursor (int& x, int &y);


	virtual void HideAll ();
	virtual void ShowAll ();

	virtual void ShowCursor (bool bShow);


	virtual void SetActiveComboBox (GUIControl* combobox);
	virtual GUIControl* GetActiveComboBox ();

	virtual void GetDeltaMouse (float& dX, float& dY);
	
  virtual GUIControl* GetElementUnderCursor ();
//element_under_mouse

	virtual void SetFocus (GUIControl* element);


	virtual const char* GetCurrentDir ();
	virtual void SetCurrentDir (const char* cDir);

	void ExecuteCreate (GUIControl* window);

	virtual GUICursor* GetCursor ();


	virtual void SetDragAndDropCursorCustomDraw (GUIControl* object, CUSTOM_CURSORDRAW custom_draw);


	void BornHint (const string& str);
	void DestroyHint ();

	void RenderHint ();

	virtual void PrecacheImages (const char* dir);
	virtual void FlushCache ();

	virtual void DeleteControl (GUIControl* ctrl);


	virtual void Enable (bool bVal);

	virtual int GetNumRegWindows();
	virtual GUIWindow* GetRegWindow(int i);

	
};


#endif


#endif