#ifndef ARRAY_ATTRIBUE
#define ARRAY_ATTRIBUE

#include "..\baseattr.h"
#include "arrayform.h"




class ArrayAttribute : public BaseAttribute
{

	bool sysDelete;
	string tmpText;
	int min;
	int max;


public:	
	struct ArrayItemInfo
	{
		ArrayAttribute* array;
		void* element;  //ArrayAttribute::ArrayElement
	};


	/* Описание одного элемента массива */
	/* все значения тут по умолчанию... */
	struct ArrayElement
	{
		ArrayItemInfo info;
		array<BaseAttribute*> elements;
		bool Expanded;
		GUITreeNode* itemNode;

		ArrayElement () : elements (_FL_, 128)
		{
			info.array = NULL;
			info.element = NULL;
			Expanded = false;
			itemNode = NULL;
		}
	};

protected:

	ArrayElement defaultelement;
	

	/*
	 Сам массив...
	*/
	array<ArrayElement> ArrayValues;


	bool RootExpanded;
	GUITreeNode* RootNode;


public:

	ArrayAttribute & operator = (ArrayAttribute & source);
	ArrayAttribute & operator = (const IMOParams::Array& source);

	virtual void SetMasterData (void* data);


	ArrayAttribute::ArrayElement& GetDefaultElement ();


	void CleanUp ();


	void Copy (ArrayElement &to, ArrayElement &from);

  TArrayEdit* Form;

	ArrayAttribute ();
	~ArrayAttribute ();



	virtual int GetMinElementCount ();
	virtual int GetMaxElementCount ();
	virtual void SetMinElementCount (int min);
	virtual void SetMaxElementCount (int max);

	virtual int GetElementsCount ();
	virtual BaseAttribute* GetElement (int index);
	virtual void AddElement (BaseAttribute* value);

	virtual int GetValuesCount ();
	virtual ArrayAttribute::ArrayElement* GetValue (int index); 

  virtual void AddToWriter (MOPWriter& wrt);
	virtual void WriteToFile (IFile* pFile);
	virtual void LoadFromFile (IFile* pFile, const char* ClassName);

	virtual void Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v = NULL);
	virtual void UpdateTree(GUITreeNode * node, string * v = NULL);

  
  virtual void PopupEdit (int pX, int pY);


	virtual void Resize (int NewSize);
  
	virtual void BeforeDelete ();

	virtual void AfterLoad (BaseAttribute* baseattr, AttributeList* OriginalAttrList, const char* szClassName);


	virtual int GetElementIndex (ArrayElement* element);
	virtual void RemoveValue (int index);
	virtual void InsertValue (int index, ArrayAttribute::ArrayElement* copyfrom = NULL);


	void DebugLog (int deep);


	virtual void WriteToXML (TextFile &file, int level);

	void ReadXML (TiXmlElement* Root, const char* szMasterClass);

};


#endif