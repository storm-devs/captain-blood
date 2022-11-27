#ifndef ENUM_ATTRIBUE
#define ENUM_ATTRIBUE

#include "..\baseattr.h"
#include "..\..\..\common_h\gui.h"
#include "enumform.h"

class ArrayAttribute;
class GroupAttribute;

class EnumAttribute : public BaseAttribute
{
	int SavedIndex;
	string SavedValue;
	MissionEditor::tCreatedMO* EditedObject;
	GUIComboBox* cValues;



	string curvalue;
	int current_index;
  GUIStringList values;


	BaseAttribute* FindInArray (ArrayAttribute* pArray, const char *szAttrName);
	BaseAttribute* FindInGroup (GroupAttribute* pGroup, const char *szAttrName);


	BaseAttribute* FindEnumInAttrList (AttributeList* attlist, const char *szAttrName);
	
  
public:


	EnumAttribute & operator = (EnumAttribute & source);
	EnumAttribute & operator = (const IMOParams::Enum& source);

  TEnumEdit* Form;

	EnumAttribute ();
	~EnumAttribute ();


	virtual void Clear ();
	virtual void AddEnum (const string& val);

	virtual int GetEnumCount () const;
	virtual const string& GetEnum (int num);

	virtual void SetValue (int index);
	virtual int GetValue () const;
	virtual const string& GetStringValue () const;

  
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


	virtual void _cdecl ValueChange (GUIControl* sender);


	virtual void WriteToXML (TextFile &file, int level);


	void ReadXML (TiXmlElement* Root, const char* szMasterClass);

};


#endif