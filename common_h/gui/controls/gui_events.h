#ifndef _XBOX

#ifndef GUI_EVENTS
#define GUI_EVENTS


//#include "gui_control.h"


class GUIControl;

typedef void (_cdecl Object::*CONTROL_EVENT)(GUIControl* sender);




class GUIEventHandler
{
protected:
	
	CONTROL_EVENT stored_event;
	Object* stored_object;
	
public:
	
	GUIEventHandler ();
	~GUIEventHandler ();
	
	
	Object* GetClass ();

	void SetEvent (CONTROL_EVENT evt);
	
	void SetObject (Object* object);
	
	bool Execute (GUIControl* sender);
	
	GUIEventHandler* GetThis (GUIEventHandler* &ptr);
	
	void operator = (CONTROL_EVENT evt);
	
	void SetHandler (Object* object, CONTROL_EVENT event);

	template<class T> inline void SetHandlerEx(GUIControl * object, T event)
	{
		SetHandler (object, (CONTROL_EVENT)event);
	};
	
};



#endif

#endif