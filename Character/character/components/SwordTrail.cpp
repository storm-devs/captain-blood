//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// Character
//===========================================================================================================================
// SwordTrail
//===========================================================================================================================

#include "swordtrail.h"
#include "..\Character.h"


#define STRAIL_FADETIME		 0.2f	//Время погасания элемента шлейфа sec
#define STRAIL_SPLITTIME	 0.1f	//Время рассечения шлейфа на дополнительные элементы sec
#define STRAIL_SPLITDIST	 0.3f	//Дистанция рассечения шлейфа на дополнительные элементы meters
#define STRAIL_SPLITSEGMENTS 0.1f	//Дистанция рассечения шлейфа на дополнительные элементы meters
#define STRAIL_MAXELEMENTS	 96		//Максимально возможное количество элементов в шлейфе
#define STRAIL_SPLDISTMAX	 10.0f	//Максимально возможная дистанция при которой удаляется след
#define STRAIL_TIMESPAWEFF	 0.5f	//Время после которого можно роджить новый партикл от енвайромента


//Счётчик шлейфов
dword SwordTrail::counter = 0;
//Общая текстура
IBaseTexture * SwordTrail::texture = null;
//Общий буфер вершин
IVBuffer * SwordTrail::vbuffer = null;
// Шейдеры
ShaderId SwordTrail::techBlend_id;
ShaderId SwordTrail::techAdd_id;
ShaderId SwordTrail::techMultiply_id;

SwordTrail::SwordTrail() : trail(_FL_,256)
{
	//lstart = lend = null;
	scene = null;
	isStartTrace = false;
	splitTime = 0.0f;
	color = 0xffffffff;
	chr = null;
	localTexture = null;
	releaseTime = 1.0f;	
	effectTable = null;
	shader_id = null;
}

SwordTrail::~SwordTrail()
{
	Release();
}

//Инициализировать
bool SwordTrail::Init(Character * _chr, IGMXScene * model, dword color, const char * uniqTexture, const char * technique, float trailPower)
{
	Assert(_chr);
	if (!counter)
	{
		_chr->Render().GetShaderId("Bullet_Trace", techBlend_id);
		_chr->Render().GetShaderId("SwordTrailAdd", techAdd_id);
		_chr->Render().GetShaderId("SwordTrailMultiply", techMultiply_id);
		//Текстура для следов
		texture = _chr->Render().CreateTexture(_FL_, "SwordTrail");
		//Буфер вершин для следов
		vbuffer = _chr->Render().CreateVertexBuffer(STRAIL_MAXELEMENTS*2*sizeof(Vertex), sizeof(Vertex), _FL_, USAGE_WRITEONLY | USAGE_DYNAMIC, POOL_DEFAULT);
		if(!vbuffer)
		{
			RELEASE(texture);
			return false;
		}
	}
	if(!chr)
	{
		chr = _chr;
		counter++;
	}
	SetModel(model);
	this->color = color;
	this->trailPower = trailPower;
	if(uniqTexture)
	{
		IBaseTexture * new_tex = _chr->Render().CreateTexture(_FL_, uniqTexture);
		RELEASE(localTexture);
		localTexture = new_tex;
	}

	Assert(technique && technique[0]);

	switch (technique[0])
	{
		case 'B':	shader_id = techBlend_id;		break;
		case 'A':	shader_id = techAdd_id;			break;
		case 'M':	shader_id = techMultiply_id;	break;
	}

	return true;
}

//Освободить ресурсы
void SwordTrail::Release()
{
	if (scene)
	{
		if (lstart.isValid())
			scene->Release();

		if (lend.isValid())
			scene->Release();
	}

	RELEASE(localTexture);

	if(chr)
	{
		Assert(counter > 0);
		if(--counter == 0)
		{
			RELEASE(texture);
			RELEASE(vbuffer);
		}
		chr = null;
	}
}

//Начать слежение за локаторами
void SwordTrail::StartTrace()
{
	if(isStartTrace || !chr) return;
	isStartTrace = true;
	trail.Empty();
	splitTime = 0.0f;
	releaseTime = 0.0f;
}

//Закончить слежение за локаторами
void SwordTrail::StopTrace()
{
	isStartTrace = false;
}

//Рисование, обновление следа
void SwordTrail::Draw(const Matrix & toWorld, const Matrix & itemPosition, float dltTime, bool lostPosition)
{
	if(!chr || !lstart.isValid() || !lend.isValid()) return;
	if(releaseTime >= 0.999f)
	{
		return;
	}

	//Рисуем след	
	Update(itemPosition, dltTime, lostPosition);

	if(trail < 2) return;
	
	Assert(trail <= STRAIL_MAXELEMENTS);
	Vertex * vrt = (Vertex *)vbuffer->Lock(0, 0, LOCK_DISCARD/*0, trail*2*sizeof(Vertex), 0*/);
	if(!vrt) return;
	float kU = 1.0f/float(trail - 1);
	dword alpha = long(Clampf(1.0f - releaseTime)*255.0f) << 24;
	color = (color & 0x00ffffff) | alpha;
	for(long i = 0; i < trail; i++, vrt += 2)
	{
		Element & el = trail[i];
		float u = i*kU/* + 1.5f*releaseTime*/;
		vrt[0].p = el.s;
		vrt[0].c = color;
		vrt[0].u = u;
		vrt[0].v = 1.0f;
		vrt[1].p = el.e;
		vrt[1].c = color;
		vrt[1].u = u;
		vrt[1].v = 0.0f;
	}
	vbuffer->Unlock();	
	
	chr->arbiter->gmRData.vTex->SetTexture((localTexture) ? localTexture : texture);
	chr->arbiter->gmRData.vPower->SetFloat(trailPower);
	chr->Render().SetStreamSource(0, vbuffer, sizeof(Vertex));
	chr->Render().SetWorld(toWorld);
	//chr->Render().SetWorld(Matrix());

	chr->Render().DrawPrimitive(shader_id, PT_TRIANGLESTRIP, 0, (trail - 1)*2);
	chr->Render().SetStreamSource(0, null, 0);
	
}

//Обновление следа без рисования
void SwordTrail::Update(const Matrix & itemPosition, float dltTime, bool lostPosition)
{
	if(!chr || !lstart.isValid() || !lend.isValid()) return;
	if(releaseTime >= 0.999f)
	{
		return;
	}
	bool forceSplit = false;
	if(dltTime < 0.0f)
	{
		forceSplit = true;
		dltTime = 0.0f;
	}	
	//Обновляем состояние
//	bool isReleaseTrail = true;
//	if(isStartTrace)
	{
		splitTime += dltTime;
		if(!lostPosition)
		{
//			isReleaseTrail = false;			
			if(trail >= 2)
			{
				if(isStartTrace)
				{
					UpdateTrail(dltTime);
				}
				

				last_elem = trail[1];

				trail[0].s = itemPosition * scene->GetNodeLocalTransform(lstart).pos;
				trail[0].e = itemPosition * scene->GetNodeLocalTransform(lend).pos;
				
				SplitTrail(forceSplit);
			}else{
				//Добавляем первые 2 элемента
				Element & el = trail[trail.Add()];
				el.s = itemPosition * scene->GetNodeLocalTransform(lstart).pos;
				el.e = itemPosition * scene->GetNodeLocalTransform(lend).pos;
				el.t = 1.0f;
				el.a = 255.0f*(1.0f - releaseTime);
				Element & ele = trail[trail.Add(el)];
				ele.t = 0.0f;
				ele.a = 0.0f;
			}
		}
	}
	
	if(!isStartTrace)
	{
		if(trail)
		{
			trail[0].a = 0.0f;
			for(long i = 0; i < trail; i++)
			{
				Element & el = trail[i];
//				el.t -= dltTime*(1.0f/STRAIL_FADETIME);
//				if(el.t < 0.0f) el.t = 0.0f;
				el.a = coremax(el.a - coremax(dltTime*50.0f*(10 - i)*(10 - i), 0.0f), 0.0f);
			}
//			if(trail[0].t <= 0.0f) trail.Empty();
			releaseTime += dltTime*(1.0f/STRAIL_FADETIME);
			if(releaseTime > 1.0f)
			{
				releaseTime = 1.0f;
				trail.Empty();
			}
		}
	}


	if (effectTable && trail.Size()>0 && chr->IsPlayer())
	{		
		IPhysicsScene::RaycastResult res;

		Matrix mtx;
		chr->GetMatrix(mtx);

		if (chr->Physics().Raycast( trail[0].s * mtx, trail[0].e * mtx, phys_mask(phys_character),&res) ||
			chr->Physics().Raycast( trail[0].e * mtx, trail[0].s * mtx, phys_mask(phys_character),&res))
		{			
			effectTable->MakeChachedEffect((PhysTriangleMaterialID)res.mtl,null,res.position,res.normal);
		}		
	}
}

//Обновить след
void SwordTrail::UpdateTrail(float dltTime)
{
	for(long i = 0; i < trail; i++)
	{
		trail[i].t -= dltTime*(1.0f/STRAIL_FADETIME);
		if(i > 0)
		{
			if(trail[i - 1].t <= 0.0f)
			{
				while(trail > i) trail.DelIndex(trail - 1);
				break;
			}
			if(trail[i].t <= 0.0f) trail[i].t = 0.0f;
		}else{
			if(isStartTrace) trail[0].t = 1.0f;
		}
	}
}

//Разделить первый элемент, если требуется
void SwordTrail::SplitTrail(bool isForceSplit)
{
	//Сначала пробуем разбить по дистанциям
	//float dist = coremax(~(trail[1].e - trail[0].e), ~(trail[1].s - trail[0].s));
	float dist = ~(last_elem.e - trail[0].e);

	if(dist > STRAIL_SPLDISTMAX*STRAIL_SPLDISTMAX)
	{
		trail.Empty();
		return;
	}	

	isForceSplit = false;

	if(dist > STRAIL_SPLITDIST*STRAIL_SPLITDIST)
	{		
		//Делим по растояниям
		dist = sqrtf(dist);

		float wholedist = dist;

		dist -= STRAIL_SPLITSEGMENTS;

		while(dist >= STRAIL_SPLITDIST)
		{
			isForceSplit = false;
			InsertElement(dist/wholedist);
			dist -= STRAIL_SPLITSEGMENTS;
		}
		splitTime = 0.0f;
	}/*else{
		//Делим по времени
		while(splitTime >= STRAIL_SPLITTIME)
		{
			isForceSplit = false;
			InsertElement(STRAIL_SPLITTIME/splitTime);
			splitTime -= STRAIL_SPLITTIME;
		}
	}*/
	//
	if(isForceSplit)
	{
		while(trail >= STRAIL_MAXELEMENTS) trail.DelIndex(trail - 1);
		Element el = trail[0];
		trail.Insert(el, 1);
	}
}

//Вставить элемент между 0 и 1 в указанную относительную позицию
void SwordTrail::InsertElement(float pos)
{
	Clampfr(pos);
	Element el;
	//Получаем вектор в точке интерполяции
	el.s.Lerp(last_elem.s, trail[0].s, 1-pos);
	el.e.Lerp(last_elem.e, trail[0].e, 1-pos);
	Vector n = !(el.e - el.s);
	//Коэфициент искривления сегмента
	float k = (!(trail[0].e - trail[0].s) | !(last_elem.e - last_elem.s));
	//Коэфициент воздействия искревления в зависимости от позиции
	float kPos = 1.0f - 4.0f*(pos - 0.5f)*(pos - 0.5f);
	//Полный коэфициент для данного элемента
	k = kPos*(1.0f - k*k)*0.1f;
	//Длины путей точек
	float paths = (trail[0].s - last_elem.s).GetLength();
	float pathe = (trail[0].e - last_elem.e).GetLength();
	//Учёт искривления
	el.s += n*paths*k;
	el.e += n*pathe*k;
	//Время сегмента
	el.t = (1.0f - last_elem.t)*pos + last_elem.t;
	el.a = 255.0f*(1.0f - releaseTime);
	//Освобождаем место
	while(trail >= STRAIL_MAXELEMENTS) trail.DelIndex(trail - 1);
	//Вставляем новый элемент
	trail.Insert(el, 1);	
}

//Установить модельку
bool SwordTrail::SetModel(IGMXScene * model)
{
	if (!chr) return false;
	
	//Удалим используемые локаторы
	if (scene)
	{
		if (lstart.isValid()) scene->Release();
		if (lend.isValid()) scene->Release();
		
		scene = null;
	}

	lstart.reset();
	lend.reset();

	if (!model) return false;
	//Перебираем все локаторы сцены

	scene = model;

	lstart = scene->FindEntity(GMXET_LOCATOR, "trails", GMXFINDMETHOD_POSTFIX);
	if (lstart.isValid()) scene->AddRef();

	lend = scene->FindEntity(GMXET_LOCATOR, "traile", GMXFINDMETHOD_POSTFIX);
	if (lend.isValid()) scene->AddRef();

	return true;
}
