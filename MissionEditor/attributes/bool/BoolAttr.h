#ifndef BOOL_ATTRIBUE
#define BOOL_ATTRIBUE

#include "..\baseattr.h"
#include "boolform.h"
#include "..\..\..\common_h\gui.h"


class BoolAttribute : public BaseAttribute
{

	bool SavedValue;
	GUICheckBox* cbRealTimeValue;
	MissionEditor::tCreatedMO* EditedObject;


	bool value;

public:

	BoolAttribute & operator = (const BoolAttribute & source);
	BoolAttribute & operator = (const IMOParams::Bool& source);

	

  TBoolEdit* Form;

	BoolAttribute ();
	~BoolAttribute ();


	virtual void SetValue (bool val);
	virtual bool GetValue () const;

  
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

	virtual void SetupMissionObject ();

	virtual bool IsSupportRTEdit () {return true;};

	virtual void WriteToXML (TextFile &file, int level);

	void ReadXML (TiXmlElement* Root, const char* szMasterClass);



};


#endif