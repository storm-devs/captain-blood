#ifndef LOCALIZED_STRING_ATTRIBUE
#define LOCALIZED_STRING_ATTRIBUE

#include "..\baseattr.h"
#include "LocStringForm.h"
#include "localtext.h"


class LocStringAttribute : public BaseAttribute
{



	long currentID;
 

public:


	LocStringAttribute & operator = (const LocStringAttribute & source);
	LocStringAttribute & operator = (const IMOParams::LocString& source);

 TLocStringEdit* Form;



	LocStringAttribute ();
  ~LocStringAttribute ();


  virtual void SetValue (long strID);
	virtual long GetValue () const;


  virtual void PopupEdit (int pX, int pY);
  
  virtual void AddToWriter (MOPWriter& wrt);
  

	virtual void WriteToFile (IFile* pFile);

	virtual void LoadFromFile (IFile* pFile, const char* ClassName);

	virtual void Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v = NULL);
	virtual void UpdateTree(GUITreeNode * node, string * v = NULL);

	virtual void WriteToXML (TextFile &file, int level);


	void ReadXML (TiXmlElement* Root, const char* szMasterClass);
};


#endif