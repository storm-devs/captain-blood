#include "Stalactite.h"
#include "..\..\..\Common_h\corecmds.h"



Stalactite::Stalactite()
{
	fCurrentRandTime = 0.0f;
	activateRadius = 10.0f;
	bShowActivator = true;

	bTryToFind = false;
	height = 0.0f;
	velocity = 0.0f;
	vibrationTime = 0.0f;
	ang = 0.0f;
	state = STATE_IDLE;

	GMXModel = NULL;

	TotalVibrationTime = 1.0f;
}

Stalactite::~Stalactite()
{
	if (GMXModel)
	{
		GMXModel->Release();
		GMXModel = NULL;
	}
}




//Создание объекта
bool Stalactite::Create(MOPReader & reader)
{
	EditMode_Update (reader);
	return true;
}


//Обновление параметров
bool Stalactite::EditMode_Update(MOPReader & reader)
{
	if (GMXModel)
	{
		GMXModel->Release();
		GMXModel = NULL;
	}

	start_pos = reader.Position();
	pos = start_pos;

	GMXModel = Geometry().CreateScene(reader.String().c_str(), &Animation(), &Particles(), &Sound(), _FL_);

	if (GMXModel)
	{
		GMXModel->SetDynamicLightState(true);

		height = fabsf (GMXModel->GetLocalBound().vMin.y);

		sizeX = 0.5f * fabsf (GMXModel->GetLocalBound().vMin.x - GMXModel->GetLocalBound().vMax.x);
		sizeZ = 0.5f * fabsf (GMXModel->GetLocalBound().vMin.z - GMXModel->GetLocalBound().vMax.z);

	} else
	{
		height = 0.5f;
		sizeX = 1.0f;
		sizeZ = 1.0f;
	}
	

	target = reader.String().c_str();

	fDamage = reader.Float();

	targetPtr.Reset();
	bTryToFind = true;

	timeFromLastDamage = 999.0f;

	TotalVibrationTime = reader.Float();
	fRandTime = reader.Float();

	waterSplashSFX = reader.String().c_str();
	vibrationSFX = reader.String().c_str();

	endTrigger.Init(reader);

	activateRadius = reader.Float();
	bShowActivator = reader.Bool();


	Activate(reader.Bool());
	Show(reader.Bool());

	fCurrentRandTime = RRnd(0.0f, fRandTime);
	return true;
}


//Получить матрицу объекта
Matrix & Stalactite::GetMatrix(Matrix & mtx)
{
	return mtx.Build(Vector(0.0f), pos);
}

//Получить размеры описывающего ящика
void Stalactite::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	if(GMXModel)
	{
		min = GMXModel->GetLocalBound().vMin;
		max = GMXModel->GetLocalBound().vMax;
		return;
	}
	min = -0.3f;
	max = 0.3f;
}


void _cdecl Stalactite::Realize(float fDeltaTime, long level)
{
/*
	static float time = 0;

	time+=fDeltaTime;

	Matrix m = Matrix(Vector(0.0f), Vector(6.0f * sin(time*4.0f), 6.0f * cos(time*4.0f), 0.0f));
	Render().DrawSphere(m.pos, 0.1f);


	if (GetAsyncKeyState('1') < 0)
	{
		CoreCommand_GetMemStat memStat;
		api->ExecuteCoreCommand(memStat);


		dword d;
		RDTSC_B(d);
		Particles().CreateParticleSystemEx2("BloodSplash", m, true, _FL_);
		RDTSC_E(d);

		CoreCommand_GetMemStat memStat2;
		api->ExecuteCoreCommand(memStat2);

		dword dwAlloc = memStat2.allocsPerFrame - memStat.allocsPerFrame;
		dword dwDeletes = memStat2.deletesPerFrame - memStat.deletesPerFrame;


		Render().Print(0, 0,  0xFFFF00FF, "Create time: %d a:%d, d:%d", d, dwAlloc, dwDeletes);
	}

*/

	if (targetPtr.Ptr() == null && bTryToFind)
	{
		FindObject(ConstString(target.c_str()), targetPtr);
		static const ConstString tid("DamageReceiver");
		if (targetPtr.Ptr() && !targetPtr.Ptr()->Is(tid))
		{
			targetPtr.Reset();
			bTryToFind = false;
		}
	} else
	{
		if (!targetPtr.Validate())
		{
			targetPtr.Reset();
			bTryToFind = false;
		}
	}

	if (targetPtr.Ptr())
	{
		Matrix m;
		targetPtr.Ptr()->GetMatrix(m);
		if ((m.pos - start_pos).GetLengthXZ() < activateRadius)
		{
			if (state == STATE_IDLE)
			{
				state = STATE_VIBRATION;
				fCurrentRandTime = RRnd(0.0f, fRandTime);
			}
		}
	}


	if (EditMode_IsOn())
	{
		state = STATE_IDLE;

		if (bShowActivator)
		{
			if (!targetPtr.Ptr())
			{
				Render().DrawSphere(pos, activateRadius, 0xFFFF0000);
			} else
			{
				Render().DrawSphere(pos, activateRadius, 0xFF00FF00);
			}
		}
	}

	if (state == STATE_DIE)
	{
		return;
	}

	


	switch (state)
	{
	case STATE_VIBRATION:
		velocity = 0.0f;
		ang.Rand(-0.07f*(vibrationTime / TotalVibrationTime), 0.07f*(vibrationTime / TotalVibrationTime));
		vibrationTime += fDeltaTime;
		if ((vibrationTime+fCurrentRandTime) >= TotalVibrationTime)
		{
			state = STATE_DROP;
		}
		break;
	case STATE_DROP:
		ang = 0.0f;
		static const Vector Acceleration = Vector (0.0f, -98.0f, 0.0f);
		velocity += Acceleration * fDeltaTime;
		pos += velocity * fDeltaTime;

		if (targetPtr.Ptr())
		{
			DamageReceiver* objectToDamage = (DamageReceiver*)targetPtr.Ptr();

			float damage = fDamage;

			
			bool bDamagePassed = false;
			if (timeFromLastDamage < 1.0f)
			{
				bDamagePassed = true;
			}


			if (!bDamagePassed) bDamagePassed = objectToDamage->Attack(this, -1, damage, pos + Vector(0.0f, 0.0f, 0.0f), pos + Vector(0.0f, -height, 0.0f));
			if (!bDamagePassed) bDamagePassed = objectToDamage->Attack(this, -1, damage, pos + Vector(sizeX, 0.0f, sizeZ), pos + Vector(sizeX, -height, sizeZ));
			if (!bDamagePassed) bDamagePassed = objectToDamage->Attack(this, -1, damage, pos + Vector(-sizeX, 0.0f, sizeZ), pos + Vector(-sizeX, -height, sizeZ));
			if (!bDamagePassed) bDamagePassed = objectToDamage->Attack(this, -1, damage, pos + Vector(sizeX, 0.0f, -sizeZ), pos + Vector(sizeX, -height, -sizeZ));
			if (!bDamagePassed) bDamagePassed = objectToDamage->Attack(this, -1, damage, pos + Vector(-sizeX, 0.0f, -sizeZ), pos + Vector(-sizeX, -height, -sizeZ));

			if (bDamagePassed)
			{
				timeFromLastDamage = 0.0f;
			} 
			timeFromLastDamage += fDeltaTime;

/*
			Render().DrawVector(pos + Vector(0.0f, 0.0f, 0.0f), pos + Vector(0.0f, -height, 0.0f), 0xFFFFFFFF);
			Render().DrawVector(pos + Vector(sizeX, 0.0f, sizeZ), pos + Vector(sizeX, -height, sizeZ), 0xFFFFFFFF);
			Render().DrawVector(pos + Vector(-sizeX, 0.0f, sizeZ), pos + Vector(-sizeX, -height, sizeZ), 0xFFFFFFFF);
			Render().DrawVector(pos + Vector(sizeX, 0.0f, -sizeZ), pos + Vector(sizeX, -height, -sizeZ), 0xFFFFFFFF);
			Render().DrawVector(pos + Vector(-sizeX, 0.0f, -sizeZ), pos + Vector(-sizeX, -height, -sizeZ), 0xFFFFFFFF);
*/

		}

		if ((pos.y - height) <= 0.0f)
		{
			//тут надо всплеск воды рождать

			Particles().CreateParticleSystemEx2(waterSplashSFX.c_str(), Matrix(0.0f, Vector(pos.x, 0.0f, pos.z)), true, _FL_);
			
		}

		if (pos.y <= -1.0f)
		{
			state = STATE_DIE;
			endTrigger.Activate(Mission(), false, this);
		}
		break;
	}

	if (GMXModel)
	{
		GMXModel->SetTransform(Matrix(ang, pos));
		GMXModel->Draw();
	} else
	{
		Render().DrawSphere(pos, 1.0f, 0xFFFF0000);
	}

	
}


void Stalactite::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if (isActive)
	{
		fCurrentRandTime = RRnd(0.0f, fRandTime);
		vibrationTime = 0.0f;
		timeFromLastDamage = 999.0f;
		pos = start_pos;
		state = STATE_IDLE;

		if (!EditMode_IsOn())
		{
			Particles().CreateParticleSystemEx2(vibrationSFX.c_str(), Matrix(0.0f, pos), true, _FL_);
		}
	} else
	{
		state = STATE_SLEEP;
	}
}


void Stalactite::Show(bool isShow)
{
	MissionObject::Show(isShow);
	
	if (isShow)
	{
		SetUpdate((MOF_UPDATE)&Stalactite::Realize, ML_PARTICLES2);
	} else
	{
		DelUpdate((MOF_UPDATE)&Stalactite::Realize);
	}
}



MOP_BEGINLISTC(Stalactite, "Stalactite", '1.00', 10000, "Stalactite")
	MOP_POSITION ("Position", Vector(0.0f));
	MOP_STRINGC ("GMX Model", "", "Модель сталактита");

	MOP_STRINGC ("Target", "Player", "Цель сталактита");
	MOP_FLOATC("Damage", 100, "Сколько снимать HP в секунду при попадании по цели");


	MOP_FLOATEX ("Vibration time", 3.0f, 0.01f, 10000.0f);
	MOP_FLOATC ("Random time", 0.0f, "Случайное время добавляемое к Vibration time")

	MOP_STRINGC ("Water Splash Effect", "", "Эффект падения в воду");
	MOP_STRINGC ("Vibration Effect", "", "Эффект рождаемый при вибрировании");

	MOP_MISSIONTRIGGER("End Trigger");

	MOP_FLOATEX("Auto activate radius", 20.0f, 1.0f, 100000.0f);
	MOP_BOOL("Show activate sphere", true);

	MOP_BOOL("Active", false);
	MOP_BOOL("Visible", true);


MOP_ENDLIST(Stalactite)
