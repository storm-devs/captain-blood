#ifndef GROUP_ATTRIBUE
#define GROUP_ATTRIBUE

#include "..\baseattr.h"




class GroupAttribute : public BaseAttribute
{

	string GroupName;

	GUITreeNode* RootNode;

	bool bExpanded;




public:	

	array<BaseAttribute*> Childs;


	GroupAttribute();
	~GroupAttribute();


	void PopupEdit (int pX, int pY) {};

	void AddToWriter (MOPWriter& wrt);

	void WriteToFile (IFile* pFile);

	void LoadFromFile (IFile* pFile, const char* ClassName) {};


	/* Добавляем к ТриВиев*/
	void Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v = NULL);
	virtual void UpdateTree(GUITreeNode * node, string * v = NULL);


	GroupAttribute & operator = (const GroupAttribute & source);
	GroupAttribute & operator = (const IMOParams::Group& source);

	virtual void SetMasterData (void* data);

	virtual void GroupHack (AttributeList* OriginalAttrList );


	void BeforeDelete ();


	void DebugLog (int deep);

	void GenerateFlatList(array<BaseAttribute*> &FlatList);


	virtual void WriteToXML (TextFile &file, int level);


	void ReadXML (TiXmlElement* Root, const char* szMasterClass);

};


#endif