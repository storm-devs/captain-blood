#ifndef _FIELD_LIST_H
#define _FIELD_LIST_H

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "../../../common_h/core.h"
#include "../../../common_h/templates.h"
#include "..\..\icommon\types.h"
#include "..\..\icommon\memfile.h"
#include "..\datadesc\data_desc.h"
#include "..\..\icommon\names.h"




class DataColor;
class DataBool;
class DataFloat;
class DataGraph;
class DataString;
class DataPosition;
class DataUV;
class TextFile;

class DataSource;

#ifndef _XBOX
class TiXmlElement;
#endif


class FieldList
{
public:

  struct FieldDesc
  {
		//При конвертации используеться этот флажок
		bool MarkForDelete;

		dword dwGUID;

		//Хэш имени
		DWORD HashValue;

		//Тип
    FieldType Type;

		//Имя
    const char* szName;

		//имя для редактора
		const char* szEditorName;

		//Указатель
    void* pPointer;

		FieldDesc()
		{
			dwGUID = 0xFFFFFFFF;
			szName = NULL;
			szEditorName = NULL;
			pPointer = NULL;
		}
  };
  
private:

	array<string> CachedGeomNames;
	array<dword> CachedForceFieldsID;


	FieldDesc Fields[GUID_MAX];


	void DeleteFieldData (const FieldList::FieldDesc& pData);


	//=================== создание пустых аттрибутов
	void CreateEmptyBoolField (dword guid, const char* Name, const char* EditorName, bool def_value);
	void CreateEmptyFloatField (dword guid, const char* Name, const char* EditorName, float def_value);
	void CreateEmptyGraphField (dword guid, const char* Name, const char* EditorName, float def_value_min, float def_value_max);
	void CreateEmptyPositionField (dword guid, const char* Name, const char* EditorName, const Vector& def_value);
	void CreateEmptyStringField (dword guid, const char* Name, const char* EditorName, const char* def_value);
	void CreateEmptyUVField (dword guid, const char* Name, const char* EditorName);
	void CreateEmptyColorField (dword guid, const char* Name, const char* EditorName, DWORD def_value);


	//=================== Прогрузка аттрибутов из файла
	void CreateBoolField (MemFile* pMemFile, DataDescripion* pDataDescriptor);
	void CreateFloatField (MemFile* pMemFile, DataDescripion* pDataDescriptor);
	void CreateGraphField (MemFile* pMemFile, DataDescripion* pDataDescriptor);
	void CreatePositionField (MemFile* pMemFile, DataDescripion* pDataDescriptor);
	void CreateStringField (MemFile* pMemFile, DataDescripion* pDataDescriptor);
	void CreateUVField (MemFile* pMemFile, DataDescripion* pDataDescriptor);
	void CreateColorField (MemFile* pMemFile, DataDescripion* pDataDescriptor);

#ifndef _XBOX
	//=================== Прогрузка аттрибутов из файла XML
	void CreateBoolFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor);
	void CreateFloatFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor);
	void CreateGraphFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor);
	void CreatePositionFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor);
	void CreateStringFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor);
	void CreateUVFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor);
	void CreateColorFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor);
#endif


	//FieldList::FieldDesc* FindField (const char* Name);

	DataSource* pMaster;


	dword GetFieldGUIDByName (const char* name, DataDescripion* pDataDescriptor);

public:

  FieldList ();
  ~FieldList ();


	void SetMasterDataSource(DataSource* Master);

	void DelAll ();

  
//Сохранить/загрузить...
	void Load (MemFile* File, DataDescripion* pDataDescriptor);


#ifndef _XBOX
	void LoadXML (TiXmlElement* root, DataDescripion* pDataDescriptor);

	void WriteXML (TextFile* xmlFile, dword level);

#endif

  void Write (MemFile* File);


	DataColor* FindColorByGUID (dword GUID);
	DataGraph* FindGraphByGUID (dword GUID);
	DataUV* FindUVByGUID (dword GUID);


	DataBool* FindBoolByGUID (dword GUID);
	DataFloat* FindFloatByGUID (dword GUID);
	DataString* FindStringByGUID (dword GUID);
	DataPosition* FindPositionByGUID (dword GUID);

/*
	DataColor* FindColor (const char* AttrName);
	DataGraph* FindGraph (const char* AttrName);
	DataUV* FindUV (const char* AttrName);

	DataBool* FindBool (const char* AttrName);
	DataFloat* FindFloat (const char* AttrName);
	DataString* FindString (const char* AttrName);
	DataPosition* FindPosition (const char* AttrName);
*/

	float GetFloat (dword dwGUID, float def_value = 0.0f);
	int GetFloatAsInt (dword dwGUID, int def_value = 0);
	bool GetBool (dword dwGUID, bool def_value = false);
	const char* GetString (dword dwGUID, const char* def_value = "");
	const Vector& GetPosition (dword dwGUID, const Vector& def_value = Vector(0.0f));

	float GetGraphVal (dword dwGUID, float Time, float LifeTime, float K_Rand, float def_value = 0.0f);
	float GetRandomGraphVal (dword dwGUID, float Time, float LifeTime, float def_value = 0.0f);


	DWORD _GetFieldCount ();
	const FieldList::FieldDesc& _GetFieldByIndex (DWORD Index);



//Конвертация ГАРАНТИРУЕТ, что в данном DataSource будут нужные поля
//и не будет лишнего мусора...
	void Convert (DataDescripion* pDataDescriptor);



	virtual void UpdateCache ();


	const char* GetRandomModelNameFromCache();
	dword GetForceFieldsCountFromCache ();
	dword GetForceFieldGUIDFromCache(dword dwIndex);


/*
protected:

	FieldList(const FieldList & source)
	{

	}
*/


/*protected:
	FieldList & operator = (const FieldList & source) 
	{ 
		return *this;
	}
*/
  

};


#endif