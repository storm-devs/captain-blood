#ifndef _EDITOR_H_
#define _EDITOR_H_

#include "..\common_h\render.h"
#include "..\common_h\mission.h"
#include "..\common_h\gui.h"
#include "MissionEditorExport.h"



#define MAX_ENTITY_NAME 4096
#define MAXOBJECTPATH 16384


#define TAG_FOLDER 0
#define TAG_OBJECT 1
#define TAG_ATTRIBUTE 2
#define TAG_EVENT 3
#define TAG_ARRAYITEM 4


#define DISABLE 0
#define X_AXIS 1
#define Y_AXIS 2
#define Z_AXIS 3



class AttributeList;

extern IMission* miss;


class MissionEditor : public MissionEditorExport
{

protected:

public:





	// Имена ентитей являющихся объектами миссии
	struct tEntitysMO
	{
		string Name;
	};

	// Доступные для создания объекты миссии
	struct tAvailableMO
	{
		IMOParams* Params;
		AttributeList* AttrList;
		char Name[MAX_ENTITY_NAME];
		char ClassName[MAX_ENTITY_NAME];
		string Comment;
	};

	// Созданные объекты миссии
	struct tCreatedMO
	{
		bool bDeleted;
		string PathInTree; //Путь в дереве к объекту...
		AttributeList* AttrList;
		MOSafePointer pObject;
		long Level;
		dword Version;
		string ClassName;

		tCreatedMO()
		{
			bDeleted = false;
		}
	};


protected:

	array<tEntitysMO> EntitysMO;
	
	
	array<tCreatedMO> CreatedMO;
	array<tCreatedMO*> CreatedMOReal;



	array<string> importedMissions;

public:

	array<tAvailableMO> AvailableMO;


	array<string> & GetImportList();


	void CreateEntitysMOList ();
	void CreateAvailableMOList ();


	dword GetCreatedMissionObjectsCount ();
	MOSafePointer GetCreatedMissionObjectByIndex (dword dwIndex);
	MissionEditor::tCreatedMO& GetCreatedMissionObjectStructByIndex (dword dwIndex);
	MissionEditor::tCreatedMO& AddCreatedMissionObjectStruct();
	void DeleteCreatedMissionObject(dword dwIndex);
	void DeleteAllCreatedMissionObjects();


	long GetCreatedMissionObjectIndex (MOSafePointer obj);

	

	virtual ~MissionEditor();
	MissionEditor();
	bool    Init();
	
	void _fastcall Execute(float dltTime);



	const char* GetCommentForClassName (const char* szClassName);

	const char* GetCommentForName (const char* szTextName);
	


	MissionEditor::tCreatedMO* GetObjectByTreePath (const char* szTreePath);

	MissionEditor::tAvailableMO* GetAvailableClassByName (const char* szClassName);

	void _cdecl VSSLoginEntered (GUIControl* sender);


	void SetVSSAvailable (bool bEnabled);

	virtual void ConvertXML2MIS (const char* xmlName);


};

#endif