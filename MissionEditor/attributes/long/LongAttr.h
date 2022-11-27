#ifndef LONG_ATTRIBUE
#define LONG_ATTRIBUE

#include "..\baseattr.h"
#include "longform.h"


class LongAttribute : public BaseAttribute
{

	float ost;
	long SavedValue;
	GUIEdit* RealTimeValue;
	GUILabel* RealTimeDesc;
	MissionEditor::tCreatedMO* EditedObject;



  long value;
	long min;
	long max;

public:

	LongAttribute & operator = (const LongAttribute & source);
	LongAttribute & operator = (const IMOParams::Long& source);

  TLongEdit* Form;

	LongAttribute ();
  ~LongAttribute ();


	virtual void SetValue (long val);
	virtual long GetValue () const;

	virtual void SetMin (long val);
  virtual long GetMin () const;

	virtual void SetMax (long val);
	virtual long GetMax () const;

  
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