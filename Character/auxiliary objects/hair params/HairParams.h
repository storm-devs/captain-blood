#pragma once

#include "..\..\..\Common_h\Mission.h"

class HairParams : public MissionObject
{
public:

	struct HairLine
	{
		float k;
		float m;

		array<ConstString> bones;

		float n;

		HairLine() : bones(_FL_)
		{
		}
	};

	struct HairData
	{
		array<HairLine> lines;

		HairData() : lines(_FL_)
		{
		}
	};

	struct CollData
	{
		ConstString a;
		ConstString b;
		float x; 
		float r; 
		Vector p; 
		dword c;
	};

	struct CollList
	{
		array<CollData> list;

		CollList() : list(_FL_)
		{
		}
	};

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

private:

	void InitParams		(MOPReader &reader);

public:

//	float m_k;
//	float m_m;

	HairData hair;
	CollList coll;

	bool updated;

	MO_IS_FUNCTION(HairParams, MissionObject);

	static const char *comment;

};
