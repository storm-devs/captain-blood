#ifndef COLOR_ATTRIBUE
#define COLOR_ATTRIBUE

#include "..\baseattr.h"
#include "colorform.h"



class ColorAttribute : public BaseAttribute
{
	MissionEditor::tCreatedMO* EditedObject;
	Color Saved;


  Color value;
	Color min;
	Color max;

public:

	ColorAttribute & operator = (const ColorAttribute & source);
	ColorAttribute & operator = (const IMOParams::Colors& source);

	TColorEdit* ColorEdit;


	ColorAttribute ();
  ~ColorAttribute ();


  virtual void SetValue (const Color& val);
	virtual const Color& GetValue () const;

	virtual void SetMin (const Color& val);
  virtual const Color& GetMin () const;

  virtual void SetMax (const Color& val);
	virtual const Color& GetMax () const;

  
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


	virtual void _cdecl OnColorApply (GUIControl* sender);
	virtual void _cdecl OnColorCancel (GUIControl* sender);
	virtual void _cdecl OnColorChange (GUIControl* sender);


	virtual bool NeedApplyCancelButtonInRT ()  {return false;};

  
	virtual void WriteToXML (TextFile &file, int level);

	void ReadXML (TiXmlElement* Root, const char* szMasterClass);

};


#endif