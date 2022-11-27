//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// SndEditor
//============================================================================================

#ifndef _SndEditor_h_
#define _SndEditor_h_

#ifndef NO_TOOLS

#include "SndBase.h"
#include "SndOptions.h"

class SndEditorMainWindow;

class SndEditor : public RegObject  
{
//--------------------------------------------------------------------------------------------
public:
	SndEditor();
	virtual ~SndEditor();

//--------------------------------------------------------------------------------------------
public:
	bool    Init();
	void    Realize(dword Delta_Time);




//--------------------------------------------------------------------------------------------
private:
	SndOptions options;
	HANDLE hMutex;
};


#endif
#endif

