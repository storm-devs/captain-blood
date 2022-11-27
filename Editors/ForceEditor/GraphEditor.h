#pragma once

#include "gui\fe_grapheditor.h"

class GraphEditor : public GUIGraphEditor
{
public:

	GraphEditor(GUIControl* pParent, int Left, int Top, int Width, int Height)
		: GUIGraphEditor(pParent,Left,Top,Width,Height)
	{
	}

	void Draw();

protected:

	bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);

};
