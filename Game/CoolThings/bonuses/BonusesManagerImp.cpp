

#include "BonusesManagerImp.h"
#include "BonusesTable.h"

//===========================================================================================================


#define BMIMP_FADESPEED_DROP	1.0f	//Скорость убывания блокирующей альфы для короткоживущих бонусов

#define BMIMP_FLYSPEED			1.0f	//Скорость полёта бонуса при взятии (пролетает 1)

#define BMIMP_WARNINGTIME		2.0f	//Время мигания перед исчезновением
#define BMIMP_FADETIME			0.5f	//Время для эффекта скрывания бонуса

#define BMIMP_FADESPEED_PICKUP	100.0f	//Скорость убывания альфы при взятие бонуса

//#define BMIMP_PLAYERHEIGHT		0.2f	//Высота игрока на которую летит бонус при взятии

#define BMIMP_IMMUNETIME_MIN    0.5f	//Минимальное время с момента рождения в течении которого бонус нельзя взять
#define BMIMP_IMMUNETIME        5.0f	//Время с момента рождения в течении которого бонус нельзя взять

#define BMIMP_MAXBONUSESCOUNT	1000	//Максимальное количество бонусов

#define BMIMP_FASTROTATE_TIME	2.0f	//Время быстрого вращения при выпадании
#define BMIMP_FASTROTATE_SPD	30.0f	//Максимальная скорость вращения при выбрасывании-забирании

//===========================================================================================================

Greedy::Greedy(BonusesManagerImp* pManager, MissionObject* pOwner)
{
	Assert( pManager && pOwner );
	pos = 0.f;
	owner = pOwner;
	manager = pManager;
}

Greedy::~Greedy()
{
}

void Greedy::Release()
{
	if( manager )
		manager->RemoveGreedy( this );
	manager = null;
	delete this;
}


BonusesManagerImp::BonusFxArchive::BonusFxArchive()
{
	prticles = null;
	sound = null;
}

bool BonusesManagerImp::BonusFxArchive::IsDone()
{
	if(prticles)
	{
		if(prticles->IsAlive())
		{
			return false;
		}
		prticles->Release();
		prticles = null;
	}
	if(sound)
	{
		if(sound->IsPlay())
		{
			return false;
		}
		sound->Release();
		sound = null;
	}
	return true;
}

void BonusesManagerImp::BonusFxArchive::ForceRelease()
{
	if(prticles)
	{
		prticles->Release();
		prticles = null;
	}
	if(sound)
	{
		sound->Release();
		sound = null;
	}
}

BonusesManagerImp::BonusFx::BonusFx()
{
	prticles = null;
	sound = null;
	isPlay = false;
}

//
#pragma optimize("", off)

void BonusesManagerImp::BonusFx::Init(IMission & mis, const Matrix & mtx, const BonusBase::FX & fx)
{
	if(fx.particles.NotEmpty())
	{
		Assert(!prticles);
		prticles = mis.Particles().CreateParticleSystemEx2(fx.particles.c_str(), mtx, false, _FL_);
		if(prticles)
		{
			prticles->Restart(rand());
			prticles->PauseEmission(true);
		}
	}
	if(fx.sound.NotEmpty())
	{
		Assert(!sound);
		sound = mis.Sound().Create3D(fx.sound, mtx.pos, _FL_, false, false);
	}	
}

#pragma optimize("", on)

inline void BonusesManagerImp::BonusFx::Udpade(const Matrix & mtx)
{
	if(!isPlay)
	{
		if(prticles)
		{
			prticles->Teleport(mtx);
		}
		if(sound)
		{
			sound->SetPosition(mtx.pos);
		}		
	}else{
		if(prticles)
		{
			prticles->SetTransform(mtx);
		}
		if(sound)
		{
			sound->SetPosition(mtx.pos);
		}
	}
}

inline void BonusesManagerImp::BonusFx::Release()
{
	if(prticles)
	{
		prticles->Release();
		prticles = null;
	}
	if(sound)
	{
		sound->Release();
		sound = null;
	}
}

inline void BonusesManagerImp::BonusFx::Play(bool isOnes)
{
	if(isOnes && isPlay) return;
	isPlay = true;
	if(prticles)
	{
		prticles->PauseEmission(false);
	}
	if(sound)
	{
		sound->Play();
	}	
}

inline bool BonusesManagerImp::BonusFx::IsActive()
{
	if(prticles)
	{
		if(prticles->IsAlive())
		{
			return true;
		}
	}
	if(sound)
	{
		if(sound->IsPlay())
		{
			return true;
		}
	}
	return false;
}

inline void BonusesManagerImp::BonusFx::PutToArchvie(BonusFxArchive & arc)
{
	arc.ForceRelease();
	arc.prticles = prticles;
	arc.sound = sound;
	prticles = null;
	sound = null;
}



inline void BonusesManagerImp::PickuperList::UpdatePickuper(MissionObject* mo)
{
	if( mo )
	{
		dword n;
		// ищем такой в списке
		for (n=0; n<list.Size(); n++)
			if (list[n].obj==mo)
				break;
		// нет в спсике, то добавим туда
		if( n==list.Size() )
		{
			list.Add();
			list[n].obj = mo;
		}
		// обновим данные для этого подборщика
		mo->GetMatrix( list[n].transform );
		list[n].bDeleteFlag = false;
	}
}

bool BonusesManagerImp::PickuperList::UpdateList(IMission& mis)
{
	//Если нет итератора, то заведем новый
	if( !gi ) gi = &mis.GroupIterator(MG_BONUSPICKUPER, __FILE__, __LINE__);
	if( !gi ) return false;
	// подготовить список к удалению
	for (long n=0; n<list; n++)
		list[n].bDeleteFlag = true;
	// обновить список
	for (gi->Reset(); !gi->IsDone(); gi->Next())
		UpdatePickuper( gi->Get() );
	// добавим в спсиок плеера -/-/- Временно!!! для пробы
	UpdatePickuper( mis.Player() );
	// удалить из списка все лишнее
	for (long k=0; k<list; k++)
		if( list[k].bDeleteFlag )
		{
			list.ExtractNoShift(k);
			k--;
		}

	return list.Size()>0;
}

MissionObject* BonusesManagerImp::PickuperList::FindNearestByBonus(Bonus* b)
{
	float dist2pow = 1000.f;
	MissionObject * mo = null;

/*	for(long n=0; n<list; n++)
	{
		// готов ли объект взять этот бонус
		if( !b->object->ReadyForPickup( list[n].obj ) )
			continue;
		
		float curdist2pow = ~(list[n].transform.pos - b->pos);
		if( curdist2pow < dist2pow )
		{
			// проверяем на пересечение
			Vector boxMin, boxMax;
			list[n].obj->GetBox( boxMin, boxMax );
			Vector center = list[n].transform.MulVertexByInverse( b->pos );
			if( Sphere::Intersection(boxMin, boxMax, center, b->pickupRadius) )
			{
				mo = list[n].obj;
				dist2pow = curdist2pow;
			}
		}
	}*/

	return mo;
}



Vector BonusesManagerImp::m_camPos;
//===========================================================================================================

BonusesManagerImp::BonusesManagerImp() : bonuses(_FL_, 256),
                                          drop(_FL_),
										  createdLights(_FL_),
										  activeFx(_FL_),
										  aGreedy(_FL_)
{
	dropAngle = 0.0f;
	bonuses.Reserve(BMIMP_MAXBONUSESCOUNT);
	drop.Reserve(256);
	createdLights.Reserve(256);
	activeFx.Reserve(1024);
	//Источники света для бонусов
	//Ambient из миссии с настроеным освещением
	long ambientR = 95;
	long ambientG = 95;
	long ambientB = 95;
	//Direction из миссии с настроеным освещением
	long directionColorR = 255;
	long directionColorG = 255;
	long directionColorB = 255;
	long directionBackColorR = 255;
	long directionBackColorG = 255;
	long directionBackColorB = 255;
	float directionMultiplier = 4.5f;
	float directionBackMultiplier = 1.0f;
	Vector directionAngles = Vector(43.572f, 342.244f, 0.0f);
	//Point
	//... пока нету
	//Free camera из миссии с настроеным освещением
	Vector cameraPosition = Vector(0.0f, 0.04f, -1.25f);
	Vector cameraAngles = Vector(6.277f, 0.572f, 0.0f);
	//Расчитываем параметры источников
	litAmbientColor = Color(ambientR/255.0f, ambientG/255.0f, ambientB/255.0f, 1.0f);	
	litDirectionColor = Color(directionMultiplier*directionColorR/255.0f, directionMultiplier*directionColorG/255.0f, directionMultiplier*directionColorB/255.0f, 1.0f);
	litDirectionBColor = Color(directionBackMultiplier*directionBackColorR/255.0f, directionBackMultiplier*directionBackColorG/255.0f, directionBackMultiplier*directionBackColorB/255.0f, 1.0f);
	Vector litDirectionInWorldSpace = -(Matrix(directionAngles*(PI/180.0f)).vz);
	Matrix freeCamera(cameraAngles*(PI/180.0f), cameraPosition);
	litDirectionInCamSpace = freeCamera.MulNormalByInverse(litDirectionInWorldSpace);
/*	lights[0].localPos = Vector(-3.0f, 0.0f, -2.0f);
	lights[0].filter = Color(0.6f, 0.6f, 0.6f, 1.0f);
	lights[0].safe = Color(0.1f, 0.1f, 0.1f, 0.1f);
	lights[0].base = Color(0.0f, 0.0f, 0.0f, 0.0f);
	lights[0].radius = 10000000.0f;
	lights[0].light = null;
	lights[1].localPos = Vector(20.0f, 100.0f, 0.0f);
	lights[1].filter = Color(0.1f, 0.1f, 0.12f, 1.0f);
	lights[1].safe = Color(0.2f, 0.2f, 0.2f, 0.1f);
	lights[1].base = Color(0.01f, 0.01f, 0.01f, 1.0f);
	lights[1].radius = 10000000.0f;
	lights[1].light = null;
*/
	needExtendedDraw = false;

	m_nGlowQnt = 0;
}


BonusesManagerImp::~BonusesManagerImp()
{
	for(long i = 0; i < activeFx; i++)
	{
		activeFx[i].ForceRelease();
	}
	activeFx.Empty();
	/*
	for(long i = 0; i < ARRSIZE(lights); i++)
	{
		lights[i].light->Release();
		lights[i].light = null;
	}
	*/
	ReleaseAll();

	for( long n=0; n<aGreedy; n++ )
		aGreedy[n]->ClearManager();
	aGreedy.DelAll();
}

//Инициализировать объект
bool BonusesManagerImp::Create(MOPReader & reader)
{
	SetUpdate(&BonusesManagerImp::Work, ML_ALPHA3);
	//Registry(MG_SHADOWDONTRECEIVE, (MOF_EVENT)&BonusesManagerImp::ShadowDraw);
/*
	for(long i = 0; i < ARRSIZE(lights); i++)
	{
		lights[i].light = Render().CreateLight(0.0f, Color(0.0f, 0.0f, 0.0f, 0.0f), lights[i].radius);
		Assert(lights[i].light);
	}
*/
	Render().GetShaderId("BonusGlow", m_idGlowShader);
	m_nGlowQnt = 0;
	m_pGlowTexture = Render().CreateTexture(_FL_,"bonus_glow_texture.txx");
	m_pGlowTextureVar = Render().GetTechniqueGlobalVariable("RectTexture",_FL_);
	m_nGlowHorz = 2;
	m_fGlowTdU = 0.5f;
	m_fGlowTdV = 0.5f;

	return true;
}

//Создание бонуса
void BonusesManagerImp::CreateBonusInside(const Vector & pos, const ConstString & tableName, DropParams * crParams)
{
	//Получим таблицу
	MOSafePointer obj;
	FindObject(tableName, obj);
	if(!obj.Ptr())
	{
		LogicDebugError("Can't drop bonus, object %s is not BonusesTable", tableName.c_str());
		return;
	}
	MO_IS_IF_NOT(tid, "BonusesTable", obj.Ptr())
	{
		LogicDebugError("Can't drop bonus, object %s is not BonusesTable", tableName.c_str());
		return;
	}
	BonusesTable * table = (BonusesTable *)obj.Ptr();
	//Получим бонус
	if(!table->SelectBonus(drop))
	{
		return;
	}
	//Создаём дропнувшиеся бонусы
	for(long i = 0; i < drop; i++)
	{
		BonusBase * bb = drop[i];
		if(!bb)
		{
			continue;
		}
		long count = bb->DropsCount();
		float dltAngle = count ? (2.0f*PI/count) : 2.0f*PI;		
		for(long j = 0; j < count; j++)
		{
			CreateBonusInside(pos, bb, dltAngle, crParams);
		}
		if(count > 0)
		{
			//Проиграем 1 эффект выпадания для любого количества бонусов одного типа
			BonusFx drop;
			const BonusBase::CreationParams & params = bb->GetCreationParams();
			Matrix initTransform(0.0f, Rnd(2.0f*PI), 0.0f, pos.x, pos.y, pos.z);
			drop.Init(Mission(), initTransform, params.drop);
			drop.Play();
			PutEffectToArchive(drop);
			drop.Release();
		}
	}
	dropAngle = Rnd(2.0f*PI);
}

//Создать пожирателя
IGreedy* BonusesManagerImp::CreateGreedyInside(MissionObject* pOwnerMO)
{
	Greedy* pGreedy = NEW Greedy(this, pOwnerMO);
	if( pGreedy )
		aGreedy.Add( pGreedy );
	return pGreedy;
}

void BonusesManagerImp::RemoveGreedy(IGreedy* pGreedy)
{
	for( dword n=0; n<aGreedy.Size(); n++ )
		if( aGreedy[n] == pGreedy )
			break;
	if( n < aGreedy.Size() )
		aGreedy.DelIndex( n );

	for( n=0; n<bonuses.Size(); n++ )
	{
		if( bonuses[n].greedy == pGreedy )
		{
			bonuses[n].greedy = 0;
			Release( bonuses[n] );
			bonuses.DelIndex( n );
			n--;
		}
	}
}

//Создать бонус
void BonusesManagerImp::CreateBonusInside(const Vector & pos, BonusBase * bonus, float dltAngle, DropParams * crParams)
{
	//Создаём внутренние представление бонуса
	const BonusBase::CreationParams & params = bonus->GetCreationParams();
	Bonus & b = bonuses[bonuses.Add()];
	b.pos = pos;
	b.pickupRadius = params.pickupRadius;
	b.radius = params.radius;
	b.state = s_immune;
	b.ay = Rnd(2.0f*PI);
	b.phase = 0.f;//Rnd(2.0f*PI);
	Matrix initTransform(0.0f, b.ay, 0.0f, pos.x, pos.y, pos.z);
//	b.drop.Init(Mission(), initTransform, params.drop);
	b.wait.Init(Mission(), initTransform, params.wait);
	b.jump.Init(Mission(), initTransform, params.jump);
	b.pickupfly.Init(Mission(), initTransform, params.pickupfly);
	b.pickup.Init(Mission(), initTransform, params.pickup);
	b.fade.Init(Mission(), initTransform, params.fade);
	if( bonus )
		BuildSafePointer( bonus, b.object.GetSPObject() );
	else
		b.object.Reset();
	b.liveTime = crParams ? crParams->skipLifeTime : 0.f;
	b.maxLiveTime = params.liveTime;
	b.flyTime = 0.0f;
	b.alpha = 1.0f;
	b.applyCount = 0;
	b.applyTime = 0.0f;
	if(params.freq > 1e-20f)
	{
		b.applyDltTime = 1.0f/params.freq;
	}else{
		b.applyDltTime = 0.0f;
	}
	b.workTime = 0.0f;
	b.dltTime = 0.0f;
	b.userBonus = bonus->CreateUserBonus();	
	if(!string::IsEmpty(params.modelName))
	{
		models.CreateModel(params.modelName, &Animation(), &Particles(), &Sound(), b.model);
	}
	b.isSleep = false;
	// пока никто не подобрал
	b.greedy = null;

	// параметры создания
	if(!crParams)
	{
		//Проверяем "свободность" места для бонуса
		b.velocity = Vector(0.0f, -1.0f, 0.0f);
		for(long i = 0, copyTo = -1; i < bonuses; i++)
		{
			Bonus & bonus = bonuses[i];
			if(bonus.state == s_immune || bonus.state == s_wait)
			{
				if((bonus.pos - pos).GetLengthXZ2() < 0.2f)
				{
					//Надо бросить бонус в сторну
					b.velocity.MakeXZ(Rnd(2.0f*PI)) *= RRnd(6.5f, 10.0f);
					b.velocity.y = RRnd(2.0f, 4.0f);
					//b.velocity *= RRnd(6.5f, 8.0f);
					//dropAngle += dltAngle*RRnd(0.9f, 1.1f);
					//dropAngle += dltAngle*RRnd(0.3f, 1.8f);
					break;
				}
			}
		}
		b.friction = 5.f;
	}else{
		//Назначаем вектор скорости изходя из заданных параметров
		float da = RRnd(crParams->minAy, crParams->maxAy);
		b.velocity.MakeXZ(da) *= RRnd(crParams->minVxz, crParams->maxVxz);
		b.velocity.y = RRnd(crParams->minVy, crParams->maxVy);
		b.friction = crParams->friction;
	}
	needExtendedDraw = (b.userBonus != null);
	//Запускаем эффекты выпадания и ожидания
//	b.drop.Play();
}

//Удалить все бонусы
void BonusesManagerImp::ResetAllBonusesInside()
{
	drop.Empty();
	for(long i = 0; i < bonuses; i++)
	{
		Release(bonuses[i]);
	}
	bonuses.Empty();
}

//Кэшировать модельку
void BonusesManagerImp::CacheModelInside(const char * modelName)
{
	if(!string::IsEmpty(modelName))
	{
		BonusesModelsManager::Index index;
		models.CreateModel(modelName, &Animation(), &Particles(), &Sound(), index);
		if(!index.IsEmpty())
		{
			models.DeleteModel(index);
		}		
	}
}

//Работа мэнеджера
void _cdecl BonusesManagerImp::Work(float dltTime, long level)
{
	for(long i = 0; i < activeFx; i++)
	{
		if(activeFx[i].IsDone())
		{
			activeFx.DelIndex(i);
			i--;
		}
	}
	//Нет бонусов, ничего не делаем...
	if(!bonuses.Size())
	{
		return;
	}

	// обновим игроков и если нет никого, то и бонусы некому брать
	//TempGreedyFrame();

	Vector playerPos = 0.f;
	bool isAlive = false;

	//Выключаем все источники света
	createdLights.Empty();
	Render().EnumCreatedLights(createdLights);
	for(long i = 0; i < createdLights; i++)
	{
		ILight * & lit = createdLights[i];
		if(lit->isEnabled())
		{
			lit->Enable(false);
		}else{
			lit = null;
		}
	}	
	Color curAmbColor = Render().GetAmbient();
	Color curGlobalColor = Render().GetGlobalLightColor();
	Color curGlobalBColor = Render().GetGlobalLightBackColor();
	Vector curGlobalDir = Render().GetGlobalLightDirection();
	//Настраиваем свой свет
	const Matrix & view = Render().GetView();
	//Амбиент
	Render().SetAmbient(litAmbientColor);
	//Направленый источник
	Vector litDirectionInWorldSpace = view.MulNormalByInverse(litDirectionInCamSpace);
	Render().SetGlobalLight(litDirectionInWorldSpace, false, litDirectionColor, litDirectionBColor);
	//Обрабатываем бонусы
	Vector boxMin, boxMax;
	//player->GetBox(boxMin, boxMax);
	bool userDraws = false;
	m_camPos = view.GetCamPos();
	for(long i = 0, copyTo = -1, count = 0; i < bonuses; i++)
	{
		Bonus & bonus = bonuses[i];
		//Если объект пропал, удалим бонус
		if(!bonus.object.Validate())
		{
			Release(bonus);
			if(copyTo < 0)
			{
				copyTo = i;
			}
			continue;
		}
		//Параметры бонуса
		const BonusBase::CreationParams & params = bonus.object.Ptr()->GetCreationParams();
		//Время идёт всегда
		bonus.liveTime += dltTime;
		//Анимируем позицию бонуса
		float kRotSpd1 = 1.0f - Clampf(bonus.liveTime/BMIMP_FASTROTATE_TIME);
		float kRotSpd2 = Clampf(bonus.flyTime);
		float kRotSpd = coremax(kRotSpd1, kRotSpd2);
		kRotSpd = sinf(kRotSpd*PI*0.5f);
		kRotSpd = kRotSpd*kRotSpd;
		bonus.ay += dltTime*(params.moverotspeed*PIm2 + kRotSpd*BMIMP_FASTROTATE_SPD);
		if(bonus.ay > PI*2.0f) bonus.ay -= PI*2.0f;
		//Альфа создания линейно гаснет		
		bonus.dropAlhpa -= dltTime*BMIMP_FADESPEED_DROP;
		if(bonus.dropAlhpa < 0.0f) bonus.dropAlhpa = 0.0f;
		//Отрабатываем в зависимости от состояния
		if(bonus.state == s_immune || bonus.state == s_wait || bonus.state == s_fade)
		{			
			//Проверяем на возможность взятия
			if(bonus.state == s_immune && bonus.liveTime > BMIMP_IMMUNETIME)
			{
				bonus.state = s_wait;
				bonus.wait.Play();
			}
			IGreedy* pGreedy = null;
			if( bonus.state == s_wait )
				pGreedy = FindNearestGreedy( &bonus );

			if( pGreedy )
			{
				//Предмет взят, начинаем расстворение
				bonus.state = s_pickup_fly;
				bonus.velocity = bonus.pos;
				bonus.pickupfly.Play();
				bonus.greedy = pGreedy;
			}else{
				bool isLive;
				if(WaitBonus(bonus, dltTime, isLive))
				{
					Release(bonus);
					if(copyTo < 0)
					{
						copyTo = i;
					}
					continue;
				}
				if( bonus.isSleep && params.moveamplitude>0.f )
				{
					bonus.phase += dltTime * params.moveliftspeed/params.moveamplitude*PI*0.5f;
					if(bonus.phase > PI*2.0f) bonus.phase -= PI*2.0f;
					bonus.pos.y = bonus.velocity.y + params.moveamplitude*0.5f*(1.f+sinf(bonus.phase));
				}
				if(isLive)
				{
					count ++;
				}
			}
		}
		//Если есть подборщик, то берем его позицию
		if( bonus.greedy )
		{
			if( bonus.greedy->OwnerMO() )
			{
				
				playerPos = bonus.greedy->GetPosition();
				isAlive = !( bonus.greedy->OwnerMO()->IsDead() || bonus.greedy->OwnerMO()->IsDie() );
			}
		}
		if(bonus.state == s_pickup_fly)
		{
			bonus.pos.Lerp(bonus.velocity, playerPos, bonus.flyTime*bonus.flyTime);
			bonus.flyTime += dltTime*BMIMP_FLYSPEED;			
			if(bonus.flyTime >= 1.0f)
			{
				bonus.state = s_pickup_start;
				bonus.flyTime = 1.0f;
				bonus.pickup.Play();
			}			
		}
		//Растворяем постепенно модельку
		if(bonus.state >= s_pickup_start)
		{
			bonus.pos = playerPos;
			if(ApplyBonus(params, bonus, dltTime, isAlive))
			{
				Release(bonus);
				if(copyTo < 0)
				{
					copyTo = i;
				}
				continue;
			}
		}
		//Рисуем модель если есть такая
		if(DrawBonus(params, bonus, dltTime))
		{
			Release(bonus);
			if(copyTo < 0)
			{
				copyTo = i;
			}
			continue;
		}
		// рисуем глоу на модели
		if( params.glowFrame!=-1 )
			DrawGlow(params,bonus,dltTime);

		if(bonus.userBonus)
		{
			userDraws = true;
		}
		//Копируем бонус если требуется уплотнение
		if(copyTo >= 0)
		{
			bonuses[copyTo++] = bonus;
		}
	}
	if(copyTo >= 0)
	{
		bonuses.DelRange(copyTo, bonuses.Size() - 1);
	}
	//Если бонусов больше допустимого числа, помечаем 1 старый на предупреждение
	if(count > BMIMP_MAXBONUSESCOUNT)
	{
		MarkForFadeout();
	}
	//Рисуем модельки
	models.Draw();
	//отрисовываем глоу билборды
	FlushGlow();
	//Востанавливаем источники света
	for(long i = 0; i < createdLights; i++)
	{
		ILight * lit = createdLights[i];
		if(lit)
		{
			lit->Enable(true);
		}
	}
	/*
	for(long i = 0; i < ARRSIZE(lights); i++)
	{
		lights[i].light->SetOn(false);
	}*/
	Render().SetAmbient(curAmbColor);
	Render().SetGlobalLight(curGlobalDir, false, curGlobalColor, curGlobalBColor);
	needExtendedDraw = userDraws;
}

//Нарисовать модельки для тени
void _cdecl BonusesManagerImp::ShadowDraw(const char * group, MissionObject * sender)
{
	if(needExtendedDraw)
	{
		for(long i = 0; i < bonuses; i++)
		{
			Bonus & bonus = bonuses[i];
			if(bonus.userBonus)
			{
				const BonusBase::CreationParams & params = bonus.object.Ptr()->GetCreationParams();
				DrawBonus(params, bonus, 0.0f);
			}
		}
	}
	models.Draw();
}

//Состояние ожидания взятия
inline bool BonusesManagerImp::WaitBonus(Bonus & bonus, float dltTime, bool & isLive)
{
	isLive = false;
	//Перемещаем бонус до нахождения точки опоры
	if(!bonus.isSleep)
	{
		if(bonus.pos.y < -1000.0f)
		{
			return true;
		}
		const float splitDltTime = 0.01f;
		for(bonus.dltTime += dltTime; bonus.dltTime > splitDltTime; bonus.dltTime -= splitDltTime)
		{
			MoveBonus(bonus, splitDltTime);
			if(bonus.isSleep)
			{
				// после усыпления бонуса применяем качание бонуса (запоминаем позицию в уже неиспользуемой переменной - скорость)
				bonus.velocity = bonus.pos;
				bonus.phase = 1.5f*PI;
				break;
			}
		}
	}

	//Проверяем время жизни
	if(bonus.maxLiveTime > 1e-10f)
	{
		if(bonus.liveTime + BMIMP_WARNINGTIME >= bonus.maxLiveTime)
		{
			//Мигаем предупреждением
			float k = 1.0f - (bonus.maxLiveTime - bonus.liveTime)*(1.0f/BMIMP_WARNINGTIME);
			//Переодически плавающий коэфициент 0..1
			bonus.alpha = 0.5f*(1.0f + cosf(k*sqrtf(k)*20.0f*2.0f*PI));

			//bonus.alpha = 0.4f*bonus.alpha + 0.2f;

			float fade = 0.8f*sqrtf(k);
			bonus.alpha = fade + bonus.alpha*(1.0f - fade);
			bonus.alpha = bonus.alpha + bonus.dropAlhpa;
			if(bonus.liveTime + BMIMP_FADETIME >= bonus.maxLiveTime)
			{
				bonus.state = s_fade;
				if(bonus.liveTime >= bonus.maxLiveTime)
				{
					return true;
				}
				float kAlpha = (bonus.maxLiveTime - bonus.liveTime)*(1.0f/BMIMP_FADETIME);
				bonus.alpha *= kAlpha*kAlpha;
				bonus.fade.Play();
			}
		}else{
			isLive = true;
		}
	}else{
		//Не позволим времени утопать слишком далеко в вечном бонусе
		const float maxTime = 24.0f*60.0f*60.0f;
		if(bonus.liveTime > maxTime)
		{
			bonus.liveTime = maxTime;
		}
		isLive = true;
	}
	return false;
}

//Переместить бонус
inline void BonusesManagerImp::MoveBonus(Bonus & bonus, float dltTime)
{
	//					Render().Print(bonus.pos + Vector(0.0f, 1.0f, 0.0f), 100.0f, 0.0f, 0xffff00ff, "(%f, %f, %f)", bonus.velocity.x, bonus.velocity.y, bonus.velocity.z);
	//Пересчитываем скорости для следующего кадра
	bonus.velocity.FrictionXZ(bonus.friction*dltTime, 1.f);
	bonus.velocity.y -= 9.8f*dltTime;

	//Отрезок перемещения
	Vector delta = bonus.velocity*dltTime;

	//Точка на краю сферы в направлении движения в системе бонуса
	Vector edge = bonus.velocity;
	edge.Normalize();
	edge *= bonus.radius;
	//					Render().DrawLine(bonus.pos, 0xff0000ff, newPos, 0xffff0000);
	Vector cp(0.0f), cn(0.0f);
	bool isCollision = false;

	// бонус уже в коллидере
	if(bonus.object.Ptr()->CollisionLine(bonus.pos, bonus.pos - Vector(0.0f, bonus.radius, 0.0f), cp, cn))
	{
		if( (cn | delta) < 0.f )
		{
			isCollision = true;
			bonus.pos = cp + Vector(0.0f, bonus.radius + 0.01f, 0.0f);
		}
	}

	//Точка, до которой проверяем наличие препятствий
	Vector newPos = bonus.pos + edge + delta;
	if(!isCollision && bonus.object.Ptr()->CollisionLine(bonus.pos, newPos, cp, cn))
	{
		if( (cn | delta) < 0.f )
			isCollision = true;
	}

	if( isCollision )
	{
		//В зависимости от скорости решаем двигатся дальше или заснуть
		cn.Normalize();
		if(bonus.state == s_immune)
		{
			if(cn.y > 0.5f && bonus.velocity.y < 0.0f)
			{
				if(bonus.liveTime > BMIMP_IMMUNETIME_MIN)
				{
					bonus.state = s_wait;
				}
			}
		}
		if(cn.y > 0.5f && bonus.velocity.GetLengthXZ2() < 0.045f && fabsf(bonus.velocity.y) < 0.8f)
		{
			//Пора заснуть
			//bonus.pos = cp - edge;
			/*
			if(bonus.liveTime > BMIMP_IMMUNETIME*0.5f && bonus.liveTime < BMIMP_IMMUNETIME)
			{
				bonus.liveTime = BMIMP_IMMUNETIME;
			}*/
			bonus.isSleep = true;
		}else{
			//Отскакиваем
			bonus.velocity.Reflection(cn);
			if(cn.y > 0.0f && bonus.velocity.y < 0.0f)
			{
				bonus.velocity.y = -bonus.velocity.y;
			}
			if(cn.y > 0.5f)
			{
				bonus.velocity.y *= (1.5f - cn.y)*1.2f;
			}
			bonus.jump.Play(false);
		}
	}else{
		//Перемещаемся в выбраном направлении
		bonus.pos += delta;
	}	
}

//Нарисовать бонус
inline bool BonusesManagerImp::DrawBonus(const BonusBase::CreationParams & params, Bonus & bonus, float dltTime)
{
	//Текущее положение
	float dy = params.moveheight*(1.f-Clampf(bonus.flyTime));//0.1f*(1.0f + sinf(bonus.phase*2.0f));
	Matrix m(0.0f, bonus.ay, 0.0f, bonus.pos.x, bonus.pos.y + dy, bonus.pos.z);
	//Масштаб
	float scale = params.modelscale;//1.0f;//powf(Clampf(1.0f - bonus.flyTime), 0.7f);
	float alpha = bonus.alpha;
	//Обновим позиции эфектов	
//	bonus.drop.Udpade(m);
	bonus.wait.Udpade(m);
	bonus.jump.Udpade(m);
	bonus.pickupfly.Udpade(m);
	bonus.pickup.Udpade(m);
	bonus.fade.Udpade(m);
	//Рисуем
	bool result = false;
	if(bonus.userBonus)
	{
		BonusBase::UserBonus::UpdateParams userParams;
		userParams.dltTime = dltTime;
		userParams.mtx = m;
		userParams.alpha = bonus.alpha;
		userParams.isLive = true;
		userParams.render = &Render();
		userParams.alpha = alpha;
		userParams.scale = scale;
		bonus.userBonus->Update(userParams);
		result = !userParams.isLive;
		alpha = userParams.alpha;
		scale = userParams.scale;
	}
	Matrix mtx(params.localModelTransform, m);
	models.SetTransform(bonus.model, mtx, alpha, scale);

	return result;
}

//Применить бонус
inline bool BonusesManagerImp::ApplyBonus(const BonusBase::CreationParams & params, Bonus & bonus, float dltTime, bool isAlive)
{
	//Расстворяем модель
	bool noModel = true;
	if(!bonus.model.IsEmpty())
	{
		//!!! Fix me Assert(bonus.alpha <= 1.1f);
		if(bonus.alpha >= 1.0f)
		{
			bonus.alpha = 1.0f;
		}
		bonus.alpha -= dltTime*BMIMP_FADESPEED_PICKUP;
		if(bonus.alpha > 0.0f)
		{
			noModel = false;
		}else{
			models.DeleteModel(bonus.model);
		}
	}
	if(bonus.state == s_pickup_fade)
	{
		return true;
	}
	//Применяем бонус
	bool isEnd = false;
	if(bonus.state == s_pickup_start)
	{
		if(isAlive)
		{
			//if(!bonus.object->Apply(bonus.greedy->OwnerMO()))
			if(!bonus.object.Ptr()->Apply(bonus.greedy))
			{
				isEnd = true;
			}
		}else{
			isEnd = true;
		}
		bonus.state = s_pickup;
		bonus.applyCount++;
	}
	//Проверка счётчика
	if(params.applyCount > 0)
	{
		if(bonus.applyCount >= params.applyCount)
		{
			isEnd = true;
		}
	}
	//Проверка времени работы
	if(params.workTime > 1e-10f)
	{
		if(bonus.workTime >= params.workTime)
		{
			isEnd = true;
		}
	}
	// > Повторные применения бонуса <
	if(params.freq > 1e-10f)
	{
		float dlt = 1.0f/params.freq;
		bonus.applyTime += dltTime;
		while(bonus.applyTime > bonus.applyDltTime && !isEnd)
		{					
			if(isAlive)
			{
				//if(!bonus.object->Apply(bonus.greedy->OwnerMO()))
				if(!bonus.object.Ptr()->Apply(bonus.greedy))
				{
					isEnd = true;
				}
			}else{
				isEnd = true;
			}
			bonus.applyTime -= bonus.applyDltTime;
			if(params.applyCount > 0)
			{
				bonus.applyCount++;
				if(bonus.applyCount >= params.applyCount)
				{
					isEnd = true;
				}
			}
		}				
	}else{
		isEnd = true;
	}
	return isEnd && noModel;
}

//Удалить бонус
void BonusesManagerImp::Release(Bonus & bonus)
{
	//Перенесём некоторые живые эфферты в буфер доигрыванияъ
	PutEffectToArchive(bonus.pickupfly);
	PutEffectToArchive(bonus.pickup);
	PutEffectToArchive(bonus.fade);
	//Очищаем бонус
	bonus.pos = 0.0f;
	bonus.velocity = 0.0f;
	bonus.pickupRadius = 0.0f;
	bonus.radius = 0.0f;
	bonus.state = s_empty;
	bonus.ay = 0.0f;
	bonus.phase = 0.0f;
	models.DeleteModel(bonus.model);
//	bonus.drop.Release();
	bonus.wait.Release();
	bonus.jump.Release();
	bonus.pickupfly.Release();
	bonus.pickup.Release();
	bonus.fade.Release();
	bonus.object.Reset();
	bonus.liveTime = 0.0f;
	bonus.maxLiveTime = 0.0f;
	bonus.flyTime = 0.0f;
	bonus.dropAlhpa = 0.0f;
	bonus.alpha = 0.0f;
	bonus.applyCount = 0;
	bonus.applyTime = 0.0f;
	bonus.applyDltTime = 0.0f;
	bonus.workTime = 0.0f;
	bonus.dltTime = 0.0f;
	bonus.isSleep = false;
	if(bonus.userBonus)
	{
		bonus.userBonus->Release();
		bonus.userBonus = null;
	}
}

//Удалить все бонусы
void BonusesManagerImp::ReleaseAll()
{
	for(long i = 0; i < bonuses; i++)
	{
		Release(bonuses[i]);
	}
	bonuses.Empty();

	RELEASE(m_pGlowTexture);
}

//Отметить бонус на растворение
void BonusesManagerImp::MarkForFadeout()
{
	float minIndexTime = -1.0f;
	for(long i = 0, minIndex = -1; i < bonuses; i++)
	{
		Bonus & bonus = bonuses[i];
		if(bonus.state == s_wait)
		{
			const BonusBase::CreationParams & params = bonus.object.Ptr()->GetCreationParams();
			if(bonus.maxLiveTime > 1e-10f)
			{
				if(bonus.liveTime + BMIMP_WARNINGTIME < bonus.maxLiveTime)
				{
					if(minIndex < 0 || minIndexTime < bonus.liveTime)
					{
						minIndex = i;
						minIndexTime = bonus.liveTime;
					}
				}
			}else{
				if(minIndex < 0)
				{
					minIndex = i;
				}
			}
		}
	}
	if(minIndex >= 0)
	{
		Bonus & bonus = bonuses[minIndex];
		const BonusBase::CreationParams & params = bonus.object.Ptr()->GetCreationParams();
		if(bonus.maxLiveTime > 0.0f)
		{
			bonus.liveTime = bonus.maxLiveTime - BMIMP_WARNINGTIME*0.999999f;
		}else{
			bonus.maxLiveTime = BMIMP_WARNINGTIME;
			bonus.liveTime = 0.0001f;
		}		
	}
}

//Перенести эффект в архив
void BonusesManagerImp::PutEffectToArchive(BonusFx & fx)
{
	if(fx.IsActive())
	{
		if(activeFx.Size() >= 1024)
		{
			activeFx[0].ForceRelease();
			activeFx.DelIndex(0);
		}
		fx.PutToArchvie(activeFx[activeFx.Add()]);
	}
}

IGreedy* BonusesManagerImp::FindNearestGreedy(Bonus* b)
{
	float dist2pow = 1000000.f;
	IGreedy* pgreedy = null;
	float pickup_dist2pow = b->pickupRadius * b->pickupRadius;

	for(long n=0; n<aGreedy; n++)
	{
		if( !aGreedy[n]->OwnerMO() ) continue;

		// готов ли объект взять этот бонус
		if( !b->object.Ptr()->ReadyForPickup( aGreedy[n] ) )
			continue;

		// объект ближе чем уже найденный
		float curdist2pow = ~(aGreedy[n]->Position() - b->pos);
		if( curdist2pow > dist2pow ) continue;

		// проверяем на пересечение
		if( curdist2pow > pickup_dist2pow ) continue;

		// текущий пожиратель ближайший
		pgreedy = aGreedy[n];
		dist2pow = curdist2pow;
	}

	return pgreedy;
}

void BonusesManagerImp::DrawGlow(const BonusBase::CreationParams & params, Bonus & bonus,float dltTime)
{
	long n = m_nGlowQnt * 6;

	Vector pos = bonus.pos;
	pos.y += params.moveheight*(1.f-Clampf(bonus.flyTime));
	pos += params.localModelTransform.pos;

	Vector vdir = m_camPos - pos;
	vdir.Normalize();
	Vector vside = vdir ^ Vector(0.f,1.f,0.f);
	if( vside.Normalize() == 0.f )
		vside = Vector(0.f,0.f,1.f);
	Vector vup = vdir ^ vside;
	pos += vdir * bonus.radius;

	m_pGlowVertex[n].pos = m_pGlowVertex[n+3].pos = pos + (vup+vside) * params.glowSize;
	m_pGlowVertex[n+1].pos = pos + (vup-vside) * params.glowSize;
	m_pGlowVertex[n+2].pos = m_pGlowVertex[n+4].pos = pos - (vside+vup) * params.glowSize;
	m_pGlowVertex[n+5].pos = pos + (vside-vup) * params.glowSize;

	m_pGlowVertex[n].u = m_pGlowVertex[n+3].u = m_pGlowVertex[n+5].u = (params.glowFrame % m_nGlowHorz) * m_fGlowTdU;
	m_pGlowVertex[n+1].u = m_pGlowVertex[n+2].u = m_pGlowVertex[n+4].u = m_pGlowVertex[n].u + m_fGlowTdU;

	m_pGlowVertex[n].v = m_pGlowVertex[n+1].v = m_pGlowVertex[n+3].v = (params.glowFrame / m_nGlowHorz) * m_fGlowTdV;
	m_pGlowVertex[n+2].v = m_pGlowVertex[n+4].v = m_pGlowVertex[n+5].v = m_pGlowVertex[n].v + m_fGlowTdV;

	m_pGlowVertex[n].col =
	m_pGlowVertex[n+1].col =
	m_pGlowVertex[n+2].col =
	m_pGlowVertex[n+3].col =
	m_pGlowVertex[n+4].col =
	m_pGlowVertex[n+5].col = params.glowColor;

	m_nGlowQnt++;
	if( m_nGlowQnt == nMaxGlowVertexQnt )
		FlushGlow();
}

void BonusesManagerImp::FlushGlow()
{
	if( m_nGlowQnt > 0 )
	{
		if( m_pGlowTextureVar )
			m_pGlowTextureVar->SetTexture(m_pGlowTexture);
		Matrix mOldWorld = Render().GetWorld();
		Render().SetWorld( Matrix() );
		Render().DrawPrimitiveUP(m_idGlowShader, PT_TRIANGLELIST, m_nGlowQnt*2, m_pGlowVertex, sizeof(GlowVertex));
		Render().SetWorld( mOldWorld );
	}
	m_nGlowQnt = 0;
}

// временная функция для теста
/*void BonusesManagerImp::TempGreedyFrame()
{
	MGIterator* gi = &Mission().GroupIterator(MG_BONUSPICKUPER, __FILE__, __LINE__);
	if( !gi ) return;

	// обновить список
	for (gi->Reset(); !gi->IsDone(); gi->Next())
	{
		// ищем в списке пожирателя или добавляем нового
		for( dword i=0; i<aGreedy.Size(); i++ )
			if( aGreedy[i]->OwnerMO() == gi->Get() )
				break;
		if( i==aGreedy.Size() )
			BonusesManager::CreateGreedy( gi->Get() );

		// обновляем позицию
		if( i<aGreedy.Size() )
		{
			MissionObject * pmo = gi->Get();
			Matrix m(true);
			pmo->GetMatrix( m );
			aGreedy[i]->Position() = m * Vector(0.f,1.f,0.f);
		}
	}

	RELEASE( gi );
}*/

MOP_BEGINLIST(BonusesManagerImp, "", '1.00', 0)
MOP_ENDLIST(BonusesManagerImp)
