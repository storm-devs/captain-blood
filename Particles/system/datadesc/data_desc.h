#ifndef DATA_DESCRIPTION_CLASS
#define DATA_DESCRIPTION_CLASS

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "../../../common_h/core.h"
#include "../../../common_h/templates.h"
#include "../../icommon/types.h"


#define MAX_DESC_COUNT 128

class DataDescripion
{

	struct DescItem
	{
		dword guid;
		FieldType Type;
		const char* Name;
		const char* EditorName;
	};

	DWORD ItemsCount;
	DescItem Fields[MAX_DESC_COUNT];

public:
 
 DataDescripion ();
 ~DataDescripion ();
 
 void AddField (FieldType Type, dword guid, const char* Name, const char* EditorName);
 
 dword GetFieldGUID (DWORD Index);
 const char* GetFieldName (DWORD Index);
 const char* GetFieldEditorName (DWORD Index);
 FieldType GetFieldType (DWORD Index);
 int GetFieldCount ();

 void Clear ();

 virtual void CreateFields () = 0;
};





#define BEGIN_DATA_DESC(Name)													\
class DataDescripion__##Name : public DataDescripion  \
{																										  \
public:																								\
	virtual void CreateFields ()												\
	{																										\
		Clear ();																					\



#define END_DATA_DESC(Name)														\
	}																										\
	DataDescripion__##Name()														\
	{																										\
		CreateFields ();																	\
	}																										\
};																										\
DataDescripion__##Name  Name;													\




#define DATA_COLOR(guid, name,EditorName)			AddField(FIELD_COLOR, guid, name,EditorName);
#define DATA_BOOL(guid, name,EditorName)				AddField(FIELD_BOOL, guid, name,EditorName);
#define DATA_FLOAT(guid, name,EditorName)			AddField(FIELD_FLOAT, guid, name,EditorName);
#define DATA_GRAPH(guid, name,EditorName)			AddField(FIELD_GRAPH, guid, name,EditorName);
#define DATA_POSITION(guid, name,EditorName)		AddField(FIELD_POSITION, guid, name,EditorName);
#define DATA_UV(guid, name,EditorName)					AddField(FIELD_UV, guid, name,EditorName);
#define DATA_STRING(guid, name,EditorName)			AddField(FIELD_STRING, guid, name,EditorName);


#endif