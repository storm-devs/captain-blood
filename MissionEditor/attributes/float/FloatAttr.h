#ifndef FLOAT_ATTRIBUE
#define FLOAT_ATTRIBUE

#include "..\baseattr.h"
#include "floatform.h"


class FloatAttribute : public BaseAttribute
{
	float SavedValue;
	GUIEdit* RealTimeValue;
	GUILabel* RealTimeDesc;
	MissionEditor::tCreatedMO* EditedObject;



  float value;
  float min;
  float max;

public:

	FloatAttribute & operator = (const FloatAttribute & source);
	FloatAttribute & operator = (const IMOParams::Float& source);

  TFloatEdit* Form;

	FloatAttribute ();
  ~FloatAttribute ();


  virtual void SetValue (float val);
	virtual float GetValue () const;

  virtual void SetMin (float val);
  virtual float GetMin () const ;

  virtual void SetMax (float val);
  virtual float GetMax () const;

  
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


	virtual void _cdecl OnRTValueChange (GUIControl* sender);
	virtual void _cdecl OnRTAdvChange (GUIControl* sender);

	virtual void SetupMissionObject ();

	virtual bool IsSupportRTEdit () {return true;};

	virtual void Clamp ();

	virtual void WriteToXML (TextFile &file, int level);

	
  
	void ReadXML (TiXmlElement* Root, const char* szMasterClass);

};


#endif