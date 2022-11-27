#pragma once
#include "..\..\common_h\mission.h"

class IslandsMap : public MissionObject
{
	// описание полей для отталкивания кораблей
	struct ColliderDescribe
	{
		Vector offset;
		float radius;
		float pushoffRadius;
		IPhysSphere* collider;

		ColliderDescribe();
		~ColliderDescribe();
		void ReadMOPs(IslandsMap* islandmap, const Matrix & transform, MOPReader & reader);
	};

	// описание объектов (острова)
	struct ObjectDescr
	{
		const char* gmxname;
		IGMXScene* geometry;
		IGMXScene* refrGeometry;
		IGMXScene* reflGeometry;

		Matrix transform;
		Matrix newtransform;

		array<ColliderDescribe> colliders;

		ObjectDescr();
		~ObjectDescr();

		void ReadMOPs(IslandsMap* islandmap, MOPReader & reader);
		void Init(MissionObject* islandmap);
	};

public:
	IslandsMap();
	~IslandsMap();

	virtual bool Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader) {ReadMOPs(reader); return true;}
	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);
	virtual void Show(bool isShow);

	MO_IS_FUNCTION(IslandsMap, MissionObject);

	void _cdecl Work(float fDeltaTime, long level);
	void _cdecl DrawRefr(const char*, MissionObject*);
	void _cdecl DrawRefl(const char*, MissionObject*);

	bool FindNearIsland(MissionObject* mo, Vector & pos);

protected:
	void ReadMOPs(MOPReader & reader);
	void DebugShow();
	void Safe_Fog();
	void Restore_Fog();
	void Set_Fog(float fDensityH, float fDensity);

private:
	array<ObjectDescr> m_Objects;
	bool m_bIsTeleported;
	bool m_bSeaRefraction;
	bool m_bSeaReflection;
	bool m_bNoSwing;

	bool m_bNotUseFogInRefraction;

	float h_density;
	float h_min;
	float h_max;
	float d_density;
	float d_min;
	float d_max;
	Color fog_color;

};
