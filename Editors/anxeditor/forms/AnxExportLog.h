//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnxExportLog
//============================================================================================

#ifndef _AnxExportLog_h_
#define _AnxExportLog_h_

#include "..\AnxBase.h"
#include "..\Graph\GraphNodeInOut.h"
#include "..\EditorControls\AnxToolButton.h"
#include "..\EditorControls\AnxToolListBox.h"
#include "..\Commands\AnxCmd_NodeState.h"

class AnxExportLog : public GUIWindow  
{
//--------------------------------------------------------------------------------------------
public:
	AnxExportLog(AnxOptions & options, array<string> & errs);
	virtual ~AnxExportLog();
	virtual void OnCreate();

//--------------------------------------------------------------------------------------------
public:
	//Рисование
	virtual void Draw();

	void _cdecl EvtClose(GUIControl * sender);

//--------------------------------------------------------------------------------------------
private:
	AnxOptions & opt;
	AnxToolListBox::LB * log;
	float size;
	float dlt;
};

#endif

