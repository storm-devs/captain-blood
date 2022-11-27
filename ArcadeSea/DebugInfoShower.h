#pragma once
#include "..\common_h\mission.h"

class DebugInfoShower : public MissionObject
{
public:
	// типы дебажной инфы
	enum InfoType
	{
		it_line,
		it_sphere,
		it_poligon4
	};

	// общий тип (для локального хранения и для передачи параметров)
	struct InfoCommon
	{
		dword col;
		union
		{
			// тип линия
			struct
			{
				Vector src;
				Vector dst;
			};
			// тип сфера
			struct
			{
				Vector centr;
				float rad;
			};
			// тип полигон 4 вершины
			struct
			{
				Vector v4[4];
			};
		};

		//Empty constructor
		InfoCommon() {}
		//Assign
		InfoCommon& InfoCommon::operator =(const InfoCommon& info)
		{
			col = info.col;
			// тип линия
			src = info.src;
			dst = info.dst;
			// тип сфера
			centr = info.centr;
			rad = info.rad;
			// тип полигон 4 вершины
			v4[0] = info.v4[0];
			v4[1] = info.v4[1];
			v4[2] = info.v4[2];
			v4[3] = info.v4[3];

			return *this;
		}
	};

	DebugInfoShower();
	~DebugInfoShower();

	virtual bool Create(MOPReader & reader);

	MO_IS_FUNCTION(DebugInfoShower, MissionObject);

	void _cdecl Work(float fDeltaTime, long level);

	void AddInfo(const char* pcGroupID, InfoType type, const InfoCommon & info);

protected:
	struct InfoGroup
	{
		dword dwLastCounter;
		string sID;
		InfoType type;
		array<InfoCommon> aInfo;

		InfoGroup() : aInfo(_FL_) {}
		void Reset();
		void AddInfo(const InfoCommon & info);
	};
	array<InfoGroup> m_aGroup;
	dword m_dwCounter;

	long FindGroup(const char* pcGroupID, InfoType type);
	long CreateGroup(const char* pcGroupID, InfoType type);

	void ShowGroup(long ngrp);
	void ShowLines(array<InfoCommon> & aInfo);
	void ShowSpheres(array<InfoCommon> & aInfo);
	void ShowPoligons4(array<InfoCommon> & aInfo);

public:
	static void AddPoligon4(MissionObject* mo, const char* grpID, const char* addictID, dword col, Vector& v1, Vector& v2, Vector& v3, Vector& v4);
	static void AddLine(MissionObject* mo, const char* grpID, const char* addictID, dword col, Vector& src, Vector& dst);
	static void AddSphere(MissionObject* mo, const char* grpID, const char* addictID, dword col, Vector& center, float rad);

protected:
	static DebugInfoShower* FindShower(MissionObject* mo);
};
