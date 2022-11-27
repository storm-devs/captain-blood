#ifndef STRING_ATTRIBUE
#define STRING_ATTRIBUE

#include "..\baseattr.h"
#include "StringForm.h"


class StringAttribute : public BaseAttribute
{



 //string value;
 char data_storage[101];
 
 int minChars;
 int maxChars;

public:


	StringAttribute & operator = (const StringAttribute & source);
	StringAttribute & operator = (const IMOParams::String& source);

 TStringEdit* Form;

		union
		{
			DWORD flags;			//Флаги
			struct
			{
				DWORD onlyLo : 1;	//Только маленькие буквы
				DWORD onlyHi : 1;	//Только большие буквы
			};
		};


	StringAttribute ();
  ~StringAttribute ();


  virtual void SetValue (const char* value);
	virtual const char* GetValue () const;

  virtual void SetMaxChars (int Max);
	virtual int GetMaxChars () const;

  virtual void SetMinChars (int Min);
	virtual int GetMinChars () const;

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