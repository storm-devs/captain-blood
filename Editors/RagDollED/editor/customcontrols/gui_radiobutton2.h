#pragma once

#include "..\..\..\..\common_h\gui.h"

extern GUIEventHandler *hack_temp_ptr;

#define OnClick \
	t_OnClick->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
	*(hack_temp_ptr)

class GUIRadioButton2 : public GUIControl
{
	bool tabbed;

protected:

	float MD_Time;

	virtual bool ProcessMessages(GUIMessage message, DWORD lparam, DWORD hparam);

public:

	int GroupID;

	bool Checked;

	DWORD FontColor;

	GUIImage *ImageNormal;
	GUIImage *ImageChecked;

	GUIEventHandler *t_OnClick;

	virtual void OnMDown (int MouseKey, const GUIPoint& pt);

	GUIRadioButton2(GUIControl *parent, int x, int y, int w, int h, bool tabbed = false);
	virtual ~GUIRadioButton2();
  
	virtual void Draw ();

	virtual bool Is(const char *className)
	{
		if( strcmp(className,"GUIRadioButton2") == 0 )
			return true;

		return GUIControl::Is(className);
	}

};
