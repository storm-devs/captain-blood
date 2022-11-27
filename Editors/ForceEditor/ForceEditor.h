#pragma once

#include "..\..\common_h\render.h"

class ForceEditorMainWindow;

class ForceEditor : public RegObject
{
	ForceEditorMainWindow *main;

public:

	 ForceEditor();
	~ForceEditor();

public:

	bool Init();

};
