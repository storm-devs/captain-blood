#include  "DebugInfoShower.h"

void DebugInfoShower::InfoGroup::Reset()
{
	aInfo.DelAll();
}

void DebugInfoShower::InfoGroup::AddInfo(const InfoCommon & info)
{
	aInfo.Add( info );
}


DebugInfoShower::DebugInfoShower() :
	m_aGroup( _FL_ )
{
	m_dwCounter = 0;
}

DebugInfoShower::~DebugInfoShower()
{
	m_aGroup.DelAll();
	m_dwCounter = 0;
}

bool DebugInfoShower::Create(MOPReader & reader)
{
	SetUpdate(&DebugInfoShower::Work, ML_DEBUG);
	return true;
}

void _cdecl DebugInfoShower::Work(float fDeltaTime, long level)
{
	// перейдем к новому кадру
	m_dwCounter++;

	// выведем сохраненную дебаг инфу
	for( long n=0; n<m_aGroup; n++ )
		ShowGroup(n);
}

void DebugInfoShower::AddInfo(const char* pcGroupID, InfoType type, const InfoCommon & info)
{
	long n = CreateGroup( pcGroupID, type );
	if( n<0 ) return;
	// обновление на новом кадре? тогда сбрасываем старые данные
	if( m_aGroup[n].dwLastCounter != m_dwCounter )
	{
		m_aGroup[n].Reset();
		m_aGroup[n].dwLastCounter = m_dwCounter;
	}
	m_aGroup[n].AddInfo( info );
}

long DebugInfoShower::FindGroup(const char* pcGroupID, InfoType type)
{
	for( long n=0; n<m_aGroup; n++ )
		if( m_aGroup[n].sID == pcGroupID && m_aGroup[n].type == type )
			return n;
	return -1;
}

long DebugInfoShower::CreateGroup(const char* pcGroupID, InfoType type)
{
	long n = FindGroup(pcGroupID, type);
	if( n>=0 ) return n;
	n = m_aGroup.Add();
	m_aGroup[n].sID = pcGroupID;
	m_aGroup[n].type = type;
	m_aGroup[n].dwLastCounter = m_dwCounter;
	return n;
}

void DebugInfoShower::ShowGroup(long ngrp)
{
	if( ngrp<0 || ngrp>=m_aGroup )
		return;
	switch( m_aGroup[ngrp].type )
	{
	case it_line: ShowLines( m_aGroup[ngrp].aInfo ); break;
	case it_sphere: ShowSpheres( m_aGroup[ngrp].aInfo ); break;
	case it_poligon4: ShowPoligons4( m_aGroup[ngrp].aInfo ); break;
	}
}

void DebugInfoShower::ShowLines(array<InfoCommon> & aInfo)
{
	for( long n=0; n<aInfo; n++ )
	{
		Render().DrawLine( aInfo[n].src, aInfo[n].col, aInfo[n].dst, aInfo[n].col );
	}
}

void DebugInfoShower::ShowSpheres(array<InfoCommon> & aInfo)
{
	for( long n=0; n<aInfo; n++ )
	{
		Render().DrawSphere( aInfo[n].centr, aInfo[n].rad, aInfo[n].col );
	}
}

void DebugInfoShower::ShowPoligons4(array<InfoCommon> & aInfo)
{
	for( long n=0; n<aInfo; n++ )
	{
		Render().DrawPolygon( aInfo[n].v4, 4, Color(aInfo[n].col) );
	}
}

DebugInfoShower* DebugInfoShower::FindShower(MissionObject* mo)
{
	if( !mo ) return NULL;
	MOSafePointer safeptr;
	static const ConstString id_DebugInfoShower("DebugInfoShower");
	mo->FindObject( id_DebugInfoShower, safeptr );
	if( !safeptr.Ptr() || !safeptr.Ptr()->Is(id_DebugInfoShower) ) return NULL;
	return (DebugInfoShower*)safeptr.Ptr();
}

void DebugInfoShower::AddPoligon4(MissionObject* mo, const char* grpID, const char* addictID, dword col, Vector& v1, Vector& v2, Vector& v3, Vector& v4)
{
	DebugInfoShower* pmo = FindShower(mo);
	if( pmo )
	{
		string sID = grpID;
		sID += addictID;
		InfoCommon info;
		info.col = col;
		info.v4[0] = v1;
		info.v4[1] = v2;
		info.v4[2] = v3;
		info.v4[3] = v4;
		pmo->AddInfo(sID,it_poligon4,info);
	}
}

void DebugInfoShower::AddLine(MissionObject* mo, const char* grpID, const char* addictID, dword col, Vector& src, Vector& dst)
{
	DebugInfoShower* pmo = FindShower(mo);
	if( pmo )
	{
		string sID = grpID;
		sID += addictID;
		InfoCommon info;
		info.col = col;
		info.src = src;
		info.dst = dst;
		pmo->AddInfo(sID,it_line,info);
	}
}

void DebugInfoShower::AddSphere(MissionObject* mo, const char* grpID, const char* addictID, dword col, Vector& center, float rad)
{
	DebugInfoShower* pmo = FindShower(mo);
	if( pmo )
	{
		string sID = grpID;
		sID += addictID;
		InfoCommon info;
		info.col = col;
		info.centr = center;
		info.rad = rad;
		pmo->AddInfo(sID,it_sphere,info);
	}
}

MOP_BEGINLISTCG(DebugInfoShower, "DebugInfoShower", '1.00', 100, "Object for collect and output debug info", "Arcade Sea");
MOP_ENDLIST(DebugInfoShower)
