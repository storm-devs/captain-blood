#ifndef UV_EDITOR
#define UV_EDITOR

#include "base.h"
#include "..\..\..\common_h\gui.h"

#include "..\CustomControls\texturesselector.h"

class DataUV;

class UVEditor : public BaseEditor
{

	TTextureSelector* TSelectorForm;
	DataUV* EditedUV;


public:


	UVEditor ();
  virtual ~UVEditor ();
  
  virtual void Release ();  

	virtual void BeginEdit (GUIControl* form, DataUV* EditedUV);
	virtual void EndEdit ();
	virtual void Apply ();

//====================================
public:
	virtual void _cdecl CloseForm (GUIControl* sender);


};





#endif
