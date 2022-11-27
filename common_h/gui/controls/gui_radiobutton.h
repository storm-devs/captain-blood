#ifndef _XBOX
#ifndef GUI_RADIOBUTTON
#define GUI_RADIOBUTTON

#include "gui_control.h"
#include "gui_font.h"

#include "gui_events.h"
#include "gui_image.h"



class GUIRadioButton : public GUIControl
{
  
  
  
protected:
  
  float MD_Time;
  
  virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);
  
public:
  
  bool Checked;
  
  DWORD FontColor;
  
  GUIImage* ImageNormal;
  GUIImage* ImageChecked;
  
  
  virtual void OnMDown (int MouseKey, const GUIPoint& pt);
  
  
  GUIRadioButton (GUIControl* pParent, int Left, int Top, int Width, int Height);
  
  virtual ~GUIRadioButton ();
  
  virtual void Draw ();
  
  
  
	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUIRadioButton") == 0) return true;
		return GUIControl::Is (className);
	}
  
  
  
  
};



#endif

#endif