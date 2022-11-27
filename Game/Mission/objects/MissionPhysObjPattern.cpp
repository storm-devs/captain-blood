

#include "MissionPhysObjPattern.h"
#include "MissionPhysObject.h"

#ifdef _DEBUG
#ifndef STOP_DEBUG

//#define ENABLE_FX_DEBUG

#endif
#endif

#ifdef ENABLE_FX_DEBUG
//После отладки переменные надо будет перенести в PlayCollisionEffect и тут всё вычистить
float minImpulseThreshold = 1.0f;
float minImpulseParticlesThreshold = 10.0f;
float maxImpulseValue = 1000.0f;
float impulseParticlesPow = 0.2f;
float impulseSoundsPow = 0.7f;
float minParticlesScale = 0.2f;
float minSoundVolume = 0.1f;
#endif

void SaveFX()
{
#ifdef ENABLE_FX_DEBUG
#ifndef STOP_DEBUG
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	IEditableIniFile * ini = fs->OpenEditableIniFile("debugFxParams.delme", file_create_always, _FL_);
	if(ini)
	{
		ini->SetFloat(null, "minImpulseThreshold", minImpulseThreshold);
		ini->SetFloat(null, "minImpulseParticlesThreshold", minImpulseParticlesThreshold);
		ini->SetFloat(null, "maxImpulseValue", maxImpulseValue);
		ini->SetFloat(null, "impulseParticlesPow", impulseParticlesPow);
		ini->SetFloat(null, "impulseSoundsPow", impulseSoundsPow);
		ini->SetFloat(null, "minParticlesScale", minParticlesScale);
		ini->SetFloat(null, "minSoundVolume", minSoundVolume);
		ini->Release();
	}
#endif
#endif
}

void _cdecl ReadFX()
{
#ifdef ENABLE_FX_DEBUG
#ifndef STOP_DEBUG
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	IIniFile * ini = fs->OpenIniFile("debugFxParams.delme", _FL_);
	if(ini)
	{
		minImpulseThreshold = ini->GetFloat(null, "minImpulseThreshold", minImpulseThreshold);
		minImpulseParticlesThreshold = ini->GetFloat(null, "minImpulseParticlesThreshold", minImpulseParticlesThreshold);
		maxImpulseValue = ini->GetFloat(null, "maxImpulseValue", maxImpulseValue);
		impulseParticlesPow = ini->GetFloat(null, "impulseParticlesPow", impulseParticlesPow);
		impulseSoundsPow = ini->GetFloat(null, "impulseSoundsPow", impulseSoundsPow);
		minParticlesScale = ini->GetFloat(null, "minParticlesScale", minParticlesScale);
		minSoundVolume = ini->GetFloat(null, "minSoundVolume", minSoundVolume);
		ini->Release();
	}
#endif
#endif
}

void _cdecl Console_UpdateFXParams(const ConsoleStack & params)
{
#ifdef ENABLE_FX_DEBUG
#pragma message("MissionPhysObjPattern::PlayCollisionEffect not complette!!!")
#ifndef STOP_DEBUG
	ReadFX();
	bool isSave = false;
	for(dword i = 0; i < params.GetSize()/2; i++)
	{
		const char * name = params.GetParam(i*2 + 0);
		float param = (float)atof(params.GetParam(i*2 + 1));
		switch(name[0])
		{
		case '1':
			isSave = true;
			minImpulseThreshold = Clampf(param, 0.001f, 100000.0f);
			break;
		case '2':
			isSave = true;
			minImpulseParticlesThreshold = Clampf(param, 0.001f, 100000.0f);
			break;
		case '3':
			isSave = true;
			maxImpulseValue = Clampf(param, 0.001f, 100000.0f);
			break;
		case '4':
			isSave = true;
			impulseParticlesPow = Clampf(param, 0.001f, 1000.0f);
			break;
		case '5':
			isSave = true;
			impulseSoundsPow = Clampf(param, 0.001f, 1000.0f);
			break;
		case '6':
			isSave = true;
			minParticlesScale = Clampf(param);
			break;
		case '7':
			isSave = true;
			minSoundVolume = Clampf(param);
			break;
		}		
	}
	if(isSave)
	{
		SaveFX();
	}
#else
#pragma message("Fix MissionPhysObjPattern::PlayCollisionEffect before release!!!")
#endif
#endif
}



MissionPhysObjPattern::FadeoutElement MissionPhysObjPattern::fadeoutElements[] =
{
	{ConstString("Hide"), fe_hide},
	{ConstString("Set static"), fe_static},
	{ConstString("No change"), fe_none}
};

MissionPhysObjPattern::MaterialDescription MissionPhysObjPattern::materialDescs[] = 
{
	{ConstString("Wood"), pmtlid_wood},
	{ConstString("Stone"), pmtlid_stone},
	{ConstString("Iron"), pmtlid_iron},
	{ConstString("Fabrics"), pmtlid_fabrics}
};


MissionPhysObjPattern::PatternObject::PatternObject() : boxes(_FL_, 1),
														capsules(_FL_, 1),
														sounds(_FL_, 1),
														particles(_FL_, 1)
{
	Assert(pmtlid_ground == 1);
	Assert(pmtlid_stone == 2);
	Assert(pmtlid_sand == 3);
	Assert(pmtlid_wood == 4);
	Assert(pmtlid_grass == 5);
	Assert(pmtlid_water == 6);
	Assert(pmtlid_iron == 7);
	Assert(pmtlid_fabrics == 8);

	dencity = 5.0f;
	centerOfMass = 0.0f;
	isStatic = true;
	isShow = true;
	isSoloDraw = false;
	fe = fe_none;
	impulsePosition = 0.0f;
	impulse = 0.0f;
	isAddImpulse = false;	
	min = 0.0f;
	max = 0.0f;
	tipPosition = 0.0f;
	model = null;
}

MissionPhysObjPattern::Effect::Effect()
{
#ifdef ENABLE_FX_DEBUG
	SaveFX();
#endif
}



MissionPhysObjPattern::MissionPhysObjPattern() : objects(_FL_),
												 locators(_FL_),
												 regObjects(_FL_),
												 scenes(_FL_)
{
	dynamicLighting = true;
	shadowCast = true;
	shadowReceive = true;
	seaReflection = false;
	level = 0;
	hideDistanceMin2 = hideDistanceK2 = 0.0f;
}

MissionPhysObjPattern::~MissionPhysObjPattern()
{
	for(long i = 0; i < regObjects; i++)
	{
		regObjects[i]->DeletePattern();
	}
	objects.DelAll();
	for(long i = 0; i < scenes; i++)
	{
		if(scenes[i].model)
		{
			scenes[i].model->Release();
		}		
	}
	scenes.DelAll();
}

//Инициализировать объект
bool MissionPhysObjPattern::Create(MOPReader & reader)
{
#ifdef ENABLE_FX_DEBUG
#ifndef STOP_DEBUG
	Console().Register_PureC_Command("ufx", "Скажи Максу удалить это!!!", Console_UpdateFXParams);
	ReadFX();
#endif
#endif
	haveSoloDraw = false;
	objects.DelAll();
	locators.DelAll();
	buildPosition = reader.Position();
	Show(reader.Bool());
	showOnlySelected = reader.Bool();
	showSolid = reader.Bool();
	showBroken = reader.Bool();
	showObjectCenter = reader.Bool();
	showSoundsPoints = reader.Bool();
	showParticlesPoints = reader.Bool();
	showBrokeImpulses = reader.Bool();
	showCenterOfMass = reader.Bool();
	showTipsPosition = reader.Bool();
	showLocators = reader.Bool();
	isPickup = reader.Bool();
	hp = reader.Float();
	activeTime = reader.Float();
	ConstString matDesc = reader.Enum();
	materialId = pmtlid_wood;
	for(dword i = 0; i < ARRSIZE(materialDescs); i++)
	{
		if(materialDescs[i].name == matDesc)
		{
			materialId = materialDescs[i].id;
			break;
		}
	}
	hitSword.particles = reader.String();
	hitSword.sound = reader.String();
	hitBullet.particles = reader.String();
	hitBullet.sound = reader.String();	
	hitBomb.particles = reader.String();
	hitBomb.sound = reader.String();
	hitShooter.particles = reader.String();
	hitShooter.sound = reader.String();
	hitCannon.particles = reader.String();
	hitCannon.sound = reader.String();
	hitFlame.particles = reader.String();	
	hitFlame.sound = reader.String();
	for(dword i = 0; i < ARRSIZE(hitToMaterial); i++)
	{
		hitToMaterial[i].particles = reader.String();
		hitToMaterial[i].sound = reader.String();
	}
	soldObjectsCount = reader.Array();
	objects.AddElements(soldObjectsCount);
	for(long i = 0; i < objects; i++)
	{
		PatternObject & obj = objects[i];
		ReadModelParams(i, reader);
		obj.particles.AddElements(reader.Array());
		for(long j = 0; j < obj.particles; j++)
		{
			obj.particles[j].name = reader.String();
			Vector pos = reader.Position();
			Vector ang = reader.Angles();
			obj.particles[j].mtx = Matrix(ang, pos);
		}
		obj.sounds.AddElements(reader.Array());
		for(long j = 0; j < obj.sounds; j++)
		{
			obj.sounds[j].name = reader.String();
			obj.sounds[j].mtx.pos = reader.Position();
		}
		obj.receiveBoom = reader.String();
		obj.waterParticle = reader.String();
		obj.waterSound = reader.String();
		obj.dencity = reader.Float();
		obj.centerOfMass = reader.Position();
		obj.isStatic = reader.Bool();
		obj.isShow = reader.Bool();
		obj.isSoloDraw = reader.Bool();
		obj.tipId = reader.String();
		obj.tipPosition = reader.Position();
		obj.bonusesTable = reader.String();
		haveSoloDraw |= obj.isSoloDraw;
	}
	objects.AddElements(reader.Array());
	for(; i < objects; i++)
	{
		PatternObject & obj = objects[i];
		ReadModelParams(i, reader);
		obj.waterParticle = reader.String();
		obj.waterSound = reader.String();
		ConstString fe = reader.Enum();
		for(dword i = 0; i < ARRSIZE(fadeoutElements); i++)
		{
			if(fe == fadeoutElements[i].name)
			{
				obj.fe = fadeoutElements[i].id;
				break;
			}
		}
		Assert(i < ARRSIZE(fadeoutElements));
		Vector pos = reader.Position();
		Vector ang = reader.Angles();
		Matrix mtx(ang, pos);
		obj.impulsePosition = mtx.pos;
		obj.impulse = mtx.vz;
		obj.impulse *= reader.Float();
		obj.isAddImpulse = reader.Bool();
		obj.dencity = reader.Float();
		obj.centerOfMass = reader.Position();
		obj.isStatic = reader.Bool();
		obj.isShow = reader.Bool();
		obj.isSoloDraw = reader.Bool();
		haveSoloDraw |= obj.isSoloDraw;
	}
	damageModifier.Init(reader);
	if(EditMode_IsOn())
	{
		Matrix toWorld(Vector(0.0f), buildPosition);
		for(long j = 0; j < scenes; j++)
		{
			scenes[j].use = false;
		}
		for(long i = 0; i < objects; i++)
		{
			PatternObject & obj = objects[i];
			obj.model = null;
			for(long j = 0; j < scenes; j++)
			{
				if(!scenes[j].model) continue;
				if(scenes[j].use == false && obj.modelName.c_str() == scenes[j].model->GetFileName())
				{
					obj.model = scenes[j].model;
					obj.model->SetTransform(obj.mtx*toWorld);
					scenes[j].use = true;
					break;
				}
			}
			if(!obj.model)
			{
				CacheScene cs;
				cs.model = obj.model = Geometry().CreateScene(obj.modelName.c_str(), &Animation(), &Particles(), &Sound(), _FL_);
				if(obj.model)
				{
					obj.model->SetTransform(obj.mtx*toWorld);
				}
				cs.use = true;
				if(cs.model)
				{
					scenes.Add(cs);
				}				
			}			
		}
		for(long j = 0; j < scenes; )
		{
			if(scenes[j].use == false)
			{
				scenes[j].model->Release();
				scenes.DelIndex(j);
			}else{
				j++;
			}
		}
		bool isAddBound = false;
		viewMinBox = viewMaxBox = 0.0f;
		for(long i = 0; i < objects; i++)
		{
			PatternObject & obj = objects[i];
			if(!obj.model) continue;
			if(isAddBound)
			{
				viewMinBox.Min(obj.model->GetBound().vMin);
				viewMinBox.Min(obj.model->GetBound().vMax);
				viewMaxBox.Max(obj.model->GetBound().vMin);
				viewMaxBox.Max(obj.model->GetBound().vMax);
			}else{
				viewMinBox = obj.model->GetBound().vMin;
				viewMaxBox = obj.model->GetBound().vMax;
				isAddBound = true;
			}
		}
	}
	dynamicLighting = reader.Bool();
	shadowCast = reader.Bool();
	shadowReceive = reader.Bool();
	seaReflection = reader.Bool();
	if(reader.Bool())
	{
		level = reader.Long() + ML_ALPHA1;
	}else{
		level = reader.Long() + ML_GEOMETRY1;
	}
	float hideDistanceMin = reader.Float();
	if(hideDistanceMin > 1e-5f)
	{
		float hideDistanceMax = hideDistanceMin + 5.0f;
		hideDistanceMin2 = hideDistanceMin*hideDistanceMin;
		hideDistanceK2 = 1.0f/(hideDistanceMax*hideDistanceMax - hideDistanceMin*hideDistanceMin);
	}else{
		hideDistanceMin2 = 0.0f;
		hideDistanceK2 = 0.0f;
	}
	for(long i = 0; i < regObjects; i++)
	{
		regObjects[i]->UpdatePatternData();
	}
	return true;
}

void MissionPhysObjPattern::ReadModelParams(long index, MOPReader & reader)
{
	PatternObject & obj = objects[index];
	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	obj.mtx.Build(ang, pos);
	obj.modelName = reader.String();
	bool isAdd = false;	
	obj.boxes.AddElements(reader.Array());
	for(long i = 0; i < obj.boxes; i++)
	{
		CollisionBox & box = obj.boxes[i];
		pos = reader.Position();
		ang = reader.Angles();
		box.mtx.Build(ang, pos);
		box.size05.x = reader.Float()*0.5f;
		box.size05.y = reader.Float()*0.5f;
		box.size05.z = reader.Float()*0.5f;
		Box::FindABBforOBB(box.mtx, -box.size05, box.size05, obj.min, obj.max, isAdd);
		isAdd = true;
	}
	obj.capsules.AddElements(reader.Array());
	for(long i = 0; i < obj.capsules; i++)
	{
		CollisionCapsule & capsule = obj.capsules[i];
		pos = reader.Position();
		ang = reader.Angles();
		capsule.mtx.Build(ang, pos);
		capsule.height = reader.Float();
		capsule.radius = reader.Float();
		if(capsule.height < 0.001f) capsule.height = 0.001f;
		capsule.height += capsule.radius*2.0f;
		capsule.size05.x = capsule.radius;
		capsule.size05.y = capsule.height*0.5f;
		capsule.size05.z = capsule.radius;
		Box::FindABBforOBB(capsule.mtx, -capsule.size05, capsule.size05, obj.min, obj.max, isAdd);
		isAdd = true;
	}

	long locatorsCount = reader.Array();
	for(long i = 0; i < locatorsCount; i++)
	{
		Locator & loc = locators[locators.Add()];
		loc.name = reader.String();
		pos = reader.Position();
		ang = reader.Angles();
		loc.mtx.Build(ang, pos);
		//loc.mtx.EqMultiplyFast(Matrix(ang, pos), obj.mtx);
		loc.objectIndex = index;
	}
}

//Инициализировать объект
bool MissionPhysObjPattern::EditMode_Create(MOPReader & reader)
{
	EditMode_Update(reader);
	return true;
}

//Обновить параметры
bool MissionPhysObjPattern::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	if(IsShow())
	{
		SetUpdate(&MissionPhysObjPattern::Draw, ML_GEOMETRY1);
	}	
	return true;
}

//Получить размеры описывающего ящика
void MissionPhysObjPattern::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = viewMinBox;
	max = viewMaxBox;
}

//Получить матрицу объекта
Matrix & MissionPhysObjPattern::GetMatrix(Matrix & mtx)
{	
	mtx.SetIdentity();
	return mtx;
}

//Нарисовать модельку
void _cdecl MissionPhysObjPattern::Draw(float dltTime, long level)
{	
	if(!IsShow())
	{
		DelUpdate(&MissionPhysObjPattern::Draw);
		return;
	}
	if(showOnlySelected && !EditMode_IsSelect()) return;
	if(!EditMode_IsVisible()) return;
	Matrix toWorld(Vector(0.0f), buildPosition);
	if(showObjectCenter)
	{
		Render().DrawMatrix(toWorld, 1.0f);
	}
	for(long i = 0; i < objects; i++)
	{
		PatternObject & obj = objects[i];
		//Пропускаем невидимые объекты
		if(!haveSoloDraw)
		{
			if(!obj.isShow) continue;
			if(i < soldObjectsCount)
			{
				if(!showSolid) continue;
			}else{
				if(!showBroken) continue;
			}			
		}else{
			if(!obj.isSoloDraw) continue;
		}
		//Позиция
		Matrix toWorld(obj.mtx);
		toWorld.pos += buildPosition;
		//Звуки-партиклы отдельно
		if(showSoundsPoints)
		{
			for(long j = 0; j < obj.sounds; j++)
			{
				Render().DrawSphere(obj.sounds[j].mtx.pos*toWorld, 0.05f, 0xff0000ff);
				Render().Print(obj.sounds[j].mtx.pos*toWorld, 20.0f, -1.0f, 0xffc0c0ff, "Sound");
				Render().Print(obj.sounds[j].mtx.pos*toWorld, 20.0f, 0.0f, 0xffc0c0ff, obj.sounds[j].name.c_str());
			}
		}
		if(showParticlesPoints)
		{
			for(long j = 0; j < obj.particles; j++)
			{
				Render().DrawSphere(obj.particles[j].mtx.pos*toWorld, 0.05f, 0xffff0000);
				Render().DrawMatrix(obj.particles[j].mtx*toWorld, 0.4f);
				Render().Print(obj.particles[j].mtx.pos*toWorld, 20.0f, -1.0f, 0xffc0c0ff, "Particles");
				Render().Print(obj.particles[j].mtx.pos*toWorld, 20.0f, 0.0f, 0xffc0c0ff, obj.particles[j].name.c_str());
			}
		}
		if(showCenterOfMass)
		{
			Render().DrawSphere(obj.centerOfMass*toWorld, 0.08f, 0xffcfcf00);
		}
		if(showTipsPosition)
		{
			Render().DrawSphere(obj.tipPosition*toWorld, 0.08f, 0xffff00ff);
		}
		//Рисуем
		if(obj.model)
		{
			obj.model->Draw();
		}
		for(long j = 0; j < obj.boxes; j++)
		{
			CollisionBox & box = obj.boxes[j];
			Render().DrawBox(-box.size05, box.size05, box.mtx*toWorld, 0xffffff80);
		}
		for(long j = 0; j < obj.capsules; j++)
		{
			CollisionCapsule & cps = obj.capsules[j];
			Render().DrawCapsule(cps.radius, cps.height-(cps.radius*2.0f), 0xffffff00, cps.mtx*toWorld);
			Render().DrawBox(-cps.size05, cps.size05, cps.mtx*toWorld, 0x5fff0000);
		}
		if(showBrokeImpulses)
		{
			if(i >= soldObjectsCount)
			{
				Vector impulsePos = toWorld*obj.impulsePosition;
				Render().DrawVector(impulsePos, impulsePos + toWorld.MulNormal(obj.impulse), 0xffff0000);
			}
		}
	}
	for(long i = 0; i < locators; i++)
	{
		Locator & loc = locators[i];
		PatternObject & obj = objects[loc.objectIndex];
		Matrix mtx(loc.mtx, obj.mtx);
		mtx.pos += buildPosition;
		Render().DrawMatrix(mtx, 0.2f);
	}
}


//Прекешировать модельки заданные в патерне
void MissionPhysObjPattern::CacheModels()
{
	if(scenes > 0)
	{
		return;
	}
	scenes.AddElements(objects);
	for(long i = 0; i < objects; i++)
	{
		PatternObject & po = objects[i];
		if(po.modelName.NotEmpty())
		{
			scenes[i].model = Geometry().CreateScene(po.modelName.c_str(), &Animation(), null, null, _FL_);
		}else{
			scenes[i].model = null;
		}		
	}
}

//Проиграть звук столкновения с данным материалом
void MissionPhysObjPattern::PlayCollisionEffect(long mtlId, const Vector & pos, float impulse2)
{
#ifdef ENABLE_FX_DEBUG
	api->Trace(")))))))))))))    Material: %i, impulse: %f", mtlId, powf(impulse2, 0.5f));
#else
	static const float minImpulseThreshold = 1.0f;
	static const float minImpulseParticlesThreshold = 10.0f;
	static const float maxImpulseValue = 1000.0f;
	static const float impulseParticlesPow = 0.2f;
	static const float impulseSoundsPow = 0.7f;
	static const float minParticlesScale = 0.2f;
	static const float minSoundVolume = 0.1f;
#endif
	//Проверяем материалы на возможность реакции
	if(mtlId > 8 || mtlId < 1) return;
	//Проверить импульс
	float maxImpulseValue2 = maxImpulseValue*maxImpulseValue;
	if(impulse2 < minImpulseThreshold*minImpulseThreshold) return;	
	impulse2 = coremin(impulse2, maxImpulseValue2);
	//Имена
	mtlId--;
	Assert(mtlId < ARRSIZE(hitToMaterial));	
	Sfx & sfx = hitToMaterial[mtlId];
	//Если импульс выше порогового для партиклов, то появляем эффект в точке контакта
	if(sfx.particles.NotEmpty() && impulse2 > minImpulseParticlesThreshold*minImpulseParticlesThreshold)
	{
		Matrix mtx;
		mtx.pos = pos;
#ifdef ENABLE_FX_DEBUG
		//IParticleSystem * ps = Particles().CreateParticleSystemEx2("woodbreak", mtx, false, _FL_);
		IParticleSystem * ps = Particles().CreateParticleSystemEx2("shiphitlarge", mtx, false, _FL_);
#else
		IParticleSystem * ps = Particles().CreateParticleSystemEx2(sfx.particles.c_str(), mtx, true, _FL_);
#endif		
		if(ps)
		{
			float scale = powf(impulse2/maxImpulseValue2, impulseParticlesPow)*(1.0f - minParticlesScale) + minParticlesScale;
			ps->SetScale(scale);
			ps->AutoDelete(true);
#ifdef ENABLE_FX_DEBUG
			api->Trace(")))))))))))))>    Show particles with scale: %f", scale);
#endif
		}
	}
	if(sfx.sound.NotEmpty())
	{
		float volume = powf((impulse2/maxImpulseValue2), impulseSoundsPow)*(1.0f - minSoundVolume) + minSoundVolume;
#ifdef ENABLE_FX_DEBUG
		Sound().Create3D(ConstString("gun_hit_wood"), pos, _FL_, true, true, volume);
		api->Trace(")))))))))))))>    Play sound with volume: %f\n", volume);
#else
		Sound().Create3D(sfx.sound, pos, _FL_, true, true, volume);
#endif
	}
}



//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(MissionPhysObjPattern, "Physic object pattern", '1.00', 0, "Pattern for physic objects", "Physics")

	MOP_ENUMBEG("Fade effects")
		for(dword i = 0; i < ARRSIZE(MissionPhysObjPattern::fadeoutElements); i++)
		{
			MOP_ENUMELEMENT(MissionPhysObjPattern::fadeoutElements[i].name.c_str())
		}
	MOP_ENUMEND
	MOP_ENUMBEG("Object material")
		for(dword i = 0; i < ARRSIZE(MissionPhysObjPattern::materialDescs); i++)
		{
			MOP_ENUMELEMENT(MissionPhysObjPattern::materialDescs[i].name.c_str())
		}
	MOP_ENUMEND

	MOP_POSITION("Build position", Vector(0.0f))
	MOP_BOOL("Show", true)
	MOP_BOOL("Show only selected", false)
	MOP_BOOL("Show solid part", true)
	MOP_BOOL("Show broken part", true)
	MOP_BOOL("Show object center", true)
	MOP_BOOLC("Show sounds points", true, "Sounds draw as blue spheres")
	MOP_BOOLC("Show particles points", true, "Particles draw as red spheres")
	MOP_BOOL("Show broke impulses", true)
	MOP_BOOL("Show center of mass", true)
	MOP_BOOL("Show tips position", true)
	MOP_BOOL("Show locators", true)
	MOP_BOOL("Is pickup", false);
	MOP_FLOATEX("HP", 100.0f, 0.0f, 100000.0f)
	MOP_FLOATEXC("Active time", 3.0f, 0.0f, 100000.0f, "Time for change state of broken parts")
	MOP_ENUM("Object material", "Material");
	MOP_GROUPBEG("Hit effects")
		MOP_GROUPBEG("Hit sword")
			MOP_STRING("Particles | Hit sword |", "")
			MOP_STRING("Sound | Hit sword |", "")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit bullet")
			MOP_STRING("Particles | Hit bullet |", "")
			MOP_STRING("Sound | Hit bullet |", "")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit bomb")
			MOP_STRING("Particles | Hit bomb |", "")
			MOP_STRING("Sound | Hit bomb |", "")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit shooter")
			MOP_STRING("Particles | Hit shooter |", "")
			MOP_STRING("Sound | Hit shooter |", "")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit cannon")
			MOP_STRING("Particles | Hit cannon |", "")
			MOP_STRING("Sound | Hit cannon |", "")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit flame")
			MOP_STRING("Particles | Hit flame |", "")
			MOP_STRING("Sound | Hit flame |", "")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit to ground")
			MOP_STRING("Particles | Hit to ground |", "hit_wood_to_solid")
			MOP_STRING("Sound | Hit to ground |", "h_wood_ground")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit to stone")
			MOP_STRING("Particles | Hit to stone |", "hit_wood_to_solid")
			MOP_STRING("Sound | Hit to stone |", "h_wood_stone")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit to sand")
			MOP_STRING("Particles | Hit to sand |", "hit_wood_to_sand")
			MOP_STRING("Sound | Hit to sand |", "h_wood_sand")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit to wood")
			MOP_STRING("Particles | Hit to wood |", "hit_wood_to_solid")
			MOP_STRING("Sound | Hit to wood |", "h_wood_wood")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit to grass")
			MOP_STRING("Particles | Hit to grass |", "hit_wood_to_grass")
			MOP_STRING("Sound | Hit to grass |", "h_wood_grass")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit to water")
			MOP_STRING("Particles | Hit to water |", "hit_wood_cloud")
			MOP_STRING("Sound | Hit to water |", "h_wood_water")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit to iron")
			MOP_STRING("Particles | Hit to iron |", "hit_wood_to_solid")
			MOP_STRING("Sound | Hit to iron |", "h_wood_iron")
		MOP_GROUPEND()
		MOP_GROUPBEG("Hit to fabrics")
			MOP_STRING("Particles | Hit to fabrics |", "hit_wood_to_fabrics")
			MOP_STRING("Sound | Hit to fabrics |", "h_wood_fabrics")
		MOP_GROUPEND()
	MOP_GROUPEND()
	MOP_ARRAYBEG("Solid parts", 1, 1)
		MOP_POSITION("Position", Vector(0.0f))
		MOP_ANGLES("Orient", Vector(0.0f))
		MOP_STRING("Model", "")
		MOP_ARRAYBEG("Box", 0, 16)		
			MOP_POSITIONC("Position", Vector(0.0f), "Local position")
			MOP_ANGLESC("Orient", Vector(0.0f), "Local orientation")
			MOP_FLOATEX("Size x", 0.1f, 0.001f, 1000.0f)
			MOP_FLOATEX("Size y", 0.1f, 0.001f, 1000.0f)
			MOP_FLOATEX("Size z", 0.1f, 0.001f, 1000.0f)
		MOP_ARRAYEND
		MOP_ARRAYBEG("Capsule", 0, 16)
			MOP_POSITIONC("Position", Vector(0.0f), "Local position")
			MOP_ANGLESC("Orient", Vector(0.0f), "Local orientation")
			MOP_FLOATEX("Height", 0.3f, 0.001f, 1000.0f)
			MOP_FLOATEX("Radius", 0.1f, 0.001f, 1000.0f)
		MOP_ARRAYEND
		MOP_ARRAYBEG("Locators", 0, 16)
			MOP_STRINGC("Name", "", "Locator's name for do access it")
			MOP_POSITIONC("Position", Vector(0.0f), "Local position")
			MOP_ANGLESC("Orient", Vector(0.0f), "Local orientation")
		MOP_ARRAYEND
		MOP_ARRAYBEG("Broke particles", 0, 10)
			MOP_STRING("Particles", "")
			MOP_POSITIONC("Position", Vector(0.0f), "Local position")
			MOP_ANGLESC("Orient", Vector(0.0f), "Local orientation")
		MOP_ARRAYEND
		MOP_ARRAYBEG("Broke sounds", 0, 10)
			MOP_STRING("Sound", "")
			MOP_POSITIONC("Position", Vector(0.0f), "Local position")
		MOP_ARRAYEND
		MOP_STRINGC("Bomb explosion", "", "Object, what receive command |boom x y z|")
		MOP_STRING("Water drop particles", "")
		MOP_STRING("Water drop sound", "")
		MOP_FLOATEX("Density", 5.0f, 0.1f, 100.0f)
		MOP_POSITIONC("Center of mass", Vector(0.0f), "Local position center of mass")
		MOP_BOOL("Is static", false)
		MOP_BOOLC("Is show", true, "Show or hide this object")
		MOP_BOOLC("Solo draw", false, "Show only this object (ignore Is show flag)")
		MOP_STRINGC("TipID", "Weapon", "Identifier of tip")
		MOP_POSITIONC("Tip position", Vector(0.0f), "Local tip pivot, text draw up on this point")
		MOP_STRING("Bonus Table", "")
	MOP_ARRAYEND
	MOP_ARRAYBEG("Broken parts", 0, 100)
		MOP_POSITIONC("Position", Vector(0.0f), "Local position")
		MOP_ANGLESC("Orient", Vector(0.0f), "Local orientation")
		MOP_STRING("Model", "")
		MOP_ARRAYBEG("Box", 0, 16)
			MOP_POSITIONC("Position", Vector(0.0f), "Local position")
			MOP_ANGLESC("Orient", Vector(0.0f), "Local orientation")
			MOP_FLOATEX("Size x", 0.1f, 0.001f, 1000.0f)
			MOP_FLOATEX("Size y", 0.1f, 0.001f, 1000.0f)
			MOP_FLOATEX("Size z", 0.1f, 0.001f, 1000.0f)
		MOP_ARRAYEND
		MOP_ARRAYBEG("Capsule", 0, 16)
			MOP_POSITIONC("Position", Vector(0.0f), "Local position")
			MOP_ANGLESC("Orient", Vector(0.0f), "Local orientation")
			MOP_FLOATEX("Height", 0.3f, 0.001f, 1000.0f)
			MOP_FLOATEX("Radius", 0.1f, 0.001f, 1000.0f)
		MOP_ARRAYEND
		MOP_ARRAYBEG("Locators", 0, 16)
			MOP_STRINGC("Name", "", "Locator's name for do access it")
			MOP_POSITIONC("Position", Vector(0.0f), "Local position")
			MOP_ANGLESC("Orient", Vector(0.0f), "Local orientation")
		MOP_ARRAYEND
		MOP_STRING("Water drop particles", "")
		MOP_STRING("Water drop sound", "")			
		MOP_ENUM("Fade effects", "Fadeout effect")		
		MOP_POSITIONC("Impulse position", Vector(0.0f), "Local impulse position")
		MOP_ANGLESC("Impulse orient", Vector(0.0f), "Local orientation")
		MOP_FLOATEX("Impulse value", 0.0f, 0.0f, 1000.0f)
		MOP_BOOLC("Add impulse", false, "Add impulse to current, else replase current impulse")		
		MOP_FLOATEX("Dencity", 5.0f, 0.1f, 100.0f)
		MOP_POSITIONC("Center of mass", Vector(0.0f), "Local position center of mass")
		MOP_BOOL("Is static", false)
		MOP_BOOLC("Is show", true, "Show or hide this object")
		MOP_BOOLC("Solo draw", false, "Show only this object (ignore Is show flag)")
	MOP_ARRAYEND
	MOP_DR_MULTIPLIERSG
	MOP_GROUPBEG("Render params")
		MOP_BOOL("Dynamic lighting", false)
		MOP_BOOLC("Shadow cast", false, "Geometry can is shadow cast by some objects")
		MOP_BOOLC("Shadow receive", false, "Geometry can is shadow receive from casting objects")
		MOP_BOOLC("Sea reflection", false, "Geometry can reflect in sea")
		MOP_BOOLC("Transparency", false, "Geometry draw as transparency (i.e. light rays)")
		MOP_LONGEXC("Level", 0, 0, 100, "Order of geometry draw")
		MOP_FLOATC("Hide distance", -1.0f, "If distance less 0, then ignore this feature")
	MOP_GROUPEND()
MOP_ENDLIST(MissionPhysObjPattern)

