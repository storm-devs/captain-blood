#include "ShipGUIUpdater.h"
#include "Ships\ship.h"

ShipGUIUpdater::ShipGUIUpdater()
{
	m_pShip.Reset();
}

bool ShipGUIUpdater::Create(MOPReader & reader)
{
	ReadMOPs(reader);

	if (!EditMode_IsOn())
	{
		// обновляемся перед GUI для того чтоб на отрисовках уже были новые данные
		SetUpdate(&ShipGUIUpdater::Work, ML_GUI1 - 1);
	}

	return true;
}

void ShipGUIUpdater::ReadMOPs(MOPReader & reader)
{
	long n,q;

	m_MOParams.pcLeftCannonProgress = reader.String();
	m_MOParams.pcRightCannonProgress = reader.String();
	m_MOParams.pcPowerShootProgress = reader.String();
	m_MOParams.pcFlamethrowerProgress = reader.String();
	m_MOParams.pcMineProgress = reader.String();

	m_MOParams.pcMineCountWidget = reader.String();
	q = reader.Array();
	m_MOParams.aMineSeparators.DelAll();
	m_MOParams.aMineSeparators.AddElements( q );
	for( n=0; n<q; n++ )
	{
		m_MOParams.aMineSeparators[n] = reader.Float();
	}

	// left side
	q = reader.Array();
	m_MOParams.aLQuadDamageEnableWidgets.DelAll();
	m_MOParams.aLQuadDamageEnableWidgets.AddElements( q );
	for( n=0; n<q; n++ )
		m_MOParams.aLQuadDamageEnableWidgets[n] = reader.String();

	q = reader.Array();
	m_MOParams.aLQuadDamageDisableWidgets.DelAll();
	m_MOParams.aLQuadDamageDisableWidgets.AddElements( q );
	for( n=0; n<q; n++ )
		m_MOParams.aLQuadDamageDisableWidgets[n] = reader.String();

	// right side
	q = reader.Array();
	m_MOParams.aRQuadDamageEnableWidgets.DelAll();
	m_MOParams.aRQuadDamageEnableWidgets.AddElements( q );
	for( n=0; n<q; n++ )
		m_MOParams.aRQuadDamageEnableWidgets[n] = reader.String();

	q = reader.Array();
	m_MOParams.aRQuadDamageDisableWidgets.DelAll();
	m_MOParams.aRQuadDamageDisableWidgets.AddElements( q );
	for( n=0; n<q; n++ )
		m_MOParams.aRQuadDamageDisableWidgets[n] = reader.String();

	q = reader.Array();
	m_MOParams.aFlamethrowerEnableWidgets.DelAll();
	m_MOParams.aFlamethrowerEnableWidgets.AddElements( q );
	for( n=0; n<q; n++ )
		m_MOParams.aFlamethrowerEnableWidgets[n] = reader.String();

	q = reader.Array();
	m_MOParams.aMineEnableWidgets.DelAll();
	m_MOParams.aMineEnableWidgets.AddElements( q );
	for( n=0; n<q; n++ )
		m_MOParams.aMineEnableWidgets[n] = reader.String();
}

void ShipGUIUpdater::PostCreate()
{
	static const ConstString playerId("Player");
	FindObject(playerId, m_pShip.GetSPObject());
	static const ConstString typeId("Ship");
	if( m_pShip.Ptr() && !m_pShip.Ptr()->Is(typeId) )
		m_pShip.Reset();
	if( !m_pShip.Ptr() )
		return;

	long n;

	FindObject( m_MOParams.pcLeftCannonProgress, m_GUIPointers.pLeftCannonProgress );
	FindObject( m_MOParams.pcRightCannonProgress, m_GUIPointers.pRightCannonProgress );
	FindObject( m_MOParams.pcFlamethrowerProgress, m_GUIPointers.pFlamethowerProgress );
	FindObject( m_MOParams.pcMineProgress, m_GUIPointers.pMineProgress );
	FindObject( m_MOParams.pcMineCountWidget, m_GUIPointers.pMineCount );
	FindObject( m_MOParams.pcPowerShootProgress, m_GUIPointers.pPowerShootProgress );

	// left side
	m_GUIPointers.aLQuadDamageEnableWidgets.DelAll();
	if( m_MOParams.aLQuadDamageEnableWidgets.Size() > 0 )
	{
		m_GUIPointers.aLQuadDamageEnableWidgets.AddElements( m_MOParams.aLQuadDamageEnableWidgets.Size() );
		for( n=0; n<m_GUIPointers.aLQuadDamageEnableWidgets; n++ )
			FindObject( m_MOParams.aLQuadDamageEnableWidgets[n], m_GUIPointers.aLQuadDamageEnableWidgets[n] );
	}
	m_GUIPointers.aLQuadDamageDisableWidgets.DelAll();
	if( m_MOParams.aLQuadDamageDisableWidgets.Size() > 0 )
	{
		m_GUIPointers.aLQuadDamageDisableWidgets.AddElements( m_MOParams.aLQuadDamageDisableWidgets.Size() );
		for( n=0; n<m_GUIPointers.aLQuadDamageDisableWidgets; n++ )
			FindObject( m_MOParams.aLQuadDamageDisableWidgets[n], m_GUIPointers.aLQuadDamageDisableWidgets[n] );
	}

	// right side
	m_GUIPointers.aRQuadDamageEnableWidgets.DelAll();
	if( m_MOParams.aRQuadDamageEnableWidgets.Size() > 0 )
	{
		m_GUIPointers.aRQuadDamageEnableWidgets.AddElements( m_MOParams.aRQuadDamageEnableWidgets.Size() );
		for( n=0; n<m_GUIPointers.aRQuadDamageEnableWidgets; n++ )
			FindObject( m_MOParams.aRQuadDamageEnableWidgets[n], m_GUIPointers.aRQuadDamageEnableWidgets[n] );
	}
	m_GUIPointers.aRQuadDamageDisableWidgets.DelAll();
	if( m_MOParams.aRQuadDamageDisableWidgets.Size() > 0 )
	{
		m_GUIPointers.aRQuadDamageDisableWidgets.AddElements( m_MOParams.aRQuadDamageDisableWidgets.Size() );
		for( n=0; n<m_GUIPointers.aRQuadDamageDisableWidgets; n++ )
			FindObject( m_MOParams.aRQuadDamageDisableWidgets[n], m_GUIPointers.aRQuadDamageDisableWidgets[n] );
	}

	m_GUIPointers.aFlamethrowerEnableWidgets.DelAll();
	if( m_MOParams.aFlamethrowerEnableWidgets.Size() > 0 )
	{
		m_GUIPointers.aFlamethrowerEnableWidgets.AddElements( m_MOParams.aFlamethrowerEnableWidgets.Size() );
		for( n=0; n<m_GUIPointers.aFlamethrowerEnableWidgets; n++ )
			FindObject( m_MOParams.aFlamethrowerEnableWidgets[n], m_GUIPointers.aFlamethrowerEnableWidgets[n] );
	}

	m_GUIPointers.aMineEnableWidgets.DelAll();
	if( m_MOParams.aMineEnableWidgets.Size() > 0 )
	{
		m_GUIPointers.aMineEnableWidgets.AddElements( m_MOParams.aMineEnableWidgets.Size() );
		for( n=0; n<m_GUIPointers.aMineEnableWidgets; n++ )
			FindObject( m_MOParams.aMineEnableWidgets[n], m_GUIPointers.aMineEnableWidgets[n] );
	}

	m_GUIPointers.bLQuadEnable = true;
	EnableObjects( m_GUIPointers.aLQuadDamageDisableWidgets, false );
	EnableObjects( m_GUIPointers.aLQuadDamageEnableWidgets, true );
	m_GUIPointers.bRQuadEnable = true;
	EnableObjects( m_GUIPointers.aRQuadDamageDisableWidgets, false );
	EnableObjects( m_GUIPointers.aRQuadDamageEnableWidgets, true );
}

void ShipGUIUpdater::EnableObjects(array<MOSafePointer> & aMO, bool bEnable)
{
	for( long n=0; n<aMO; n++ )
		if( aMO[n].Ptr() )
			aMO[n].Ptr()->Show( bEnable );
}

void _cdecl ShipGUIUpdater::Work(float fDeltaTime, long level)
{
	if( !m_pShip.Validate() )
		return;

	// получаем данные от корабля
	long n;
	float fLCannonProgress, fRCannonProgress, fFlamethrowerProgress, fMineProgress;
	char param[256];
	const char* pPar = param;
	m_pShip.Ptr()->GetWeaponSidesProgress( fLCannonProgress, fRCannonProgress, fFlamethrowerProgress, fMineProgress );

	long nMineQ = m_pShip.Ptr()->GetMineCount();

	float fPowerShootProgress = m_pShip.Ptr()->GetCannonPowerShootProgress();

	// left quad damage
	if( fLCannonProgress < 1.f )
	{
		if( m_GUIPointers.bLQuadEnable )
		{
			m_GUIPointers.bLQuadEnable = false;
			EnableObjects( m_GUIPointers.aLQuadDamageEnableWidgets, false );
			EnableObjects( m_GUIPointers.aLQuadDamageDisableWidgets, true );
		}
	}
	else
	{
		if( !m_GUIPointers.bLQuadEnable )
		{
			m_GUIPointers.bLQuadEnable = true;
			EnableObjects( m_GUIPointers.aLQuadDamageDisableWidgets, false );
			EnableObjects( m_GUIPointers.aLQuadDamageEnableWidgets, true );
		}
	}
	// right quad damage
	if( fRCannonProgress < 1.f )
	{
		if( m_GUIPointers.bRQuadEnable )
		{
			m_GUIPointers.bRQuadEnable = false;
			EnableObjects( m_GUIPointers.aRQuadDamageEnableWidgets, false );
			EnableObjects( m_GUIPointers.aRQuadDamageDisableWidgets, true );
		}
	}
	else
	{
		if( !m_GUIPointers.bRQuadEnable )
		{
			m_GUIPointers.bRQuadEnable = true;
			EnableObjects( m_GUIPointers.aRQuadDamageDisableWidgets, false );
			EnableObjects( m_GUIPointers.aRQuadDamageEnableWidgets, true );
		}
	}

	// flamethrower
	if( fFlamethrowerProgress < 1.f )
	{
		if( m_GUIPointers.bFlameEnable )
		{
			m_GUIPointers.bFlameEnable = false;
			for( n=0; n<m_GUIPointers.aFlamethrowerEnableWidgets; n++ )
				if( m_GUIPointers.aFlamethrowerEnableWidgets[n].Ptr() )
					m_GUIPointers.aFlamethrowerEnableWidgets[n].Ptr()->Show( false );
		}
	}
	else
	{
		if( !m_GUIPointers.bFlameEnable )
		{
			m_GUIPointers.bFlameEnable = true;
			for( n=0; n<m_GUIPointers.aFlamethrowerEnableWidgets; n++ )
				if( m_GUIPointers.aFlamethrowerEnableWidgets[n].Ptr() )
					m_GUIPointers.aFlamethrowerEnableWidgets[n].Ptr()->Show( true );
		}
	}
	// mine
	if( fMineProgress < 1.f )
	{
		if( m_GUIPointers.bMineEnable )
		{
			m_GUIPointers.bMineEnable = false;
			for( n=0; n<m_GUIPointers.aMineEnableWidgets; n++ )
				if( m_GUIPointers.aMineEnableWidgets[n].Ptr() )
					m_GUIPointers.aMineEnableWidgets[n].Ptr()->Show( false );
		}
	}
	else
	{
		if( !m_GUIPointers.bMineEnable )
		{
			m_GUIPointers.bMineEnable = true;
			for( n=0; n<m_GUIPointers.aMineEnableWidgets; n++ )
				if( m_GUIPointers.aMineEnableWidgets[n].Ptr() )
					m_GUIPointers.aMineEnableWidgets[n].Ptr()->Show( true );
		}
	}

	fLCannonProgress *= 10000.f;
	fRCannonProgress *= 10000.f;
	fFlamethrowerProgress *= 10000.f;
	fMineProgress *= 10000.f;
	fPowerShootProgress *= 10000.f;

	// ставим данные в GUI
	//----------------------------------------------------------------------
	if( m_GUIPointers.pLeftCannonProgress.Ptr() )
	{
		crt_snprintf(param,sizeof(param),"%f",fLCannonProgress);
		m_GUIPointers.pLeftCannonProgress.Ptr()->Command( "Set", 1, &pPar );
	}
	if( m_GUIPointers.pRightCannonProgress.Ptr() )
	{
		crt_snprintf(param,sizeof(param),"%f",fRCannonProgress);
		m_GUIPointers.pRightCannonProgress.Ptr()->Command( "Set", 1, &pPar );
	}

	if( m_GUIPointers.pFlamethowerProgress.Ptr() )
	{
		crt_snprintf(param,sizeof(param),"%f",fFlamethrowerProgress);
		m_GUIPointers.pFlamethowerProgress.Ptr()->Command( "Set", 1, &pPar );
	}
	if( m_GUIPointers.pMineProgress.Ptr() )
	{
		crt_snprintf(param,sizeof(param),"%f",fMineProgress);
		m_GUIPointers.pMineProgress.Ptr()->Command( "Set", 1, &pPar );
	}

	if( m_GUIPointers.pMineCount.Ptr() && m_MOParams.aMineSeparators.Size()>0 && nMineQ>=0 )
	{
		float fMineQ = (nMineQ >= m_MOParams.aMineSeparators) ? m_MOParams.aMineSeparators.LastE() : m_MOParams.aMineSeparators[nMineQ];
		crt_snprintf(param,sizeof(param),"%f",fMineQ);
		m_GUIPointers.pMineCount.Ptr()->Command( "Set", 1, &pPar );
	}

	if( m_GUIPointers.pPowerShootProgress.Ptr() )
	{
		crt_snprintf(param,sizeof(param),"%f",fPowerShootProgress);
		m_GUIPointers.pPowerShootProgress.Ptr()->Command( "Set", 1, &pPar );
	}
}

MOP_BEGINLISTCG(ShipGUIUpdater, "ShipGUIUpdater", '1.00', 100, "Ship GUI updater", "Arcade Sea");
	MOP_STRING("Left side cannons progress", "");
	MOP_STRING("Right side cannons progress", "");
	MOP_STRING("PowerShoot progress", "");
	MOP_STRING("Flamethrower progress", "");
	MOP_STRING("Mine ready progress", "");

	MOP_STRING("Mine counter progress", "");
	MOP_ARRAYBEG("Mine counter separator",1,100);
		MOP_FLOAT("Value",10000.f);
	MOP_ARRAYEND;

	MOP_ARRAYBEG("Left side quaddamage enable widgets",0,100);
		MOP_STRING("object name","");
	MOP_ARRAYEND;
	MOP_ARRAYBEG("Left side quaddamage disable widgets",0,100);
		MOP_STRING("object name","");
	MOP_ARRAYEND;

	MOP_ARRAYBEG("Right side quaddamage enable widgets",0,100);
	MOP_STRING("object name","");
	MOP_ARRAYEND;
	MOP_ARRAYBEG("Right side quaddamage disable widgets",0,100);
		MOP_STRING("object name","");
	MOP_ARRAYEND;

	MOP_ARRAYBEG("Flamethrower enable widgets",0,100);
		MOP_STRING("object name","");
	MOP_ARRAYEND;
	MOP_ARRAYBEG("Mine enable widgets",0,100);
		MOP_STRING("object name","");
	MOP_ARRAYEND;
MOP_ENDLIST(ShipGUIUpdater)
