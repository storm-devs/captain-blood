#ifndef ROTATION_ATTRIBUE
#define ROTATION_ATTRIBUE

#include "..\baseattr.h"
#include "RotationForm.h"


class RotationAttribute : public BaseAttribute
{

	Vector SavedValue;
	GUIEdit* RealTimeValueX;
	GUIEdit* RealTimeValueY;
	GUIEdit* RealTimeValueZ;
	GUILabel* RealTimeDesc;
	MissionEditor::tCreatedMO* EditedObject;

	GUIButton* btnSetFromCamera;
	GUIButton* btnSetToZero;
	GUIButton* btnCopyFrom;
	GUIButton* btnPasteTo;





 Vector value;
 Vector min;
 Vector max;

public:

	RotationAttribute & operator = (const RotationAttribute & source);
	RotationAttribute & operator = (const IMOParams::Angles& source);

 TRotationEdit* Form;

	RotationAttribute ();
  ~RotationAttribute ();


  virtual void SetValue (const Vector& value);
	virtual const Vector& GetValue () const;

  virtual void SetMax (const Vector& Max);
	virtual const Vector& GetMax () const;

  virtual void SetMin (const Vector& Min);
	virtual const Vector& GetMin () const;

  
  virtual void PopupEdit (int pX, int pY);
  
  virtual void AddToWriter (MOPWriter& wrt);
  
	virtual void WriteToFile (IFile* pFile);

	virtual void LoadFromFile (IFile* pFile, const char* ClassName);

	virtual void Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v = NULL);
	virtual void UpdateTree(GUITreeNode * node, string * v = NULL);


	/* Начать изменение аттрибута в реалтайме... */
	virtual void BeginRTEdit (MissionEditor::tCreatedMO* MissionObject, GUIControl* parent);

	/* Подтвердить изменение аттрибута... */
	virtual void ApplyRTEdit ();

	/* Отменить изменения в аттрибуте... */
	virtual void CancelRTEdit ();
	virtual void CloseRTEdit ();



	virtual void SetupMissionObject ();

	virtual bool IsSupportRTEdit () {return true;};

	virtual void Clamp ();


	virtual void _cdecl OnRTValueChangeX (GUIControl* sender);
	virtual void _cdecl OnRTAdvChangeX (GUIControl* sender);
	virtual void _cdecl OnRTValueChangeY (GUIControl* sender);
	virtual void _cdecl OnRTAdvChangeY (GUIControl* sender);
	virtual void _cdecl OnRTValueChangeZ (GUIControl* sender);
	virtual void _cdecl OnRTAdvChangeZ (GUIControl* sender);


	void ClampAndLimit (float& angle, float minLimit, float maxLimit, bool IsLimit);

	virtual void _cdecl OnCameraCapture (GUIControl* sender);
	virtual void _cdecl OnZero (GUIControl* sender);

	virtual void _cdecl OnCopy (GUIControl* sender);
	virtual void _cdecl OnPaste (GUIControl* sender);


	virtual void WriteToXML (TextFile &file, int level);


	void ReadXML (TiXmlElement* Root, const char* szMasterClass);

	

};


#endif