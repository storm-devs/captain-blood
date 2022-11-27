#ifndef _XBOX


#include "gui_events.h"
#include "gui_control.h"




GUIEventHandler::GUIEventHandler ()
{
	stored_event = NULL;
	stored_object = NULL;
}

Object* GUIEventHandler::GetClass ()
{
	return stored_object;
}

GUIEventHandler::~GUIEventHandler ()
{
}

void GUIEventHandler::SetEvent (CONTROL_EVENT evt)
{
	stored_event = evt;
	
	
}

bool GUIEventHandler::Execute (GUIControl* sender)
{
	if (stored_object == NULL) return false;
	if (stored_event == NULL) return false;
	
	(stored_object->*stored_event) (sender);

	return true;
}



void GUIEventHandler::SetObject (Object* object)
{
	stored_object = object;
}





void GUIEventHandler::operator = (CONTROL_EVENT evt)
{
	SetEvent (evt);
}


GUIEventHandler* GUIEventHandler::GetThis (GUIEventHandler* &ptr) 
{
	ptr = this; 
	return this;
};


void GUIEventHandler::SetHandler (Object* object, CONTROL_EVENT event)
{
	stored_object = object;
	stored_event = event;
}


#endif