#include "MOSailOwner.h"
#include "ClothBurns.h"

MOSailOwner::MOSailOwner()
{
	m_pMO = NULL;
}

void MOSailOwner::SetMO(MissionSailOwner* pMO)
{
	m_pMO = pMO;
}

IPhysCombined* MOSailOwner::GetPhysView()
{
	return m_pMO->GetPhysBody();
}

IPhysCloth* MOSailOwner::CreateCloth(const char* file, long line, IClothRenderInfo & renderInfo, IClothMeshBuilder & builder, IPhysCloth::SimulationData & simData)
{
	if( m_pMO )
		return m_pMO->Physics().CreateCloth(file,line, renderInfo, builder, simData);
	return NULL;
}

bool MOSailOwner::EditMode_IsOn()
{
	if( m_pMO ) return m_pMO->EditMode_IsOn();
	return false;
}

Matrix& MOSailOwner::GetMatrix(Matrix& mtx)
{
	if( m_pMO ) return m_pMO->GetMatrix(mtx);
	mtx.SetIdentity();
	return mtx;
}

float MOSailOwner::GetLastDeltaTime()
{
	if( m_pMO ) return m_pMO->GetLastDeltaTime();
	return 0.f;
}

IClothBurns* MOSailOwner::GetClothBurns()
{
	return NULL;
}

const char* MOSailOwner::GetSoundNameForEvent(dword dwIdx)
{
	return "";
}

void MOSailOwner::AcceptVisitor(IPartVisitor& clsf)
{
}


SailsPattern::~SailsPattern()
{
	ReleaseAllSails();
}

bool SailsPattern::Create(MOPReader & reader)
{
	ReadMOPs(reader);
	if( EditMode_IsOn() )
		SetUpdate( &SailsPattern::EditModeDraw, ML_GEOMETRY1-10 );
	return true;
}

bool SailsPattern::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);
	return true;
}

void _cdecl SailsPattern::EditModeDraw(float fDeltaTime, long level)
{
	if ( !Mission().EditMode_IsAdditionalDraw() ) return;
	const float lineEndMarkerSize = 0.1f;
	for (unsigned int k = 0; k < m_aSailPattern.Size(); k++)
	{
		Sail::Params & p = m_aSailPattern[k].param;

		Matrix mtx(p.orient,p.pos);

		Vector v[4];
		for (unsigned int i = 0; i < p.pointsCount; ++i)
		{
			v[i] = mtx.MulVertex(p.points[i]);
			Render().DrawSphere(v[i], lineEndMarkerSize, 0xFF00FF00);
		}

		// рисуем одну сторону
		Render().DrawPolygon( v, p.pointsCount, 0x80008040 );
		// рисуем другую сторону
		Vector vtmp = v[1];
		if( p.pointsCount==3 )
		{
			v[1] = v[2];
			v[2] = vtmp;
		}
		else
		{
			v[1] = v[3];
			v[3] = vtmp;
		}
		Render().DrawPolygon( v, p.pointsCount, 0x80008040 );
	}
}

// построить парус
void SailsPattern::BuildSail(unsigned int index)
{
	// недопустимый параметр
	Assert( index < m_aSailPattern.Size() );

	// создаем билдер паруса
	IPhysics * physService = (IPhysics*)api->GetService("PhysicsService");
	Assert(physService);
	IClothMeshBuilder * builder = physService->CreateClothMeshBuilder();

	// параметры паттерна
	SailPattern& pattern = m_aSailPattern[index];

	// создаем парус
	unsigned int sailIndex = 0;
	if (pattern.param.pointsCount == 4)
		sailIndex = builder->AddSail( pattern.param.points, pattern.param.widthPointsCount,
			pattern.param.heightPointsCount, pattern.param.windCurvature, pattern.param.bottomCurvature);
	else
		sailIndex = builder->AddTriangle( pattern.param.points, pattern.param.widthPointsCount);

	// имя паттерна паруса
	string sailBinName;
	sailBinName.Format("%s_%s_%d.pxccm", Mission().GetMissionName(), GetObjectID().c_str(), index);
	// пытаемся создать парус
	bool bBuildSail = false;
	if( !EditMode_IsOn() )
	{
		// в режиме игры, считываем готовый парус из предварительно заготовленного буфера
		bBuildSail = builder->Load(Physics(), Mission(), sailBinName.c_str());
		if( !bBuildSail )
			api->Trace("Error! Can`t load cooked mesh for sail %s (check file: shippattern\\%s.pxm)",sailBinName.GetBuffer(),Mission().GetMissionName());
	}
	else
		bBuildSail = builder->Build(Physics(), true, &Mission(), sailBinName.c_str());

	// если не был построен парус, то мы удаляем билдер за ненадобностью
	if( !bBuildSail )
	{
		builder->Release();
		pattern.cooked = NULL;
	}
	else
		pattern.cooked = builder;
}

void SailsPattern::ReadMOPs(MOPReader & reader)
{
	ReleaseAllSails();

	// массив парусов
	dword dwSailsQantity = reader.Array();
	if( dwSailsQantity > 0 )
		m_aSailPattern.AddElements( dwSailsQantity );
	// считываем параметры для каждого паруса
	for( dword n=0; n<dwSailsQantity; n++ )
	{
		// угловые точки паруса
		dword q = reader.Array();
		m_aSailPattern[n].param.pointsCount = q;
		for( dword i=0; i<q; i++ )
			m_aSailPattern[n].param.points[i] = reader.Position();

		// размеры сетки паруса
		m_aSailPattern[n].param.widthPointsCount = reader.Long();
		m_aSailPattern[n].param.heightPointsCount = reader.Long();

		// положение паруса в группе
		m_aSailPattern[n].param.orient = reader.Angles();
		m_aSailPattern[n].param.pos = reader.Position();

		// текстуры и цвета паруса
		m_aSailPattern[n].param.texture = reader.String().c_str();
		m_aSailPattern[n].param.glimpseTexture = reader.String().c_str();
		m_aSailPattern[n].param.color = reader.Colors();
		m_aSailPattern[n].param.glimpseColor = reader.Colors();
		m_aSailPattern[n].param.colorScale = reader.Float();

		// параметры фейкового паруса
		m_aSailPattern[n].param.fakeAnimAmplitude = reader.Float();
		m_aSailPattern[n].param.xFreq = reader.Float();
		m_aSailPattern[n].param.yFreq = reader.Float();

		// крепкость сетки паруса
		m_aSailPattern[n].param.tearFactor = reader.Float();
		// фома паруса (прогибы)
		m_aSailPattern[n].param.windCurvature = reader.Float();
		m_aSailPattern[n].param.bottomCurvature = reader.Float();

		// пока нет готового паруса
		m_aSailPattern[n].cooked = NULL;
	}

	// строим новые паруса
	for( dword n=0; n<m_aSailPattern.Size(); n++ )
	{
		RELEASE( m_aSailPattern[n].cooked );
		BuildSail(n);
	}
}

void SailsPattern::ReleaseAllSails()
{
	for( dword n=0; n<m_aSailPattern.Size(); n++ )
	{
		// удаляем готовые паруса
		RELEASE( m_aSailPattern[n].cooked );
	}
	m_aSailPattern.DelAll();
}


//Конструктор
MissionSailOwner::MissionSailOwner() :
	m_aSails(_FL_)
{
	m_MOSailOwner.SetMO( this );

	m_pCBurns.Reset();

	m_pSailPattern = null;

	m_pConnectTo.Reset();

	m_fLastDeltaTime = 0.f;
	m_fAlpha = 1.f;

	m_bNoSwing = true;
	m_bReflection = true;
	m_bRefraction = true;
	m_bFakeSails = true;
	m_bVisible = true;
	m_bActive = true;

	m_pFinder = null;
	m_vFinderMin = Vector(-10.f,0.f,10.f);
	m_vFinderMax = Vector(10.f,10.f,-10.f);

	m_pRigidBody = null;
}

//Деструктор
MissionSailOwner::~MissionSailOwner()
{
	ReleaseAll();
}

void _cdecl MissionSailOwner::Work(float fDeltaTime, long level)
{
	// запомним значение времени
	m_fLastDeltaTime = fDeltaTime;

	// матрица изменения положения
	Matrix mtxDiff(m_mtxTransform);
	mtxDiff.Inverse();

	// получаем матрицу трансформации от привязанного объекта
	if( m_pConnectTo.Validate() )
		m_pConnectTo.Ptr()->GetMatrix(m_mtxTransform);
	else
		m_mtxTransform.SetIdentity();

	mtxDiff *= m_mtxTransform;

	// обновим позицию в QT
	if( m_pFinder )
		m_pFinder->SetMatrix(m_mtxTransform);

	// двигаем физ тело
	if( m_pRigidBody )
	{
		m_pRigidBody->SetTransform(m_mtxTransform);
		/* Дебажная отрисовка
		long q = m_pRigidBody->GetCount();
		for( long k=0; k<q; k++ )
		{
			Vector vSize;
			Matrix mtx(true);
			m_pRigidBody->GetBox(k,vSize);
			m_pRigidBody->GetGlobalTransform(k,mtx);
			Render().DrawBox( -vSize, vSize, mtx );
		}*/
	}

	// дебажная отрисовка
	if ( api->DebugKeyState(VK_SHIFT, VK_CONTROL, 'Z') )
	{
		if( m_pFinder )
		{
			Render().DrawBox(
				m_pFinder->GetBoxCenter() - m_pFinder->GetBoxSize()*0.5f,
				m_pFinder->GetBoxCenter() + m_pFinder->GetBoxSize()*0.5f,
				m_mtxTransform);
		}
	}

	// обработка парусов на кадре
	for( dword n=0; n<m_aSails.Size(); n++ )
	{
		if( !m_aSails[n].sail ) continue;
		// дотягиваем парус до нового положения, так как движение идет не из физики, а от внешней трансформации
		// поэтому будет расхождение с текущим положением, а нам не надо этого
		//mtxDiff

		m_aSails[n].sail->StartFrame();
		// расчеты
		m_aSails[n].sail->Work( m_mtxTransform );
		// отрисовка
		m_aSails[n].sail->DrawPart( m_mtxTransform );
	}
}

void _cdecl MissionSailOwner::DrawRefl(const char*, MissionObject*)
{
	for( dword n=0; n<m_aSails.Size(); n++ )
		if( m_aSails[n].sail )
			m_aSails[n].sail->DrawPartRefl( m_mtxTransform );
}

void _cdecl MissionSailOwner::DrawRefr(const char*, MissionObject*)
{
	for( dword n=0; n<m_aSails.Size(); n++ )
		if( m_aSails[n].sail )
			m_aSails[n].sail->DrawPartRefl( m_mtxTransform );
}

void MissionSailOwner::Accept(IPartVisitor& visitor)
{
	for( dword n=0; n<m_aSails.Size(); n++ )
		if( m_aSails[n].sail )
			m_aSails[n].sail->AcceptVisitor(visitor);
}

//Воздействовать на объект сферой
bool MissionSailOwner::Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius)
{
	return false;
}

//Воздействовать на объект линией
bool MissionSailOwner::Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to)
{
	for( dword n=0; n<m_aSails.Size(); n++ )
	{
		if( m_aSails[n].sail &&
			m_aSails[n].sail->Attack(from, to, source==DamageReceiver::ds_check) )
			return true;
	}
	return false;
}

//Воздействовать на объект выпуклым чехырёхугольником
bool MissionSailOwner::Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4])
{
	return false;
}

// Создание объекта
bool MissionSailOwner::Create(MOPReader & reader)
{
	ReadMOPs(reader);

	// видимость/активность на старте
	Show(true);//m_bVisible);
	Activate(m_bActive);

	return true;
}

void MissionSailOwner::PostCreate()
{
	MOSafePointer ptr;

	// ищем миссионный объект управляющий горящими парусами
	static const ConstString id_ClothBurns("ClothBurns");
	FindObject(id_ClothBurns, m_pCBurns.GetSPObject());
	if( m_pCBurns.Ptr() && !m_pCBurns.Ptr()->Is(id_ClothBurns) )
		m_pCBurns.Reset();

	// ищем паттерн парусов
	static const ConstString id_SailsPattern("SailsPattern");
	m_pSailPattern = null;
	if( m_pcSailPatternName.NotEmpty() )
		if( FindObject(m_pcSailPatternName, ptr) )
			if( ptr.Ptr()->Is(id_SailsPattern) )
				m_pSailPattern = (SailsPattern*)ptr.Ptr();

	// ищем объект для привязки парусов
	if( m_pcConnectToName.IsEmpty() )
		m_pConnectTo.Reset();
	else
		FindObject(m_pcConnectToName, m_pConnectTo);

	// получим матрицу трансформации текущего положения
	if( m_pConnectTo.Ptr() )
		m_pConnectTo.Ptr()->GetMatrix(m_mtxTransform);
	else
		m_mtxTransform.SetIdentity();

	// создаем физ тело, к которому будут крепиться физ паруса
	m_pRigidBody = Physics().CreateCombined(_FL_, Matrix());
	Assert(m_pRigidBody);
	m_pRigidBody->AddBox( Vector(0.5f), Vector(0.f) );
	m_pRigidBody->SetMass(0, 0.1f);
	m_pRigidBody->EnableCollision(0, false);
	m_pRigidBody->EnableResponse(0, false);
	m_pRigidBody->EnableRaycast(0, false);

	m_vFinderMin = m_vFinderMax = 0.f;

	// точки паруса
	Vector vcor[4];
	// создаем реальные паруса
	dword n;
	if( m_pSailPattern )
		for( n=0; n<m_aSails.Size(); n++ )
		{
			const SailsPattern::SailPattern * pPattern = m_pSailPattern->GetPattern(m_aSails[n].idx);
			if( !pPattern ) continue;

			// паттерн паруса
			const SailsPattern::SailPattern & pattern = *pPattern;

			// матрица паруса
			Matrix mtxSail( pattern.param.orient, pattern.param.pos );
			// размер боксов для крепления паруса
			Vector sizeBoxTop = 0.2f;
			Vector sizeBoxBtm = 0.2f;

			// точки паруса
			for( unsigned int j=0; j<pattern.param.pointsCount; j++ )
			{
				vcor[j] = mtxSail.MulVertex( pattern.param.points[j] );
				m_vFinderMin.Min(vcor[j]);
				m_vFinderMax.Max(vcor[j]);
			}

			// матрицы положения боксов для крепления паруса
			Matrix mtxBoxTop, mtxBoxBtm;
			if( pattern.param.pointsCount==3 )
			{
				// матрица бля бокса верхнего крепления
				mtxBoxTop.BuildPosition( vcor[0] );

				// формируем матрицу для бокса нижнего крепления
				mtxBoxBtm.vx = vcor[1] - vcor[2];
				sizeBoxBtm.x = mtxBoxBtm.vx.Normalize();
				mtxBoxBtm.vz = (mtxBoxBtm.vx ^ Vector(0.f,1.f,0.f));
				if( mtxBoxBtm.vz.Normalize() < 0.001f ) continue;
				mtxBoxBtm.vy = mtxBoxBtm.vz ^ mtxBoxBtm.vx;
				mtxBoxBtm.pos = (vcor[1] + vcor[2]) * 0.5f;
			}
			else
			{
				// формируем матрицу для бокса верхнего крепления
				mtxBoxTop.vx = vcor[1] - vcor[0];
				sizeBoxTop.x = mtxBoxTop.vx.Normalize();
				mtxBoxTop.vz = (mtxBoxTop.vx ^ Vector(0.f,1.f,0.f));
				if( mtxBoxTop.vz.Normalize() < 0.001f ) continue;
				mtxBoxTop.vy = mtxBoxTop.vz ^ mtxBoxTop.vx;
				mtxBoxTop.pos = (vcor[1] + vcor[0]) * 0.5f;

				// формируем матрицу для бокса нижнего крепления
				mtxBoxBtm.vx = vcor[2] - vcor[3];
				sizeBoxBtm.x = mtxBoxBtm.vx.Normalize();
				mtxBoxBtm.vz = (mtxBoxBtm.vx ^ Vector(0.f,1.f,0.f));
				if( mtxBoxBtm.vz.Normalize() < 0.001f ) continue;
				mtxBoxBtm.vy = mtxBoxBtm.vz ^ mtxBoxBtm.vx;
				mtxBoxBtm.pos = (vcor[2] + vcor[3]) * 0.5f;
			}
			// пропускаем неправильные паруса
			if( sizeBoxTop.x < 0.001f || sizeBoxBtm.x < 0.001f )
				continue;
			// верхняя планка
			m_pRigidBody->AddBox(sizeBoxTop, mtxBoxTop);
			m_aSails[n].nBoxTop = m_pRigidBody->GetCountForBuild() - 1;
			// нижняя планка
			m_pRigidBody->AddBox(sizeBoxBtm, mtxBoxBtm);
			m_aSails[n].nBoxBottom = m_pRigidBody->GetCountForBuild() - 1;

			// создаем экземпляр паруса
			Sail* sail = NEW Sail(pattern.param, "", &m_MOSailOwner);
			Assert(sail);
			// никого выше паруса нет (верхний по иерархии)
			sail->SetParent(null);
			// хозяин паруса этот объект - все обращения как к миссионному объекту
			sail->SetOwner(null,this);
			// создать парус
			sail->Make(m_bFakeSails, pattern.cooked, false);

			// низ
			if( pattern.param.pointsCount==3 )
			{
				sail->TieSailToPhysShape( true, *m_pRigidBody, m_aSails[n].nBoxTop, mtxBoxTop.MulVertexByInverse(vcor[0]), vcor[1] );
				sail->TieSailToPhysShape( false, *m_pRigidBody, m_aSails[n].nBoxBottom, mtxBoxBtm.MulVertexByInverse(vcor[1]), mtxBoxBtm.MulVertexByInverse(vcor[2]) );
			}
			else
			{
				sail->TieSailToPhysShape( true, *m_pRigidBody, m_aSails[n].nBoxTop, mtxBoxTop.MulVertexByInverse(vcor[0]), mtxBoxTop.MulVertexByInverse(vcor[1]) );
				sail->TieSailToPhysShape( false, *m_pRigidBody, m_aSails[n].nBoxBottom, mtxBoxBtm.MulVertexByInverse(vcor[2]), mtxBoxBtm.MulVertexByInverse(vcor[3]) );
			}

			// пропишем парус в списке
			m_aSails[n].sail = sail;
		}

	// строим физ объект
	m_pRigidBody->Build();
	m_pRigidBody->EnableGravity( false );
	m_pRigidBody->SetTransform(m_mtxTransform);

	if( m_pFinder )
		m_pFinder->SetBox( m_vFinderMin, m_vFinderMax );

	// видимость/активность на старте
//	Show(m_bVisible);
//	Activate(m_bActive);
}

// Рестарт объекта
void MissionSailOwner::Restart()
{
	ReleaseAll();
	ReCreate();
	PostCreate();
}

// Обновление параметров
bool MissionSailOwner::EditMode_Update(MOPReader & reader)
{
	ReadMOPs(reader);
	return true;
}

Matrix & MissionSailOwner::GetMatrix(Matrix & mtx)
{
	mtx = m_mtxTransform;
	return mtx;
}

void MissionSailOwner::GetBox(Vector & min, Vector & max)
{
	min = m_vFinderMin;
	max = m_vFinderMax;
}

void MissionSailOwner::Show(bool isShow)
{
	MissionObject::Show( isShow );
}

void MissionSailOwner::Activate(bool isActive)
{
	MissionObject::Activate( isActive );

	// нужна, нет? обработка на кадре/рисование?
	bool bWork = false;
	if( isActive )
		if( !EditMode_IsOn() )
			bWork = true;

	// основная обработка на кадре с рисованием
	if( bWork )
		SetUpdate( &MissionSailOwner::Work, ML_GEOMETRY1 );
	else
		DelUpdate( &MissionSailOwner::Work );

	// рисование отражения
	if( bWork && m_bReflection )
		Registry(MG_SEAREFLECTION, &MissionSailOwner::DrawRefl, ML_GEOMETRY5);
	else
		Unregistry(MG_SEAREFLECTION);

	// рисование переломления
	if( bWork && m_bRefraction )
		Registry(MG_SEAREFRACTION, &MissionSailOwner::DrawRefr, ML_GEOMETRY5);
	else
		Unregistry(MG_SEAREFRACTION);

	// объект для поиска по QT
	if( bWork )
	{
		if( !m_pFinder )
			m_pFinder = QTCreateObject(MG_DAMAGEACCEPTOR, _FL_);

		if( m_pFinder )
		{
			m_pFinder->SetBox( m_vFinderMin, m_vFinderMax );
			m_pFinder->Activate(true);
		}
		Registry( MG_DAMAGEACCEPTOR );
	}
	else
	{
		Unregistry( MG_DAMAGEACCEPTOR );
		RELEASE(m_pFinder);
	}
}

//Обработчик команд для объекта
void MissionSailOwner::Command(const char * id, dword numParams, const char ** params)
{
	if( !id ) return;
	if( string::IsEqual(id, "DestroySails") )
	{
		for( dword n=0; n<m_aSails.Size(); n++ )
			if( m_aSails[n].sail )
				m_aSails[n].sail->LaunchDestroy();
	}
}

// чтение параметров
void MissionSailOwner::ReadMOPs(MOPReader & reader)
{
	// удаляем старые паруса
	for( dword n=0; n<m_aSails.Size(); n++ )
	{
		DELETE( m_aSails[n].sail );
	}
	m_aSails.DelAll();

	// объект к которому присоединяемся
	m_pcConnectToName = reader.String();
	m_pConnectTo.Reset();

	// паттерн парусов
	m_pcSailPatternName = reader.String();
	m_pSailPattern = null;

	// список парусов
	long q = reader.Array();
	if( q>0 )
	{
		m_aSails.AddElements(q);
		for( long i=0; i<q; i++ )
		{
			m_aSails[i].idx = reader.Long();
			// пока не создан парус
			m_aSails[i].sail = null;
		}
	}

	// звук рвущегося паруса
	m_pcTearSound = reader.String();

	// флажки
	m_bNoSwing = reader.Bool();
	m_bReflection = reader.Bool();
	m_bRefraction = reader.Bool();
	m_bFakeSails = reader.Bool();
	m_bVisible = reader.Bool();
	m_bActive = reader.Bool();
}

// удаление внутренних данных объекта
void MissionSailOwner::ReleaseAll()
{
	// отписываем объект из системы горения парусов
	if( m_pCBurns.Validate() )
		m_pCBurns.Ptr()->ExcludeObjects( (MissionObject*)this );

	// удаляем паруса
	for( dword n=0; n<m_aSails.Size(); n++ )
	{
		DELETE( m_aSails[n].sail );
	}
	m_aSails.DelAll();

	// удаляем объект для поиска по QT
	RELEASE( m_pFinder );

	// убиваем физ тело
	RELEASE( m_pRigidBody );
}

void MissionSailOwner::SetFake(long idx,bool bFake)
{
	if( idx>=0 && idx<m_aSails )
	{
		if( m_aSails[idx].sail )
		{
			m_aSails[idx].sail->SetFake(bFake);
		}
	}
}



// паттерн парусов
MOP_BEGINLISTCG(SailsPattern, "SailsPattern", '1.00', 100, "Sails pattern object", "Arcade Sea");
	MOP_ARRAYBEG("Sails", 0, 100)
		MOP_ARRAYBEG("Sail corners", 3, 4)
			MOP_POSITIONC("Corner", Vector(0.0f, 10.0f, 0.0f), "It is relative position")
		MOP_ARRAYEND

		MOP_LONGEX("Width points count", 10, 5, 100)
		MOP_LONGEX("Height points count", 10, 5, 100)

		MOP_ANGLESEX("Orientation", Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, 0.0f), Vector(360.0f, 360.0f, 360.0f))
		MOP_POSITION("Position", Vector(0.0f, 0.0f, 0.0f))

		MOP_STRINGC("Diffuse texture", "Parus", "Texture File Name  ( *.txx, *.tga )")
		MOP_STRINGC("Glimpse texture", "", "Texture File Name  ( *.txx, *.tga )")
		MOP_COLOR("Sail color", Color (1.0f, 1.0f, 1.0f))
		MOP_COLOR("Glimpse color", Color (1.0f, 1.0f, 0.8f))
		MOP_FLOAT("Color scale", 1.0f);

		MOP_FLOAT("Anim sail wave amplitude", 0.25f)
		MOP_FLOAT("X Wave frequency", 1.0f)
		MOP_FLOAT("Y Wave frequency", 1.0f)

		MOP_FLOATEXC("Tear Factor", 35.0f, 1, 100, "Cloth Tear Factor")
		MOP_FLOAT("Wind curvature", 1.5f)
		MOP_FLOATEX("Bottom curvature", 0.9f, 0.0f, 2.0f)
	MOP_ARRAYEND
MOP_ENDLIST(SailsPattern)

// миссионный объект держатель паруса
static char MissionSailOwnerDescr[] =
"-= Sails owner object =-\n"
" Commands:\n"
"DestroySails - уничтожить паруса. Быстро уничтожает все прикрепленные к объекту паруса.";
MOP_BEGINLISTCG(MissionSailOwner, "SailOwner", '1.00', 101, MissionSailOwnerDescr, "Arcade Sea");
	MOP_STRINGC("ConnectTo", "", "ConnectTo mission object")

	MOP_STRINGC("Sail pattern", "", "Sails pattern object ID")
	MOP_ARRAYBEG("Sails", 0, 10)
		MOP_LONGC("Sail index", 0, "Sail index into pattern")
	MOP_ARRAYEND

	MOP_STRINGC("SailTearSound", "", "Sound for sail hit and tear")

	MOP_BOOLC("No swing", true, "No swing sails in swing machine")
	MOP_BOOL("Sea reflection", true)
	MOP_BOOL("Sea refraction", true)
	MOP_BOOL("Fake sails", true)
	MOP_BOOL("Visible", true)
	MOP_BOOL("Active", true)
MOP_ENDLIST(MissionSailOwner)
