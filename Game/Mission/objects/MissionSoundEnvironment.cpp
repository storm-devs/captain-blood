

#include "MissionSoundEnvironment.h"

#define MG_SOUNDENVIRONMENT		GroupId('S','E','g','O')
#define MG_SOUNDENVIRONMENT_MNG	GroupId('S','E','M','g')


MissionSoundEnvironment::MissionSoundEnvironment()
{
	params = null;
	finder = null;
}

MissionSoundEnvironment::~MissionSoundEnvironment()
{
	RELEASE(finder);
}

//Инициализировать объект
bool MissionSoundEnvironment::Create(MOPReader & reader)
{
	envName = reader.String();
	if(!finder)
	{
		finder = QTCreateObject(MG_SOUNDENVIRONMENT, _FL_);
		Assert(finder);
	}
	Registry(MG_SOUNDENVIRONMENT);
	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	Vector size;
	size.x = reader.Float()*0.5f;
	size.y = reader.Float()*0.5f;
	size.z = reader.Float()*0.5f;
	finder->SetBox(-size, size);
	finder->SetMatrix(Matrix(ang, pos));	
	UpdateStates(GetManager());
	return true;
}

//Обновить состояние
void MissionSoundEnvironment::UpdateStates(MissionSoundEnvironmentManager * mng)
{	
	bool isShow = false;
	bool isActive = true;
	if(mng)
	{
		isShow = mng->IsShow();
		if(envName.NotEmpty())
		{
			params = mng->FindPreset(envName);
			if(!params)
			{
				if(!EditMode_IsOn())
				{
					isActive = false;
					LogicDebugError("Object don't work! environment preset \"%s\" not found.", envName.c_str());
				}
			}
		}
	}else{		
		Show(false);
		if(!EditMode_IsOn())
		{
			isActive = false;
			LogicDebugError("Object don't work! Environment manager not found.");
		}
	}
	finder->Activate(isActive);
#ifndef STOP_DEBUG
	if(isShow)
	{
		SetUpdate(&MissionSoundEnvironment::EditModeDraw, ML_ALPHA5);
	}else{
		DelUpdate(&MissionSoundEnvironment::EditModeDraw);
	}
#endif
}

//Применить параметры среды
bool MissionSoundEnvironment::ApplyParams(ISoundScene::Enveronment & env)
{
	if(!params)
	{
		return false;
	}
	env = *params;
	return true;
}

//Инициализировать объект
bool MissionSoundEnvironment::EditMode_Create(MOPReader & reader)
{	
	Create(reader);
	return true;
}

//Обновить параметры
bool MissionSoundEnvironment::EditMode_Update(MOPReader & reader)
{
	Create(reader);
	return true;
}


//Нарисовать детектор
void _cdecl MissionSoundEnvironment::EditModeDraw(float dltTime, long level)
{
	//Получить состояния мэнеджера
	dword color = 0x00ffff00;
	MissionSoundEnvironmentManager * mng = GetManager();
	bool isParams = true;
	if(!params)
	{
		if(envName.NotEmpty())
		{
			isParams = false;
		}
	}
	bool isActive = false;
	if(mng && isParams)
	{
		if(mng->IsActive())
		{
			if(mng->IsActiveBox(this))
			{
				isActive = true;
				color = 0x00ff0fff;
			}
		}else{
			color = 0x00909090;
		}
	}else{
		color = 0x00ff0000;
	}
	Vector size = finder->GetBoxSize();
	size *= 0.5f;
	Render().DrawBox(-size, size, finder->GetMatrix(), color | 0xff000000);
	if(isActive)
	{
		Render().Print(50.0f, 130.0f, color | 0xff000000, "Environment box: %s, preset: %s", GetObjectID().c_str(), envName.c_str());
	}
	Render().Print(finder->GetMatrix().pos, 100.0f, 0.0f, color | 0xff000000, "%s", envName.c_str());
}

//Получить мэнеджер
MissionSoundEnvironmentManager * MissionSoundEnvironment::GetManager()
{
	MissionSoundEnvironmentManager * mng = null;
	MGIterator & it = GroupIterator(MG_SOUNDENVIRONMENT_MNG, _FL_);
	if(!it.IsDone())
	{
		mng = (MissionSoundEnvironmentManager *)it.Get();
		Assert(mng);
	}
	it.Release();
	return mng;
}


MOP_BEGINLISTCG(MissionSoundEnvironment, "Environment box", '1.00', 100, "Box volue trigger for sound environment preset", "Effects")
	MOP_STRINGC("Preset", "", "Environment preset defined in \"Environment manager\"")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLES("Angles", Vector(0.0f))
	MOP_FLOATEX("Width", 10.0f, 1.0f, 1000000.0f)
	MOP_FLOATEX("Height", 10.0f, 1.0f, 1000000.0f)
	MOP_FLOATEX("Length", 10.0f, 1.0f, 1000000.0f)
MOP_ENDLIST(MissionSoundEnvironment)

//-------------------------------------------------------------------------------------------------------------
//MissionSoundEnvironmentManager
//-------------------------------------------------------------------------------------------------------------


MissionSoundEnvironmentManager::MissionSoundEnvironmentManager() : boxes(_FL_)
{
	Assert(ARRSIZE(evtNames) == ARRSIZE(envSetups));
	boxes.Reserve(64);
	Show(false);
	blendTime = 2.0f;
	memset(envSetups, 0, sizeof(envSetups));
}

//Текущий активный
bool MissionSoundEnvironmentManager::IsActiveBox(MissionSoundEnvironment * se)
{
	if(boxes.Size() > 0)
	{
		if(boxes[boxes.Size() - 1].env == se)
		{
			return true;
		}
	}
	return false;
}

//Найти пресет по имени
ISoundScene::Enveronment * MissionSoundEnvironmentManager::FindPreset(const ConstString & name)
{
	for(dword i = 0; i < ARRSIZE(evtNames); i++)
	{
		if(evtNames[i] == name)
		{
			return &envSetups[i];
		}
	}
	return null;
}

//Инициализировать объект
bool MissionSoundEnvironmentManager::Create(MOPReader & reader)
{
	if(!UniqueObjectCheck()) return false;
	SetUpdate(&MissionSoundEnvironmentManager::UpdateScene, ML_SCENE_END - 0xff);
	Init(reader);
	return true;
}

//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
void MissionSoundEnvironmentManager::PostCreate()
{
	UpdateScene(0.0f, ML_FIRST);
}

//Активировать/деактивировать объект
void MissionSoundEnvironmentManager::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
}

//Инициализировать объект
bool MissionSoundEnvironmentManager::EditMode_Create(MOPReader & reader)
{
	if(!UniqueObjectCheck()) return false;
	return EditMode_Update(reader);
}

//Обновить параметры
bool MissionSoundEnvironmentManager::EditMode_Update(MOPReader & reader)
{
	Init(reader);	
	MGIterator & it = GroupIterator(MG_SOUNDENVIRONMENT, _FL_);
	for(; !it.IsDone(); it.Next())
	{
		MissionSoundEnvironment * menv = (MissionSoundEnvironment *)it.Get();
		menv->UpdateStates(this);
	}
	it.Release();	
	return true;
}

//Проверить на единственную копию объекта
bool MissionSoundEnvironmentManager::UniqueObjectCheck()
{
#ifndef STOP_DEBUG
	bool isAlone = true;
	MGIterator & it = GroupIterator(MG_SOUNDENVIRONMENT_MNG, _FL_);
	if(!it.IsDone())
	{
		if(it.Get() != this)
		{
			isAlone = false;
		}
		it.Next();
		if(!it.IsDone())
		{
			isAlone = false;
		}
	}
	it.Release();
	if(!isAlone)
	{
		LogicDebugError("This object can be only one in mission");
		return false;
	}	
	return true;
#else
	return true;
#endif
}

//Общая инициализация
void MissionSoundEnvironmentManager::Init(MOPReader & reader)
{
	Registry(MG_SOUNDENVIRONMENT_MNG);
	blendTime = reader.Float();
	memset(envSetups, 0, sizeof(envSetups));
	long count = reader.Array();
	Assert(count <= ARRSIZE(envSetups));
	for(long i = 0; i < count; i++)
	{	
		evtNames[i] = reader.String();
		envSetups[i].predelayTime = reader.Float();
		envSetups[i].earlyTime = reader.Float();
		envSetups[i].earlyAttenuation = reader.Float();
		envSetups[i].damping = reader.Float();
		envSetups[i].dispersion = reader.Float();
		envSetups[i].wet = reader.Float();
		envSetups[i].dry = reader.Float();
		envSetups[i].unuse = 0.0f;
	}
	Activate(reader.Bool());
#ifndef STOP_DEBUG
	Show(reader.Bool());
#else
	Show(false);
#endif
}

//Посчитать текущий активный детектор
void _cdecl MissionSoundEnvironmentManager::UpdateScene(float dltTime, long level)
{
	//Получаем слушателя
	Sound().GetListenerMatrix(listener);
	//Обновляем список ящиков, в которые попал слушатель
	bool isChange = false;
	BoxesUnuse();
	dword count = QTFindObjects(MG_SOUNDENVIRONMENT, listener.pos, listener.pos);
	for(dword i = 0; i < count; i++)
	{
		IMissionQTObject * qtObj = QTGetObject(i);
		if(!qtObj) continue;
		Vector localPos = qtObj->GetMatrix().MulVertexByInverse(listener.pos);
		localPos *= 2.0f;
		const Vector & size = qtObj->GetBoxSize();
		if(localPos.x > size.x || localPos.x < -size.x) continue;
		if(localPos.z > size.z || localPos.z < -size.z) continue;
		if(localPos.y > size.y || localPos.y < -size.y) continue;
		//Добавляем-обновляем ящик в списке
		MissionSoundEnvironment & mse = (MissionSoundEnvironment &)qtObj->GetMissionObject();
		isChange |= BoxesUse(&mse);
	}
	isChange |= BoxesRemoveUnuse();
	float bTime = blendTime;
#ifndef STOP_DEBUG
	if(IsShow())
	{
		if(api->DebugKeyState(VK_TAB))
		{
			boxes.Empty();
			bTime = 0.0f;
			Render().Print(50.0f, 130.0f, 0xffff00ff, "<TAB> disable environment effects");
		}
	}
#endif
	//Однобоксовая версия
	if(isChange)
	{
		if(boxes.Size() > 0)
		{		
			MissionSoundEnvironment * env = boxes[boxes.Size() - 1].env;
			ISoundScene::Enveronment result;
			if(env->ApplyParams(result))
			{
				Sound().SetSoundEnvironmentScene(&result, bTime);
			}		
		}else{
			Sound().SetSoundEnvironmentScene(null, bTime);		
		}
	}
}

//Сбросить состояние ящиков
void MissionSoundEnvironmentManager::BoxesUnuse()
{
	//Маркируем все ящики как необходимые удалить
	ActiveBox * bs = boxes.GetBuffer();
	dword count = boxes.Size();
	for(dword i = 0; i < count; i++)
	{
		bs[i].isUse = 0;
	}
}

//Устонавить состояние ящика в используемое
bool MissionSoundEnvironmentManager::BoxesUse(MissionSoundEnvironment * env)
{
	//Маркируем все ящики как необходимые удалить
	ActiveBox * bs = boxes.GetBuffer();
	dword count = boxes.Size();
	for(dword i = 0; i < count; i++)
	{
		if(bs[i].env == env)
		{
			bs[i].isUse = 1;
			return false;
		}
	}
	ActiveBox & newBox = boxes[boxes.Add()];
	newBox.env = env;
	newBox.isUse = 1;
	return true;
}

//Удалить неиспользуемые ящики
bool MissionSoundEnvironmentManager::BoxesRemoveUnuse()
{
	bool isChange = false;
	for(dword i = 0; i < boxes.Size(); )
	{
		if(boxes[i].isUse)
		{
			i++;
		}else{
			boxes.DelIndex(i);
			isChange = true;
		}
	}
	return isChange;
}


MOP_BEGINLISTCG(MissionSoundEnvironmentManager, "Environment manager", '1.00', 10, "Environment scene manager", "Effects")	
	MOP_FLOATEXC("Blend time (s)", 0.2f, 0.0f, 10.0f, "Время изменения с текущих на новые параметры")
	MOP_ARRAYBEG("Setups", 0, 20)
		MOP_STRING("Name", "")
		MOP_FLOATEXC("Predelay (ms)", 20.0f, 0.0f, 500.0f, "Задержка до первого отражённого звука 0..500мс")
		MOP_FLOATEXC("Early time (ms)", 200.0f, 0.0f, 1000.0f, "Время всех вторичных отражений 0..1000мс")
		MOP_FLOATEXC("Early attenuation", 0.3f, 0.0f, 1.0f, "Коэфициент возвращения первичных отражений")
		MOP_FLOATEXC("Damping", 0.2f, 0.0f, 1.0f, "Поглощение средой высокочастотной составляющей (захламлёность)")
		MOP_FLOATEXC("Dispersion", 0.5f, 0.0f, 1.0f, "Степень рассеивания (мелкие предметы)")
		MOP_FLOATEXC("Wet", 0.5f, 0.0f, 1.0f, "Какая часть рассеяного звука попадает в выходной сигнал (0..1)")
		MOP_FLOATEXC("Dry", 1.0f, 0.0f, 1.0f, "Какая часть исходного звука попадает в выходной сигнал (0..1)")
	MOP_ARRAYEND
	MOP_BOOL("Active", true)
	MOP_BOOL("Draw boxes", false)
MOP_ENDLIST(MissionSoundEnvironmentManager)
