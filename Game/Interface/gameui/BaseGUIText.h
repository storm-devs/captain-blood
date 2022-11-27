#pragma once

#include "..\..\..\Common_h\IGUIText.h"

class GUITextManager : public IGUITextManager
{
public:

	 GUITextManager();
	~GUITextManager();

public:

	IGUIText *CreateText();

public:

	bool Create(MOPReader &reader);

	void Activate(bool isActive);
	void Show	 (bool isShow);

	MO_IS_FUNCTION(GUITextManager, MissionObject);

	void Command(const char *id, dword numParams, const char **params);

private:

	void InitParams();
	void CreateBuffers();

private:

	//

};
