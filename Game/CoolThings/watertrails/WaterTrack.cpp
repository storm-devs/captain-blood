#include "WaterTrack.h"

WaterTrack::WaterTrack()
{
	bPrevPosInited = false;
	vPrevPos = 0.f;

	pMaster.Reset();

	BurunParticle = NULL;
	BurunBackParticle = NULL;
	m_fParticleScale = 1.f;

	offset = 0.f;
	back_offset = 0.f;

	curPos = 0.f;
	backPos = 0.f;

	fTime = 0.0f;

	bSmoothSpeedInited = false;
	fSmoothSpeed = 0.0f;

	bWorkOnlyWithActive = true;
	bWorkOnlyWithVisible = true;
	bWorkOnlyWithLive = true;

	noSwing = false;
}


WaterTrack::~WaterTrack()
{
	RELEASE(BurunParticle);
	RELEASE(BurunBackParticle);
}

//Создание объекта
bool WaterTrack::Create(MOPReader &reader)
{
	EditMode_Update(reader);
	SetUpdate((MOF_UPDATE)&WaterTrack::Work,ML_ALPHA4);
	return true;
}

//Инициализировать объект
bool WaterTrack::EditMode_Create(MOPReader &reader)
{
	EditMode_Update(reader);
	return true;
}

//Обновление параметров
bool WaterTrack::EditMode_Update(MOPReader &reader)
{
	MissionObjectName = reader.String();
	FindObject(MissionObjectName,pMaster);

	ConstString BurunParticles = reader.String();
	RELEASE(BurunParticle);
	BurunParticle = Particles().CreateParticleSystemEx(BurunParticles.c_str(),_FL_);

	BurunParticles = reader.String();
	RELEASE(BurunBackParticle);
	BurunBackParticle = Particles().CreateParticleSystemEx(BurunParticles.c_str(),_FL_);

	m_fParticleScale = reader.Float();

	PauseParticles(true);

	offset = reader.Position();
	back_offset = reader.Position();

	bPrevPosInited = false;
	bSmoothSpeedInited = false;
	fSmoothSpeed = 0.0f;
	fTime = 0.0f;

	bWorkOnlyWithActive	 = reader.Bool();
	bWorkOnlyWithVisible = reader.Bool();
	bWorkOnlyWithLive = reader.Bool();

	noSwing = reader.Bool();

	Activate(reader.Bool());
	Show(true);
	return true;
}

void WaterTrack::Restart()
{
	pMaster.Reset();

	if( MissionObjectName.NotEmpty() )
		FindObject(MissionObjectName,pMaster);

	ClearTrack();
}

void WaterTrack::ClearTrack()
{
	bPrevPosInited = false;
	bSmoothSpeedInited = false;
	fTime = 0.0f;
	fSmoothSpeed = 0.0f;
	PauseParticles(true);
}

void WaterTrack::PauseParticles(bool bPause)
{
	if( BurunParticle )
		BurunParticle->PauseEmission(bPause);
	if( BurunBackParticle )
		BurunBackParticle->PauseEmission(bPause);
}

void _cdecl WaterTrack::Work(float fDeltaTime, long level)
{
	// обновляем поинтер на привязанный объект
	if( !pMaster.Validate() && MissionObjectName.NotEmpty() )
	{
		FindObject(MissionObjectName,pMaster);
		if( !pMaster.Ptr())
			 MissionObjectName.Empty();
	}

	// объект деактивирован или мертв, значит уходим
	if( !IsActive() || IsDead())
	{
		ClearTrack();
		return;
	}

	// не за кем следить - значит убиваем все и уходим
	if( !pMaster.Ptr() )
	{
		ClearTrack();
		return;
	}

	// проверяем состояние того, за кем наблюдаем
	if( bWorkOnlyWithActive && !pMaster.Ptr()->IsActive() ||
		bWorkOnlyWithVisible && !pMaster.Ptr()->IsShow() ||
		bWorkOnlyWithLive && pMaster.Ptr()->IsDead() )
	{
		ClearTrack();
		return;
	}

	// обновляем позицию
	pMaster.Ptr()->GetMatrix( objectMatrix );
	if(noSwing)
	{
		//Видовая матрица с включёной свинг матрицей
		Matrix view = Render().GetView();
		//Чистая видовая матрица
		view.Inverse();
		Matrix realView = Mission().GetInverseSwingMatrix() * view;
		realView.Inverse();
		//Разносная матрица
		Matrix diff = Matrix(Matrix(realView), view);
		objectMatrix.EqMultiply(objectMatrix, diff);
	}

	backPos = objectMatrix * back_offset;
	curPos = objectMatrix * offset;

	// партиклы
	if( BurunParticle || BurunBackParticle )
	{
		Matrix m = objectMatrix;
		if( BurunParticle )
		{
			m.pos = curPos;
			BurunParticle->SetTransform(m);
		}
		if( BurunBackParticle )
		{
			m.pos = backPos;
			BurunBackParticle->SetTransform(m);
		}
	}

	// перерасчет скорости
	if( fDeltaTime > 0.0f )
	{
		// накапливаем время
		fTime += fDeltaTime;

		float fSpeed = 0.0f;
		// уже двигались ранее
		if( bPrevPosInited )
		{
			if( fTime > 0.01f )
			{
				fSpeed = (vPrevPos - backPos).GetLength() / fTime;
				fTime = 0.0f;
			}
		}
		// только стартовали движение
		else
		{
			PauseParticles(false);
			fTime = 0.f;
		}
		vPrevPos = backPos;
		bPrevPosInited = true;

		// расчет плавной скорости
		if (!bSmoothSpeedInited)
		{
			fSmoothSpeed = 0.f;
			bSmoothSpeedInited = true;
		} else
		{
			fSmoothSpeed = Lerp(fSmoothSpeed,fSpeed,0.1f);
		}
	}

	// скейл для партиклов
	if (BurunParticle || BurunBackParticle)
	{
		float scaleK = Clampf((fSmoothSpeed-3.0f) / 25.0f);
		if( BurunParticle )
			BurunParticle->SetScale(scaleK*m_fParticleScale);
		if( BurunBackParticle )
			BurunBackParticle->SetScale(scaleK*m_fParticleScale);
	}
}

MOP_BEGINLISTCG(WaterTrack, "ShipWaterTrack", '1.00', 101, "Water track for ship", "Arcade Sea");
		MOP_STRINGC("Mission object name", "", "С этого объекта будет считана позиция и угол для пены")

		MOP_STRINGC("Burun particles", "", "Имя партиклов для бурунов с носа корабля")
		MOP_STRINGC("Burun back particles", "", "Имя партиклов для бурунов с кормы корабля")
		MOP_FLOATC("Particles scale", 1.0f, "Для масштабирования партикла под размер корабля")

		MOP_POSITIONC("Offset", Vector(0.f, 0.f, 27.f), "Позиция носовой части корабля");
		MOP_POSITIONC("BackOffset", Vector(0.f, 0.f, -18.f), "Позиция для расчета следа от киля");

		MOP_BOOLC("Work only with active objects", true, "Отображать трейл, только за активными объектами");
		MOP_BOOLC("Work only with visible objects", true, "Отображать трейл, только за видимыми объектами");
		MOP_BOOLC("Work only with live objects", true, "Отображать трейл, только за живыми объектами");

		MOP_BOOLC("No swing", true, "No swing model in swing machine")

		MOP_BOOLC("Active", true, "Активность трейла");
MOP_ENDLIST(WaterTrack)
