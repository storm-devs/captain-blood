#pragma once

#include "..\ships\ShipSide.h"

//////////////////////////////////////////////////////////////////////////
class WeaponPlatform : public MissionObject
{
public:
	WeaponPlatform();
	~WeaponPlatform();

    //
	MO_IS_FUNCTION(WeaponPlatform, MissionObject);

	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();
	// Рестарт объекта
	virtual void Restart();
	virtual bool EditMode_Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);

	virtual void Activate(bool isActive);

	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	virtual Matrix & GetMatrix(Matrix & mtx);
	void _cdecl Draw( float deltaTime, long level);
	void _cdecl EditDraw( float deltaTime, long level);

protected:
	void ReadParams(MOPReader&);
	void Release();
	bool ZoneIntersection( const Vector & pos, Vector & targ );
	void FireCannon(dword nSide, const Vector & vtarg);
	void ShootByTarget( MissionObject* pObj );
	void ShootByTarget( Vector& pos );
	void FireToZone( WeaponTargetZone& wzone );

	Matrix m_mTransform;
	struct Side
	{
		ShipSide* side;
		float fDirectionAngle;
		Vector vdir;

		Side() {side = null;}
		~Side() {if(side) {side->ClearWeapons(); delete side; side=null;} }
	};
	array<Side> m_aWeaponSide;

	MGIterator * m_pShipIterator;
	struct ShootZone
	{
		float fMinDist;
		float fMaxDist;
		float fMaxAng;
		float fCos_MaxAng;
		bool bShowDebug;
	} m_shootZone;

	bool m_bAutoTarget;

	bool m_bUseTargetPoint;
};
