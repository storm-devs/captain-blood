#include "StainKiller.h"

#include "..\..\..\Common_h\IStainManager.h"

StainKiller:: StainKiller()
{
	m_rad = 1.0f;
	m_man = null;

	m_debug = false;
}

StainKiller::~StainKiller()
{
}

bool StainKiller::Create		 (MOPReader &reader)
{
	InitParams(reader);

	if( !m_man )
	{
		MOSafePointer sp;

		static const ConstString objectId("StainManager");
		Mission().CreateObject(sp,"StainManager",objectId);

		m_man = (IStainManager *)sp.Ptr();
		Assert(m_man)
	}

	return true;
}

bool StainKiller::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void StainKiller::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( EditMode_IsOn() || m_debug )
	{
		if( isShow )
			SetUpdate(&StainKiller::Draw,ML_ALPHA1);
		else
			DelUpdate(&StainKiller::Draw);
	}
}

void StainKiller::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	CheckStains();
}

void StainKiller::CheckStains()
{
	if( m_man )
		m_man->ClearRad(m_pos,m_rad,m_time);
}

void _cdecl StainKiller::Draw(float dltTime, long level)
{
	if( !EditMode_IsOn() || Mission().EditMode_IsAdditionalDraw())
	{
	//	Render().DrawSphere(m_pos,m_rad,0x800000ff);
		Render().DrawSphereGizmo(m_pos,m_rad,-1,-1);
	}
}

void StainKiller::Command(const char *id, dword numParams, const char **params)
{
	//
}

void StainKiller::InitParams(MOPReader &reader)
{
	m_pos = reader.Position();
	m_rad = reader.Float();

	m_time = reader.Float();

	m_debug = reader.Bool();

	Show	(reader.Bool());
	Activate(reader.Bool());
}

MOP_BEGINLISTCG(StainKiller, "StainKiller", '1.00', 0, "StainKiller", "Default")

	MOP_POSITION("Pos", 0.0f);
	MOP_FLOAT	("Rad", 1.0f);

	MOP_FLOAT("Time", 0.0f);

	MOP_BOOL("Debug draw", false);

	MOP_BOOL("Show"	 , true);
	MOP_BOOL("Active", true);

MOP_ENDLIST(StainKiller)
