#ifndef _XBOX
#ifndef GUI_BASE_CONTROL
#define GUI_BASE_CONTROL

#include "gui_font.h"
#include "gui_rectangle.h"
#include "gui_messages.h"
#include "gui_point.h"
#include "..\..\core.h"
#include "..\..\math3d.h"
#include "..\..\render.h"
#include "..\..\Templates\string.h"

#include "gui_events.h"

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


#include "gui_cliper.h"



enum GUIAlignment
{
	GUIAL_Left = 0,
	GUIAL_Right,
	GUIAL_Center,
	
	GUIAL_FORCE_DWORD = 0x7fffffff
	
};




class GUIEventHandler;


extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnBeforeHintShow \
t_OnBeforeHintShow->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)



// Базовый класс
class GUIControl : public Object
{
	
	friend class IGUIManager;
	friend class GUIEdit;

	int UniqueAllocID;
	
	
protected:
	
	
	int mouse_inside;
	
	
	
	
	GUICliper cliper;
	

	
	GUIRectangle DrawRect; // Где рисуеться элемент
	
	GUIRectangle ClientRect; // Где рисовать детей 
	
	GUIRectangle TmpScreenRect;
	
	
	GUIControl* parent;
	
	
	void AddChild (GUIControl* pChild);
	
	void RemoveChild (GUIControl* pChild);
	
	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
	

	
public:

	bool OnHintShow_RetValue;

	GUIEventHandler _eventBeforeHintShow;
	GUIEventHandler* t_OnBeforeHintShow;
	string Hint;
	int IsActive;

	bool bDragAndDrop;

	bool bAlwaysOnTop;
  void SetAllocID (int id);
	int GetAllocID ();

private:
	GUIFont* pHintFont;

public:
	string Caption;     // Заголовок

	bool Enabled;			// Вкл. / выкл.
	bool Visible;      // Видимо ?
 
	array <GUIControl*> Childs;
	
	IGUIManager* Application;
	int draw_level;
	
	GUIFont* pFont;    // Фонт
	
	DWORD GetDrawLevel () const;
	
	GUIControl (GUIControl* parent);
	virtual ~GUIControl ();
	
	//Элемент на дереве этого элемента (текущий или потомок)
	bool InTree(GUIControl * ctrl);

	virtual const GUIRectangle& GetDrawRect ();
	virtual void SetDrawRect (const GUIRectangle& rect);
	
	
	virtual const GUIRectangle& GetClientRect ();
	virtual void SetClientRect (const GUIRectangle& rect);
	
	
	
	
	GUIControl* GetMainControl ();
	
	
	virtual void Draw ();
	
	
	virtual const GUIRectangle& GetScreenRect ();
	
	
	virtual bool SendMessage (GUIMessage message, DWORD lparam  = 0, DWORD hparam  = 0);
	
	virtual void ScreenToClient (GUIPoint& p1);
	
	virtual void ClientToScreen (GUIPoint& p1);
	
	virtual void ScreenToClient (GUIRectangle& p1);
	
	virtual void ClientToScreen (GUIRectangle& p1);
	
	
	// |----- Events ------------------------------------------------
	
	// |----- Events ------------------------------------------------
	
	
	GUIControl* GetElementUnderCursor (const GUIPoint& pt);
	
	virtual bool DragOver (int MouseX, int MouseY, GUIControl* dragfrom)
	{
			return false;
	}

	virtual bool DragBegin ()	{	return true; };
	virtual void DragDrop (int MouseX, int MouseY, GUIControl* dragfrom)	{	};


	virtual void MouseUp (int button, const GUIPoint& pt);
	virtual void MouseDown (int button, const GUIPoint& pt);
	virtual void MouseMove (int button, const GUIPoint& pt);
	virtual void KeyPressed (int key, bool bSysKey);


	virtual IRender* GetRender ();

	virtual GUIControl* GetParent ();


	virtual void SetFocus ();
	
	
	virtual void OnCreate ();

	virtual void sysOnCreate ();

	virtual bool BeforeHintShow ();

	virtual void DrawHint (int pX, int pY, const string& Hint);

	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIControl") == 0) return true;
		return false;
	}

	static dword controlsCounter;
	static GUIFont * stdHintFont;
};



#endif

#endif