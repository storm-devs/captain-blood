#pragma once

#include "..\..\common_h\render.h"
#include "..\..\common_h\gui.h"

#include "GraphEditor.h"

class ForceEditorMainWindow : public GUIWindow
{
	GraphEditor *graph;

	GUIGraphLine *ls;
	GUIGraphLine *rs;

	GUILabel *caption; GUIEdit *edit;

public:

	 ForceEditorMainWindow();
	~ForceEditorMainWindow();

public:

	void _cdecl InitEffect(GUIControl* sender);

	void _cdecl LoadEffect(GUIControl* sender);
	void _cdecl Load	  (GUIControl* sender);

	void _cdecl SaveEffect(GUIControl* sender);
	void _cdecl Save	  (GUIControl* sender);

	void _cdecl PlayEffect(GUIControl* sender);
	void _cdecl Play	  (GUIControl* sender);

	void _cdecl Time	  (GUIControl* sender);
	void _cdecl Exit	  (GUIControl* sender);

};
