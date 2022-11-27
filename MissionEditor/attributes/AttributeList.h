#ifndef ATTRIBUE_LIST
#define ATTRIBUE_LIST


#include "baseattr.h"
#include "bool\BoolAttr.h"
#include "long\LongAttr.h"
#include "float\FloatAttr.h"
#include "position\positionAttr.h"
#include "Rotation\RotationAttr.h"
#include "String\StringAttr.h"
#include "LocString\LocStringAttr.h"
#include "Color\ColorAttr.h"
#include "Enumerate\EnumAttr.h"
#include "Array\ArrayAttr.h"
#include "Group\GroupAttr.h"



class TextFile;
class TiXmlElement;

class AttributeList
{

 array <BaseAttribute*> Storage;


 BaseAttribute* FindInArray (ArrayAttribute* pArray, const char *szAttrName, IMOParams :: Type Type);
 BaseAttribute* FindInGroup (GroupAttribute* pGroup, const char *szAttrName, IMOParams :: Type Type);


public:

	AttributeList ();
	~AttributeList ();

	void CreateFromParams (IMOParams* params);

	void CreateFromList (AttributeList* list);

	void AddParam (BaseAttribute* attr);
	
  __forceinline int Add (BaseAttribute* pPtr)
	{
		return Storage.Add (pPtr);
	}

	
	void Clear ();
	
	__forceinline int GetCount ()
	{
		return Storage.Size();
	}

	
	__forceinline BaseAttribute* Get(int index)
	{
		return Storage[index];
	}

	
  __forceinline void Insert (int index, BaseAttribute* pPtr)
	{
		Storage.Insert (pPtr, index);
	}

	
	__forceinline void Delete (int index)
	{
		Storage.DelIndex (index);
	}

	
	__forceinline BaseAttribute* operator[] (int index)
	{
		return Get(index);
	}



	void AddToTree (GUITreeNode* node, TreeNodesPool* nodesPool);

	void AddToWriter (MOPWriter& wrt);



	BaseAttribute* CreateFromParam (const IMOParams::Param* m_param);

/* Вызываеться перед удалением из дерева */
	virtual void BeforeDelete ();

	
	virtual void SetMasterData (void* data);

	virtual int GetCountForSave ();

	int GetAttributesInGroup(GroupAttribute* pattr);


	__forceinline BaseAttribute* FindInAttrList (const char *szAttrName, IMOParams :: Type Type)
	{
		for (int n = 0; n < GetCount(); n++)
		{
			BaseAttribute* SourceAttr = Get(n);
			if (SourceAttr->GetType() == Type)
			{
				if (string::IsEqual (SourceAttr->GetName(), szAttrName))
				{
					return SourceAttr;
				}
			}

			if (SourceAttr->GetType() == IMOParams::t_array)
			{
				ArrayAttribute* ArrayAttr = (ArrayAttribute*) SourceAttr;

				BaseAttribute* pRes = FindInArray (ArrayAttr, szAttrName, Type);
				if (pRes) return pRes;
			}

			if (SourceAttr->GetType() == IMOParams::t_group)
			{
				GroupAttribute* GroupAttr = (GroupAttribute*) SourceAttr;

				BaseAttribute* pRes = FindInGroup (GroupAttr, szAttrName, Type);
				if (pRes) return pRes;
			}
		}

		return NULL;
	}


	void DebugLog ();


	static const char* GetTextType (IMOParams::Type type);


	void GenerateFlatList(array<BaseAttribute*> &FlatList);

	
	void WriteToXML (TextFile &file, int level);
	void ReadXML (TiXmlElement* Root, const char* szMasterClass);


	static IMOParams::Type GetTypeFromXML (TiXmlElement* Root);
	


};


#endif