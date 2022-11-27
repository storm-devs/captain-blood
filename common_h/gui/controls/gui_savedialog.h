#ifndef _XBOX
#ifndef FILESAVE_DIALOG
#define FILESAVE_DIALOG


#include "gui_opendialog.h"
//#include "..\..\gui.h"



class GUIFileSave : public GUIFileOpen
{

 public:

   GUIFileSave ();
  ~GUIFileSave ();

	virtual void OnCreate ();

	virtual void _cdecl btnOKPressed ();

	virtual void _cdecl ReplaceOK ();
	virtual void _cdecl ReplaceCancel ();

	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIFileSave") == 0) return true;
		return GUIFileOpen::Is (className);
	}

};

#endif

#endif