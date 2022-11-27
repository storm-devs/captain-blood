#include "SeaMissionParams.h"
#include  "ships\sail.h"
#include  "ships\ship.h"
#include  "weapons\CannonBallBatcher.h"
#include  "weapons\Cannon.h"

SeaMissionParams::SeaMissionParams() :
	m_aPhysicSails(_FL_),
	m_aClothForDelete(_FL_),
	m_aActorForDelete(_FL_),
	m_aConnectorForDelete(_FL_)
{
	m_nPhysicsSailsQuantityMax = 8;
	m_bShowDebugInfo = false;
	m_pSailForSailInfo = NULL;
	m_pcCannonBallShadowTexture = NULL;
	m_pShipIterator = NULL;
}

SeaMissionParams::~SeaMissionParams()
{
	RELEASE( m_pShipIterator );

	// доудаляем все физ паруса из списка удаления
	for( long n=0; n<m_aClothForDelete; n++ )
		if( m_aClothForDelete[n] )
			m_aClothForDelete[n]->Release();
	m_aClothForDelete.DelAll();

	// доудаляем все физ актеров из списка удаления
	for( n=0; n<m_aActorForDelete; n++ )
		if( m_aActorForDelete[n] )
			m_aActorForDelete[n]->Release();
	m_aActorForDelete.DelAll();

	// доудаляем все физ коннекторы из списка удаления
	for( n=0; n<m_aConnectorForDelete; n++ )
		if( m_aConnectorForDelete[n] )
			m_aConnectorForDelete[n]->Release();
	m_aConnectorForDelete.DelAll();
}

bool SeaMissionParams::Create(MOPReader & reader)
{
	ReadMOPs(reader);

	SetUpdate(&SeaMissionParams::Work, ML_DEBUG);

	MOSafePointer safeptr;
	static const ConstString id_CannonBallContainer("CannonBallContainer");
	if( !FindObject(id_CannonBallContainer,safeptr) )
		Mission().CreateObject( safeptr, "CannonBallContainer", id_CannonBallContainer );
	if( safeptr.Ptr() && safeptr.Ptr()->Is(id_CannonBallContainer) )
		((CannonBallContainer*)safeptr.Ptr())->SetShadowTexture( GetCannonBallShadowTexture() );

	m_pShipIterator = &Mission().GroupIterator( MG_SHIP, _FL_ );

	Console().UnregisterCommand("shipinf");
	Console().RegisterCommand("shipinf","Command for ship information outview", this, (CONSOLE_COMMAND)&SeaMissionParams::Console_ShipCommand );

	return true;
}

void _cdecl SeaMissionParams::Work(float fDeltaTime, long level)
{
	SailLimit_Frame();

	// покадровое удаление физ парусов
	if( m_aClothForDelete.Size()>0 )
	{
		if( m_aClothForDelete.LastE() )
			m_aClothForDelete.LastE()->Release();
		m_aClothForDelete.DelIndex(m_aClothForDelete.Last());
	}

	// покадровое удаление физ коннекторов
	if( m_aConnectorForDelete.Size()>0 )
	{
		if( m_aConnectorForDelete.LastE() )
			m_aConnectorForDelete.LastE()->Release();
		m_aConnectorForDelete.DelIndex(m_aConnectorForDelete.Last());
	}

	// покадровое удаление физ актеров
	if( m_aActorForDelete.Size()>0 )
	{
		if( m_aActorForDelete.LastE() )
			m_aActorForDelete.LastE()->Release();
		m_aActorForDelete.DelIndex(m_aActorForDelete.Last());
	}

	// show debug info
	if( api->DebugKeyState( VK_SHIFT, VK_CONTROL, 'A' ) )
		m_bShowDebugInfo = !m_bShowDebugInfo;

	if( m_bShowDebugInfo )
	{
		Render().Print( 20,20, 0xFFFFFFFF, "SeaMission: physics sails limit = %d, start destroy sail quantity = %d, physics sail quantity = %d, destroing sail quantity = %d",
			m_nPhysicsSailsQuantityMax, m_nPhysicsSailsQuantityForDestroy, m_aPhysicSails.Size(), m_nDestroingSailQuantity );

		long nVQ=0, nIQ=0;
		for(long ns=0; ns<m_aPhysicSails; ns++)
		{
			Sail::SailDebugData sdd;
			m_aPhysicSails[ns]->GetDebugData(sdd);
			nVQ += sdd.nVertexQ;
			nIQ += sdd.nIndexQ;
		}
		Render().Print( 20,38, 0xFFFFFFFF, "         physics sails vertex quantity = %d, physics sails index quantity = %d", nVQ, nIQ );

		// выбор паруса и показ его инфы
		bool bSailNext = api->DebugKeyState( VK_SHIFT, VK_CONTROL, VK_UP );
		bool bSailPrev = api->DebugKeyState( VK_SHIFT, VK_CONTROL, VK_DOWN );
		bool bAllSailNext = api->DebugKeyState( VK_SHIFT, VK_CONTROL, VK_RIGHT );

		if( m_bKeyPressed && !bSailNext && !bSailPrev && !bAllSailNext )
			m_bKeyPressed = false;

		// скачем по списку физических парусов зарегестрированных в ограничении физпарусов
		if( !m_bKeyPressed && (bSailNext || bSailPrev) )
		{
			m_bKeyPressed = true;

			// найдем текущий селекченый
			dword dwCurSail = m_aPhysicSails.Find( m_pSailForSailInfo );

			// вперед к следущему парусу
			if( bSailNext )
			{
				if( dwCurSail == INVALID_ARRAY_INDEX )
				{
					// если такого паруса уже нет, то сбрасываем текущий в NULL
					if( m_pSailForSailInfo!=NULL ) dwCurSail = INVALID_ARRAY_INDEX;
					// иначе выбираем первый в списке
					else dwCurSail = m_aPhysicSails.First();
				}
				else
					if( ++dwCurSail == m_aPhysicSails.Size() ) dwCurSail = INVALID_ARRAY_INDEX;
			}
			// назад к предыдущему парусу
			if( bSailPrev )
			{
				if( dwCurSail == INVALID_ARRAY_INDEX )
				{
					// если такого паруса уже нет, то сбрасываем текущий в NULL
					if( m_pSailForSailInfo!=NULL ) dwCurSail = INVALID_ARRAY_INDEX;
					// иначе выбираем последний в списке
					else dwCurSail = m_aPhysicSails.Last();
				}
				else
					if( dwCurSail-- == 0 ) dwCurSail = INVALID_ARRAY_INDEX;
			}

			// ставим выбранный парус как текущий для вывода дебаг инфы
			if( dwCurSail == INVALID_ARRAY_INDEX || dwCurSail >= m_aPhysicSails.Size() )
				m_pSailForSailInfo = NULL;
			else
				m_pSailForSailInfo = m_aPhysicSails[dwCurSail];
		}

		// скачем по всем парусам вообще
		if( !m_bKeyPressed && bAllSailNext )
		{
			m_bKeyPressed = true;

			class AllSailCollector : public	IPartVisitor
			{
			public:
				array<Sail*> aAllSails;

				AllSailCollector() : aAllSails(_FL_) {}

				void ProcessChildrens(ShipPart& part)
				{
					for( long n=0; n<part.GetChildren(); n++ )
						part.GetChildren()[n]->AcceptVisitor(*this);
				}

				// уловители
				virtual void Visit(ShipPart& part) {ProcessChildrens(part);}
				virtual void Visit(Ship& part) {ProcessChildrens(part);}
				virtual void Visit(HullPart& part) {ProcessChildrens(part);}
				virtual void Visit(MastPart& part) {ProcessChildrens(part);}
				virtual void Visit(Mast& part) {ProcessChildrens(part);}
				virtual void Visit(Yard& part) {}
				virtual void Visit(Rope& part) {}
				virtual void Visit(Sail& part) {aAllSails.Add(&part);}

				virtual void Reset() {}
			};
			AllSailCollector sailcoll;

			if( m_pShipIterator )
				for( m_pShipIterator->Reset(); !m_pShipIterator->IsDone(); m_pShipIterator->Next() )
				{
					Ship* pShip = (Ship*)m_pShipIterator->Get();
					pShip->AcceptVisitor(sailcoll);
				}

			dword dwCur = sailcoll.aAllSails.Find(m_pSailForSailInfo);
			if( dwCur != INVALID_ARRAY_INDEX )
			{
				if( ++dwCur >= sailcoll.aAllSails.Size() )
					dwCur = INVALID_ARRAY_INDEX;
			}
			else
				if( sailcoll.aAllSails.Size() > 0 ) dwCur = 0;
			if( dwCur == INVALID_ARRAY_INDEX ) m_pSailForSailInfo = NULL;
			else m_pSailForSailInfo = sailcoll.aAllSails[dwCur];
		}

		if( m_pSailForSailInfo )
			m_pSailForSailInfo->ShowSailDebugInfo(20,60);
	}
}

void SeaMissionParams::ReadMOPs(MOPReader & reader)
{
	m_nPhysicsSailsQuantityMax = reader.Long();
	m_nPhysicsSailsQuantityForDestroy = reader.Long();
	m_pcCannonBallShadowTexture = reader.String().c_str();
}

bool SeaMissionParams::SailLimit_SetFake( bool bFake, Sail* pSail )
{
	if( bFake )
	{
		m_aPhysicSails.Del(pSail);
	}
	else
	{
		if( m_aPhysicSails.IsExist( pSail ) )
			return true;

		if( (long)m_aPhysicSails.Size() >= m_nPhysicsSailsQuantityMax )
			return false;

		m_aPhysicSails.Add( pSail );
	}
	return true;
}

void SeaMissionParams::SailLimit_DelSail( Sail* pSail )
{
	m_aPhysicSails.Del( pSail );
}

void SeaMissionParams::SailLimit_AddSail( Sail* pSail )
{
	if( !m_aPhysicSails.IsExist( pSail ) )
		m_aPhysicSails.Add( pSail );
}

const char* SeaMissionParams::GetMaterialName(dword dwMat)
{
	switch(dwMat)
	{
	case pmtlid_air: return "air";
	case pmtlid_ground: return "ground";
	case pmtlid_stone: return "stone";
	case pmtlid_sand: return "sand";
	case pmtlid_wood: return "wood";
	case pmtlid_grass: return "grass";
	case pmtlid_water: return "water";
	case pmtlid_iron: return "iron";
	case pmtlid_fabrics: return "fabrics";
	case pmtlid_other1: return "other1";
	case pmtlid_other2: return "other2";
	case pmtlid_other3: return "other3";
	case pmtlid_other4: return "other4";
	}
	return null;
}

void _cdecl SeaMissionParams::Console_ShipCommand(const ConsoleStack &params)
{
	if( params.GetSize() < 1 )
	{
		Console().Trace(COL_CMD_OUTPUT, "shipinf format: (where <id> is ID of ship mission object or keyword 'all' for every ship)");
		Console().Trace(COL_CMD_OUTPUT, "  shipinf ship <id> - show common information about ship");
		Console().Trace(COL_CMD_OUTPUT, "  shipinf phys <id> - show physics form information about ship");
		Console().Trace(COL_CMD_OUTPUT, "  shipinf weapon <id> - show weapon information");
		Console().Trace(COL_CMD_OUTPUT, "  shipinf ai <id> - AI information");
		Console().Trace(COL_CMD_OUTPUT, "  shipinf off - turn off all information about every ship");
		Console().Trace(COL_CMD_OUTPUT, "  shipinf on <id> - turn on any information about every ship");
		return;
	}

	if( !m_pShipIterator )
	{
		Console().Trace(COL_CMD_OUTPUT, "Not created ship iterator. Command is useless.");
		return;
	}

	// команда всем кораблям
	ConstString checkID;
	if( params.GetSize() > 1 )
	{
		if( crt_stricmp(params.GetParam(1),"all") != 0 )
			checkID.Set( params.GetParam(1) );
	}

	dword dwMask = 0;
	dword dwAction = cmdact_switch;
	// включить всю инфу
	if( crt_stricmp(params.GetParam(0),"on")==0 ) {
		dwMask = -1;
		dwAction = cmdact_set;
	}
	// отключить всю инфу
	else if( crt_stricmp(params.GetParam(0),"off")==0 ) {
		dwMask = -1;
		dwAction = cmdact_reset;
	}
	// общая инфа
	else if( crt_stricmp(params.GetParam(0),"ship")==0 )
		dwMask = Ship::sdim_ship_info;
	// физическая форма
	else if( crt_stricmp(params.GetParam(0),"phys")==0 )
		dwMask = Ship::sdim_phys_form;
	else if( crt_stricmp(params.GetParam(0),"weapon")==0 )
		dwMask = Ship::sdim_cannon_info;
	else if( crt_stricmp(params.GetParam(0),"ai")==0 )
		dwMask = Ship::sdim_ai_info;
	// не распознаная команда
	else
	{
		Console().Trace(COL_CMD_OUTPUT, "Unknown command. Can`t execute");
		return;
	}

	// отдаем команду кораблям
	bool bSuccess = false;
	for( m_pShipIterator->Reset(); !m_pShipIterator->IsDone(); m_pShipIterator->Next() )
	{
		Ship* pship = (Ship*)m_pShipIterator->Get();
		// пропускаем корабли которым не нужно слать команду
		if( !checkID.IsEmpty() && pship->GetObjectID()!=checkID )
			continue;
		if( SendInfoCommandToShip( pship, dwMask, dwAction ) )
			bSuccess = true;
	}

	if( !bSuccess )
	{
		Console().Trace(COL_CMD_OUTPUT, "Command failed");
	}
}

bool SeaMissionParams::SendInfoCommandToShip( Ship* pship, dword mask, dword action )
{
	switch(action)
	{
	case cmdact_set: pship->Info_SetMask( mask ); return true;
	case cmdact_reset: pship->Info_ResetMask( mask ); return true;
	case cmdact_switch: pship->Info_SwitchMask( mask ); return true;
	}
	return false;
}

void SeaMissionParams::SailLimit_Frame()
{
	m_nDestroingSailQuantity = 0;
	long nQ = m_aPhysicSails.Size();

	if( nQ < m_nPhysicsSailsQuantityForDestroy )
		return;

	// подсчитаем реальное количество "целых" парусов
	for( long n=0; n<m_aPhysicSails; n++ )
		if( m_aPhysicSails[n]->IsDestroying() )
			nQ--;

	while( nQ >= m_nPhysicsSailsQuantityForDestroy )
	{
		// найдем парус с наибольшим фактором разрыва ткани
		long nDestroySail = -1;
		float fDestroyFactor = 0.f;
		for( long n=0; n<m_aPhysicSails; n++ )
		{
			if( m_aPhysicSails[n]->IsDestroying() )
				continue;

			float fFactor = m_aPhysicSails[n]->GetTearedState();
			if( fFactor > fDestroyFactor )
			{
				nDestroySail = n;
				fDestroyFactor = fFactor;
			}
		}

		// нашли такой парус - уничтожаем
		if( nDestroySail != -1 )
		{
			SailLimit_LaunchSailDestroy( m_aPhysicSails[nDestroySail] );
			nQ--;
		}
		// больше не нашли поврежденных парусов
		else
		{ // удаляем паруса по вероятности
			for( long n=0; n<m_aPhysicSails && nQ>=m_nPhysicsSailsQuantityForDestroy; n++ )
				if( !m_aPhysicSails[n]->IsDestroying() && FRAND(100.f) < 30.f )
				{
					SailLimit_LaunchSailDestroy( m_aPhysicSails[n] );
					nQ--;
				}
		}
	}

	m_nDestroingSailQuantity = m_aPhysicSails.Size() - nQ;
}

void SeaMissionParams::SailLimit_LaunchSailDestroy(Sail* pSail)
{
	pSail->LaunchDestroy();
}

MOP_BEGINLISTCG(SeaMissionParams, "SeaMissionParams", '1.00', 100, "Sea mission common parameters", "Arcade Sea");
	MOP_LONG("Physics sails limit",10)
	MOP_LONG("Physics sails quantity for destroy begin",8)
	MOP_STRING("Cannon ball shadow texture","ballShadow")
MOP_ENDLIST(SeaMissionParams)
