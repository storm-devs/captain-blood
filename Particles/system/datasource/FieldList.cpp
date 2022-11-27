
#include "..\..\..\common_h\core.h"
#include "fieldlist.h"
#include "databool.h"
#include "datacolor.h"
#include "datafloat.h"
#include "datagraph.h"
#include "dataposition.h"
#include "datauv.h"
#include "datastring.h"
#include "datasource.h"

#include "..\ParticleProcessor\nameparser.h"
#include "..\..\..\common_h\particles\iparticlemanager.h"

#include "..\..\service\particleservice.h"

extern ParticleService* PService;

#ifndef _XBOX
#include "..\..\..\common_h\tinyxml\tinyxml.h"
#endif

#include "..\..\TextFile.h"

#include "..\..\..\Common_h\data_swizzle.h"






FieldList::FieldList () : CachedGeomNames(_FL_),
													CachedForceFieldsID(_FL_)
{

	for (dword n = 0; n < GUID_MAX; n++)
	{
		Fields[n].Type = FIELD_UNKNOWN;
	}
	
	

	pMaster = NULL;
}

void FieldList::SetMasterDataSource(DataSource* Master)
{
	pMaster = Master;
}

FieldList::~FieldList ()
{
	//нельзя тут делать DelAll надо тогда будет делать нормальный ctor копирования, и оператор присваивания
	//а без этого в массиве будут вызывать постоянно деструкторы и жопа
	//DelAll ();
}


#ifndef _XBOX
void FieldList::CreateBoolFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor)
{
	DataBool* Field = NEW DataBool(this);
	Field->LoadXML(root);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_BOOL;

	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;

	}

	//Fields.Add(pDesc);
}

void FieldList::CreateFloatFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor)
{
	DataFloat* Field = NEW DataFloat(this);
	Field->LoadXML(root);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_FLOAT;
	
	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;

	}

	//Fields.Add(pDesc);
}

void FieldList::CreateGraphFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor)
{
	//DataGraph* Field = NEW DataGraph(this);
	DataGraph* Field = PService->AllocateDataGraph(this);
	Field->LoadXML(root);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_GRAPH;

	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;

	}

	//Fields.Add(pDesc);
}

void FieldList::CreatePositionFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor)
{
	DataPosition* Field = NEW DataPosition(this);
	Field->LoadXML(root);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_POSITION;

	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;

	}

	//Fields.Add(pDesc);
}

void FieldList::CreateStringFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor)
{
	DataString* Field = NEW DataString(this);
	Field->LoadXML(root);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_STRING;

	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;

	}

	//Fields.Add(pDesc);
}

void FieldList::CreateUVFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor)
{
	DataUV* Field = NEW DataUV(this);
	Field->LoadXML(root);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_UV;

	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;

	}

	//Fields.Add(pDesc);
}

void FieldList::CreateColorFieldXML (TiXmlElement* root, DataDescripion* pDataDescriptor)
{
	DataColor* Field = NEW DataColor(this);
	Field->LoadXML(root);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_COLOR;

	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;

	}

	//Fields.Add(pDesc);
}




void FieldList::LoadXML (TiXmlElement* root, DataDescripion* pDataDescriptor)
{
	for(TiXmlElement* child = root->FirstChildElement(); child; child = child->NextSiblingElement())
	{
		string NodeName = child->Value();
		string NodeVal = child->Attribute("val");

		if (NodeName != "Field") continue;

		if (NodeVal == "FIELD_BOOL")
		{
			CreateBoolFieldXML (child, pDataDescriptor);
		}

		if (NodeVal == "FIELD_FLOAT")
		{
			CreateFloatFieldXML (child, pDataDescriptor);
		}

		if (NodeVal == "FIELD_GRAPH")
		{
			CreateGraphFieldXML (child, pDataDescriptor);
		}

		if (NodeVal == "FIELD_POSITION")
		{
			CreatePositionFieldXML (child, pDataDescriptor);
		}

		if (NodeVal == "FIELD_STRING")
		{
			CreateStringFieldXML (child, pDataDescriptor);
		}

		if (NodeVal == "FIELD_UV")
		{
			CreateUVFieldXML (child, pDataDescriptor);
		}

		if (NodeVal == "FIELD_COLOR")
		{
			CreateColorFieldXML (child, pDataDescriptor);
		}

		
	}
}

#endif

void FieldList::Load (MemFile* File, DataDescripion* pDataDescriptor)
{
	dword DataFieldsCount = 0;
	File->ReadType(DataFieldsCount);
	XSwizzleDWord(DataFieldsCount);

	for (DWORD n = 0; n < DataFieldsCount; n++)
	{
		FieldType fldType = FIELD_UNKNOWN;
		File->ReadType(fldType);
		fldType = (FieldType)SwizzleDWord(fldType);

		switch (fldType)
		{
			case FIELD_BOOL:
			{
				//api->Trace ("Particles info: BOOL field");
				CreateBoolField (File, pDataDescriptor);
				break;
			}
			case FIELD_FLOAT:
			{
				//api->Trace ("Particles info: FLOAT field");
				CreateFloatField (File, pDataDescriptor);
				break;
			}
			case FIELD_GRAPH:
			{
				//api->Trace ("Particles info: GRAPH field");
				CreateGraphField (File, pDataDescriptor);
				break;
			}
			case FIELD_POSITION:
			{
				//api->Trace ("Particles info: POSITION field");
				CreatePositionField (File, pDataDescriptor);
				break;
			}
			case FIELD_STRING:
			{
				//api->Trace ("Particles info: STRING field");
				CreateStringField (File, pDataDescriptor);
				break;
			}
			case FIELD_UV:
			{
				//api->Trace ("Particles info: UV field");
				CreateUVField (File, pDataDescriptor);
				break;
			}
			case FIELD_COLOR:
			{
				//api->Trace ("Particles info: COLOR field");
				CreateColorField (File, pDataDescriptor);
				break;
			}
			default:
			{
				throw ("Particles: Unknown field type !!!!");
			}
		}	//switch
	} // for all fileds
}



void FieldList::CreateEmptyBoolField (dword guid, const char* Name, const char* EditorName, bool def_value)
{
	DataBool* Field = NEW DataBool(this);
	Field->SetName(Name, EditorName);
	Field->SetValue(def_value);

	FieldDesc pDesc;
	pDesc.dwGUID = guid;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.szEditorName = EditorName;
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_BOOL;
	//Fields.Add(pDesc);
	Fields[guid] = pDesc;
}

void FieldList::CreateEmptyFloatField (dword guid, const char* Name, const char* EditorName, float def_value)
{
	DataFloat* Field = NEW DataFloat(this);
	Field->SetName(Name, EditorName);
	Field->SetValue(def_value);

	FieldDesc pDesc;
	pDesc.dwGUID = guid;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.szEditorName = EditorName;
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_FLOAT;
	//Fields.Add(pDesc);
	Fields[guid] = pDesc;
}

void FieldList::CreateEmptyGraphField (dword guid, const char* Name, const char* EditorName, float def_value_min, float def_value_max)
{
	//DataGraph* Field = NEW DataGraph(this);
	DataGraph* Field = PService->AllocateDataGraph(this);
	Field->SetName(Name, EditorName);

	if (crt_stricmp (PARTICLE_COLORMULTIPLY, Name) == 0)
	{
		Field->SetDefaultValue(1.0f, 1.0f);
	} else
	{
		Field->SetDefaultValue(def_value_max, def_value_min);
	}
	

	FieldDesc pDesc;
	pDesc.dwGUID = guid;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.szEditorName = EditorName;
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_GRAPH;
	//Fields.Add(pDesc);
	Fields[guid] = pDesc;
}

void FieldList::CreateEmptyPositionField (dword guid, const char* Name, const char* EditorName, const Vector& def_value)
{
	DataPosition* Field = NEW DataPosition(this);
	Field->SetName(Name, EditorName);
	Field->SetValue(def_value);

	FieldDesc pDesc;
	pDesc.dwGUID = guid;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.szEditorName = EditorName;
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_POSITION;
	//Fields.Add(pDesc);
	Fields[guid] = pDesc;
}

void FieldList::CreateEmptyStringField (dword guid, const char* Name, const char* EditorName, const char* def_value)
{
	DataString* Field = NEW DataString(this);
	Field->SetName(Name, EditorName);
	Field->SetValue(def_value);

	FieldDesc pDesc;
	pDesc.dwGUID = guid;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.szEditorName = EditorName;
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_STRING;
	//Fields.Add(pDesc);
	Fields[guid] = pDesc;

}

void FieldList::CreateEmptyUVField (dword guid, const char* Name, const char* EditorName)
{
	DataUV* Field = NEW DataUV(this);
	Field->SetName(Name, EditorName);
	Field->SetValues(&Vector4(0.0f, 0.0f, 1.0f, 1.0f), 1);

	FieldDesc pDesc;
	pDesc.dwGUID = guid;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.szEditorName = EditorName;
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_UV;
	//Fields.Add(pDesc);
	Fields[guid] = pDesc;
}

void FieldList::CreateEmptyColorField (dword guid, const char* Name, const char* EditorName, DWORD def_value)
{
	DataColor* Field = NEW DataColor(this);
	Field->SetName(Name, EditorName);
	ColorVertex defVal[2];
	defVal[0].MinValue = dword(def_value);
	defVal[0].MaxValue = dword(def_value);
	defVal[1].MinValue = dword(def_value);
	defVal[1].MaxValue = dword(def_value);
	Field->SetValues(defVal, 2);

	FieldDesc pDesc;
	pDesc.dwGUID = guid;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.szEditorName = EditorName;
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_COLOR;
	//Fields.Add(pDesc);
	Fields[guid] = pDesc;
}


void FieldList::CreateBoolField (MemFile* pMemFile, DataDescripion* pDataDescriptor)
{
	DataBool* Field = NEW DataBool(this);
	Field->Load(pMemFile);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_BOOL;
	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;
	} else
	{
		delete Field;
	}


	//Fields.Add(pDesc);
	
}

void FieldList::CreateFloatField (MemFile* pMemFile, DataDescripion* pDataDescriptor)
{
	DataFloat* Field = NEW DataFloat(this);
	Field->Load(pMemFile);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_FLOAT;
	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;
	} else
	{
		delete Field;
	}

	//Fields.Add(pDesc);
}

void FieldList::CreateGraphField (MemFile* pMemFile, DataDescripion* pDataDescriptor)
{
	//DataGraph* Field = NEW DataGraph(this);
	DataGraph* Field = PService->AllocateDataGraph(this);
	Field->Load(pMemFile);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_GRAPH;
	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;
	} else
	{
		delete Field;
	}


	//Fields.Add(pDesc);
}

void FieldList::CreatePositionField (MemFile* pMemFile, DataDescripion* pDataDescriptor)
{
	DataPosition* Field = NEW DataPosition(this);
	Field->Load(pMemFile);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_POSITION;
	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;
	} else
	{
		delete Field;
	}


	//Fields.Add(pDesc);
}

void FieldList::CreateStringField (MemFile* pMemFile, DataDescripion* pDataDescriptor)
{
	DataString* Field = NEW DataString(this);
	Field->Load(pMemFile);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_STRING;
	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;
	} else
	{
		delete Field;
	}


	//Fields.Add(pDesc);
}

void FieldList::CreateUVField (MemFile* pMemFile, DataDescripion* pDataDescriptor)
{
	DataUV* Field = NEW DataUV(this);
	Field->Load(pMemFile);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_UV;
	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;
	} else
	{
		delete Field;
	}


	//Fields.Add(pDesc);
}

void FieldList::CreateColorField (MemFile* pMemFile, DataDescripion* pDataDescriptor)
{
	DataColor* Field = NEW DataColor(this);
	Field->Load(pMemFile);

	FieldDesc pDesc;
	pDesc.MarkForDelete = false;
	pDesc.szName = Field->GetName();
	pDesc.HashValue = string::HashNoCase(pDesc.szName);
	pDesc.pPointer = Field;
	pDesc.Type = FIELD_COLOR;
	dword dwGUID = GetFieldGUIDByName(pDesc.szName, pDataDescriptor);
	if (dwGUID != 0xFFFFFFFF)
	{
		Fields[dwGUID] = pDesc;
	} else
	{
		delete Field;
	}


	//Fields.Add(pDesc);
}

void FieldList::DelAll ()
{
	for (DWORD n = 0; n < GUID_MAX; n++)
	{
		if (Fields[n].Type == FIELD_UNKNOWN) continue;
		DeleteFieldData (Fields[n]);
		Fields[n].Type = FIELD_UNKNOWN;
	} // loop

	//Fields.DelAll();
}



DataColor* FieldList::FindColorByGUID (dword GUID)
{
	if (GUID >= GUID_MAX) return NULL;

	if (Fields[GUID].Type != FIELD_COLOR)
	{
		return NULL;
	}

	return ((DataColor*)Fields[GUID].pPointer);

/*
	for (DWORD n = 0; n < Fields.Size(); n++)
	{
		if (Fields[n].Type == FIELD_COLOR)
		{
			if (GUID == Fields[n].dwGUID)
			{
					return ((DataColor*)Fields[n].pPointer);
			}
		}
	}

	return NULL;
*/
}


/*
DataColor* FieldList::FindColor (const char* AttrName)
{
	DWORD SearchHash = string::HashNoCase(AttrName);
	for (DWORD n = 0; n < GUID_MAX; n++)
	{
		if (Fields[n].Type == FIELD_COLOR)
		{
			if (SearchHash == Fields[n].HashValue)
			{
				if (crt_stricmp (Fields[n].szName, AttrName) == 0)
				{
					return ((DataColor*)Fields[n].pPointer);
				}
			}
		}
	}

	return NULL;
}
*/



DataBool* FieldList::FindBoolByGUID (dword GUID)
{
	if (GUID >= GUID_MAX) return NULL;

	if (Fields[GUID].Type != FIELD_BOOL)
	{
		return NULL;
	}

	return ((DataBool*)Fields[GUID].pPointer);

/*
	for (DWORD n = 0; n < Fields.Size(); n++)
	{
		if (Fields[n].Type == FIELD_BOOL)
		{
			if (GUID == Fields[n].dwGUID)
			{
					return ((DataBool*)Fields[n].pPointer);
			}
		}
	}

	return NULL;
*/
}

/*
DataBool* FieldList::FindBool (const char* AttrName)
{
	DWORD SearchHash = string::HashNoCase(AttrName);
	for (DWORD n = 0; n < GUID_MAX; n++)
	{
		if (Fields[n].Type == FIELD_BOOL)
		{
			if (SearchHash == Fields[n].HashValue)
			{
				if (crt_stricmp (Fields[n].szName, AttrName) == 0)
				{
					return ((DataBool*)Fields[n].pPointer);
				}
			}
		}
	}

	return NULL;
}

*/

DataFloat* FieldList::FindFloatByGUID (dword GUID)
{
	if (GUID >= GUID_MAX) return NULL;

	if (Fields[GUID].Type != FIELD_FLOAT)
	{
		return NULL;
	}

	return ((DataFloat*)Fields[GUID].pPointer);

/*
	for (DWORD n = 0; n < Fields.Size(); n++)
	{
		if (Fields[n].Type == FIELD_FLOAT)
		{
			if (GUID == Fields[n].dwGUID)
			{
					return ((DataFloat*)Fields[n].pPointer);
			}
		}
	}

	return NULL;
*/
}


/*
DataFloat* FieldList::FindFloat (const char* AttrName)
{
	DWORD SearchHash = string::HashNoCase(AttrName);
	for (DWORD n = 0; n < GUID_MAX; n++)
	{
		if (Fields[n].Type == FIELD_FLOAT)
		{
			if (SearchHash == Fields[n].HashValue)
			{
				if (crt_stricmp (Fields[n].szName, AttrName) == 0)
				{
					return ((DataFloat*)Fields[n].pPointer);
				}
			}
		}
	}

	return NULL;
}
*/


DataGraph* FieldList::FindGraphByGUID (dword GUID)
{
	if (GUID >= GUID_MAX) return NULL;

	if (Fields[GUID].Type != FIELD_GRAPH)
	{
		return NULL;
	}

	return ((DataGraph*)Fields[GUID].pPointer);

/*
	for (DWORD n = 0; n < Fields.Size(); n++)
	{
		if (Fields[n].Type == FIELD_GRAPH)
		{
			if (GUID == Fields[n].dwGUID)
			{
					return ((DataGraph*)Fields[n].pPointer);
			}
		}
	}

	return NULL;
*/
}


/*
DataGraph* FieldList::FindGraph (const char* AttrName)
{
	DWORD SearchHash = string::HashNoCase(AttrName);

	for (DWORD n = 0; n < GUID_MAX; n++)
	{
		if (Fields[n].Type == FIELD_GRAPH)
		{
			if (SearchHash == Fields[n].HashValue)
			{
				if (crt_stricmp (Fields[n].szName, AttrName) == 0)
				{
					return ((DataGraph*)Fields[n].pPointer);
				}
			}
		}
	}

	return NULL;
}
*/

DataString* FieldList::FindStringByGUID (dword GUID)
{
	if (GUID >= GUID_MAX) return NULL;

	if (Fields[GUID].Type != FIELD_STRING)
	{
		return NULL;
	}

	return ((DataString*)Fields[GUID].pPointer);

/*
	for (DWORD n = 0; n < Fields.Size(); n++)
	{
		if (Fields[n].Type == FIELD_STRING)
		{
			if (GUID == Fields[n].dwGUID)
			{
					return ((DataString*)Fields[n].pPointer);
			}
		}
	}

	return NULL;
*/
}

/*

DataString* FieldList::FindString (const char* AttrName)
{
	DWORD SearchHash = string::HashNoCase(AttrName);

	for (DWORD n = 0; n < GUID_MAX; n++)
	{
		if (Fields[n].Type == FIELD_STRING)
		{
			if (SearchHash == Fields[n].HashValue)
			{
				if (crt_stricmp (Fields[n].szName, AttrName) == 0)
				{
					return ((DataString*)Fields[n].pPointer);
				}
			}
		}
	}

	return NULL;
}
*/

DataPosition* FieldList::FindPositionByGUID (dword GUID)
{
	if (GUID >= GUID_MAX) return NULL;

	if (Fields[GUID].Type != FIELD_POSITION)
	{
		return NULL;
	}

	return ((DataPosition*)Fields[GUID].pPointer);

/*
	for (DWORD n = 0; n < Fields.Size(); n++)
	{
		if (Fields[n].Type == FIELD_POSITION)
		{
			if (GUID == Fields[n].dwGUID)
			{
					return ((DataPosition*)Fields[n].pPointer);
			}
		}
	}

	return NULL;
*/
}


/*
DataPosition* FieldList::FindPosition (const char* AttrName)
{
	DWORD SearchHash = string::HashNoCase(AttrName);

	for (DWORD n = 0; n < GUID_MAX; n++)
	{
		if (Fields[n].Type == FIELD_POSITION)
		{
			if (SearchHash == Fields[n].HashValue)
			{
				if (crt_stricmp (Fields[n].szName, AttrName) == 0)
				{
					return ((DataPosition*)Fields[n].pPointer);
				}
			}
		}
	}

	return NULL;
}
*/

DataUV* FieldList::FindUVByGUID (dword GUID)
{
	if (GUID >= GUID_MAX) return NULL;

	if (Fields[GUID].Type != FIELD_UV)
	{
		return NULL;
	}

	return ((DataUV*)Fields[GUID].pPointer);

/*
	for (DWORD n = 0; n < Fields.Size(); n++)
	{
		if (Fields[n].Type == FIELD_UV)
		{
			if (GUID == Fields[n].dwGUID)
			{
					return ((DataUV*)Fields[n].pPointer);
			}
		}
	}
	return NULL;
*/
}


/*
DataUV* FieldList::FindUV (const char* AttrName)
{
	DWORD SearchHash = string::HashNoCase(AttrName);

	for (DWORD n = 0; n < Fields.Size(); n++)
	{
		if (Fields[n].Type == FIELD_UV)
		{
			if (SearchHash == Fields[n].HashValue)
			{
				if (crt_stricmp (Fields[n].szName, AttrName) == 0)
				{
					return ((DataUV*)Fields[n].pPointer);
				}
			}
		}
	}
	return NULL;
}
*/

/*
FieldList::FieldDesc* FieldList::FindField (const char* Name)
{
	DWORD SearchHash = string::HashNoCase(Name);

	for (DWORD n = 0; n < Fields.Size(); n++)
	{
			if (SearchHash == Fields[n].HashValue)
			{
				if (crt_stricmp (Fields[n].szName, Name) == 0)
				{
					return &Fields[n];
				}
			}
	}
	return NULL;
}
*/

float FieldList::GetFloat (dword dwGUID, float def_value)
{
	DataFloat* pFind = FindFloatByGUID(dwGUID);
	if (!pFind) return def_value;
	return pFind->GetValue();
}

int FieldList::GetFloatAsInt (dword dwGUID, int def_value)
{
	float val = GetFloat(dwGUID, (float)def_value);
	return (int)val;
}


bool FieldList::GetBool (dword dwGUID, bool def_value)
{
	DataBool* pFind = FindBoolByGUID(dwGUID);
	if (!pFind) return def_value;
	return pFind->GetValue();
}

const char* FieldList::GetString (dword dwGUID, const char* def_value)
{
	DataString* pFind = FindStringByGUID(dwGUID);
	if (!pFind) return def_value;
	return pFind->GetValue();
}

const Vector& FieldList::GetPosition (dword dwGUID, const Vector& def_value)
{
	DataPosition* pFind = FindPositionByGUID(dwGUID);
	if (!pFind) return def_value;
	return pFind->GetValue();
}

float FieldList::GetGraphVal (dword dwGUID, float Time, float LifeTime, float K_Rand, float def_value)
{
	DataGraph* pFind = FindGraphByGUID(dwGUID);
	if (!pFind) return def_value;
	return pFind->GetValue(Time, LifeTime, K_Rand);
}

float FieldList::GetRandomGraphVal (dword dwGUID, float Time, float LifeTime, float def_value)
{
	DataGraph* pFind = FindGraphByGUID(dwGUID);
	if (!pFind) return def_value;
	return pFind->GetRandomValue(Time, LifeTime);
}

DWORD FieldList::_GetFieldCount ()
{
	return GUID_MAX;
}

const FieldList::FieldDesc& FieldList::_GetFieldByIndex (DWORD Index)
{
	return Fields[Index];
}

dword FieldList::GetFieldGUIDByName (const char* name, DataDescripion* pDataDescriptor)
{
	DWORD NeedFieldsCount = pDataDescriptor->GetFieldCount();

	for (DWORD n = 0; n < NeedFieldsCount; n++)
	{
		const char* NeedFieldName = pDataDescriptor->GetFieldName(n);

		if (crt_stricmp(NeedFieldName, name) == 0)
		{
			dword dwFieldGUID = pDataDescriptor->GetFieldGUID(n);
			return dwFieldGUID;
		}
	}

	return 0xFFFFFFFF;

}

void FieldList::Convert (DataDescripion* pDataDescriptor)
{
	for (DWORD i = 0; i < GUID_MAX; i++)
	{
		Fields[i].MarkForDelete = true;
	}

	DWORD NeedFieldsCount = pDataDescriptor->GetFieldCount();

	for (DWORD n = 0; n < NeedFieldsCount; n++)
	{
		dword dwFieldGUID = pDataDescriptor->GetFieldGUID(n);

		const char* szEditorName = pDataDescriptor->GetFieldEditorName(n);
		const char* NeedFieldName = pDataDescriptor->GetFieldName(n);
		FieldType NeedFieldType = pDataDescriptor->GetFieldType(n);
		//FieldDesc* pDesc = FindField (NeedFieldName);
		
		FieldDesc* pDesc = NULL;
		if (dwFieldGUID < GUID_MAX)
		{
			if (Fields[dwFieldGUID].Type != FIELD_UNKNOWN)
			{
				pDesc = &Fields[dwFieldGUID];
			}
		}
		


		bool FieldExist = false;
		if (pDesc)
		{
			if (pDesc->Type == NeedFieldType)
			{
				//Если нашли поле и оно нужного типа, убираем пометку об удалении
				pDesc->MarkForDelete = false;
				FieldExist = true;
				pDesc->szEditorName = szEditorName;
				pDesc->dwGUID = dwFieldGUID;


				switch (NeedFieldType)
				{
				case FIELD_COLOR:
					((DataColor*)pDesc->pPointer)->SetName(NeedFieldName, szEditorName);
					break;
				case FIELD_BOOL:
					((DataBool*)pDesc->pPointer)->SetName(NeedFieldName, szEditorName);
					break;
				case FIELD_FLOAT:
					((DataFloat*)pDesc->pPointer)->SetName(NeedFieldName, szEditorName);
					break;
				case FIELD_GRAPH:
					((DataGraph*)pDesc->pPointer)->SetName(NeedFieldName, szEditorName);
					break;
				case FIELD_POSITION:
					((DataPosition*)pDesc->pPointer)->SetName(NeedFieldName, szEditorName);
					break;
				case FIELD_UV:
					((DataUV*)pDesc->pPointer)->SetName(NeedFieldName, szEditorName);
					break;
				case FIELD_STRING:
					((DataString*)pDesc->pPointer)->SetName(NeedFieldName, szEditorName);
					break;
				default:
					throw ("Unknown Field type !!!");
				}

				
			}
		}

		//Если поле не существует, надо его создать...
		if (!FieldExist)
		{
			switch (NeedFieldType)
			{
			case FIELD_COLOR:
				CreateEmptyColorField(dwFieldGUID, NeedFieldName, szEditorName, 0xFFFFA030);
				break;
			case FIELD_BOOL:
				CreateEmptyBoolField(dwFieldGUID, NeedFieldName, szEditorName, false);
				break;
			case FIELD_FLOAT:
				CreateEmptyFloatField(dwFieldGUID, NeedFieldName, szEditorName, 0.0f);
				break;
			case FIELD_GRAPH:
				CreateEmptyGraphField(dwFieldGUID, NeedFieldName, szEditorName, 0.0f, 0.0f);
				break;
			case FIELD_POSITION:
				CreateEmptyPositionField(dwFieldGUID, NeedFieldName, szEditorName, Vector(0.0f));
				break;
			case FIELD_UV:
				CreateEmptyUVField(dwFieldGUID, NeedFieldName, szEditorName);
				break;
			case FIELD_STRING:
				CreateEmptyStringField(dwFieldGUID, NeedFieldName, szEditorName, "#empty#");
				break;
			default:
				throw ("Unknown Field type !!!");
			}
		}
	}

	for (i = 0; i < GUID_MAX; i++)
	{
		if (Fields[i].MarkForDelete)
		{
			if (Fields[i].Type == FIELD_UNKNOWN) continue;

			DeleteFieldData (Fields[i]);
			Fields[i].Type = FIELD_UNKNOWN;
			//Fields.ExtractNoShift(i);
			//i--;
		}
	}

}

void FieldList::DeleteFieldData (const FieldList::FieldDesc& pData)
{
	FieldType fldType = pData.Type;

	switch (fldType)
	{
	case FIELD_BOOL:
		{
			delete ((DataBool*)pData.pPointer);
			break;
		}
	case FIELD_FLOAT:
		{
			delete ((DataFloat*)pData.pPointer);
			break;
		}
	case FIELD_GRAPH:
		{
			PService->FreeDataGraph((DataGraph*)pData.pPointer);
			//delete ((DataGraph*)pData.pPointer);
			break;
		}
	case FIELD_POSITION:
		{
			delete ((DataPosition*)pData.pPointer);
			break;
		}
	case FIELD_STRING:
		{
			delete ((DataString*)pData.pPointer);
			break;
		}
	case FIELD_UV:
		{
			delete ((DataUV*)pData.pPointer);
			break;
		}
	case FIELD_COLOR:
		{
			delete ((DataColor*)pData.pPointer);
			break;
		}
	default:
		{
			throw ("Particles: Try delete unknown field type !!!!");
		}
	}	//switch
}


#ifndef _XBOX
void FieldList::WriteXML (TextFile* xmlFile, dword level)
{

	xmlFile->Write((level+1), "<Fields>\n");
	level++;


	for (DWORD n = 0; n < GUID_MAX; n++)
	{

		if (Fields[n].Type == FIELD_UNKNOWN) continue;



		switch (Fields[n].Type)
		{
		case FIELD_BOOL:
			{
				xmlFile->Write((level+1), "<Field val = \"FIELD_BOOL\">\n");

				DataBool* pBoolField = (DataBool*)Fields[n].pPointer;
				pBoolField->WriteXML (xmlFile, (level+1));
				break;
			}
		case FIELD_FLOAT:
			{
				xmlFile->Write((level+1), "<Field val = \"FIELD_FLOAT\">\n");

				DataFloat* pFloatField = (DataFloat*)Fields[n].pPointer;
				pFloatField->WriteXML (xmlFile, (level+1));
				break;
			}
		case FIELD_GRAPH:
			{
				xmlFile->Write((level+1), "<Field val = \"FIELD_GRAPH\">\n");

				DataGraph* pGraphField = (DataGraph*)Fields[n].pPointer;
				pGraphField->WriteXML (xmlFile, (level+1));
				break;
			}
		case FIELD_POSITION:
			{
				xmlFile->Write((level+1), "<Field val = \"FIELD_POSITION\">\n");

				DataPosition* pPositionField = (DataPosition*)Fields[n].pPointer;
				pPositionField->WriteXML (xmlFile, (level+1));
				break;
			}
		case FIELD_STRING:
			{
				xmlFile->Write((level+1), "<Field val = \"FIELD_STRING\">\n");

				DataString* pStringField = (DataString*)Fields[n].pPointer;
				pStringField->WriteXML (xmlFile, (level+1));
				break;
			}
		case FIELD_UV:
			{
				xmlFile->Write((level+1), "<Field val = \"FIELD_UV\">\n");

				DataUV* pUVField = (DataUV*)Fields[n].pPointer;
				pUVField->WriteXML (xmlFile, (level+1));
				break;
			}
		case FIELD_COLOR:
			{
				xmlFile->Write((level+1), "<Field val = \"FIELD_COLOR\">\n");

				DataColor* pColorField = (DataColor*)Fields[n].pPointer;
				pColorField->WriteXML (xmlFile, (level+1));
				break;
			}
		default:
			{
				throw ("Particles: Unknown field type !!!!");
			}
		}	//switch


		xmlFile->Write((level+1), "</Field>\n");


	} // loop


	xmlFile->Write((level+0), "</Fields>\n");

}
#endif


void FieldList::Write (MemFile* File)
{
	DWORD DataFieldsCount = 0;
	for (dword n = 0; n < GUID_MAX; n++)
	{
		if (Fields[n].Type != FIELD_UNKNOWN)
		{
			DataFieldsCount++;
		}
	}

	
	File->WriteType(DataFieldsCount);



	for (DWORD n = 0; n < GUID_MAX; n++)
	{
		if (Fields[n].Type == FIELD_UNKNOWN) continue;

		File->WriteType(Fields[n].Type);

		switch (Fields[n].Type)
		{
			case FIELD_BOOL:
				{
					DataBool* pBoolField = (DataBool*)Fields[n].pPointer;
					pBoolField->Write (File);
					break;
				}
			case FIELD_FLOAT:
				{
					DataFloat* pFloatField = (DataFloat*)Fields[n].pPointer;
					pFloatField->Write (File);
					break;
				}
			case FIELD_GRAPH:
				{
					DataGraph* pGraphField = (DataGraph*)Fields[n].pPointer;
					pGraphField->Write (File);
					break;
				}
			case FIELD_POSITION:
				{
					DataPosition* pPositionField = (DataPosition*)Fields[n].pPointer;
					pPositionField->Write (File);
					break;
				}
			case FIELD_STRING:
				{
					DataString* pStringField = (DataString*)Fields[n].pPointer;
					pStringField->Write (File);
					break;
				}
			case FIELD_UV:
				{
					DataUV* pUVField = (DataUV*)Fields[n].pPointer;
					pUVField->Write (File);
					break;
				}
			case FIELD_COLOR:
				{
					DataColor* pColorField = (DataColor*)Fields[n].pPointer;
					pColorField->Write (File);
					break;
				}
			default:
				{
					throw ("Particles: Unknown field type !!!!");
				}
		}	//switch
	} // loop

}


void FieldList::UpdateCache ()
{
	CachedGeomNames.DelAll();
	const char* GeomNames = GetString(GUID_PARTICLE_GEOM_NAMES);

	GeomNameParser Parser;
	Parser.Tokenize(GeomNames);

	for (dword i = 0; i < Parser.GetTokensCount(); i++)
	{
		CachedGeomNames.Add(Parser.GetTokenByIndex(i));
	}

	CachedForceFieldsID.DelAll();
	if (pMaster)
	{
		const char* ForceFields = GetString(GUID_FORCEFIELDS_NAME);
		//api->Trace("ff: '%s'", ForceFields);
		Parser.Tokenize(ForceFields);

		if (Parser.GetTokensCount() > 0)
		{
			for (dword z = 0; z < Parser.GetTokensCount(); z++)
			{
				const char* szForceFiledName = Parser.GetTokenByIndex(z);
				if (szForceFiledName && szForceFiledName[0] == '#') continue;
				//dword dwForceFieldGUID = pManager->GetForceFieldUniqueID(szForceFiledName);
				//CachedForceFieldsID.Add(dwForceFieldGUID);
				Assert(!"dword dwForceFieldGUID = pManager->GetForceFieldUniqueID(szForceFiledName);");
				
			}
		}
	}
}

const char* FieldList::GetRandomModelNameFromCache()
{
	if (CachedGeomNames.Size() == 0) return NULL;

	return CachedGeomNames[(rand () % CachedGeomNames.Size())];
}

dword FieldList::GetForceFieldsCountFromCache ()
{
	return CachedForceFieldsID.Size();
}

dword FieldList::GetForceFieldGUIDFromCache(dword dwIndex)
{
	return CachedForceFieldsID[dwIndex];
}

