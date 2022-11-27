
#ifndef _MissionSoundEnvironment_h_
#define _MissionSoundEnvironment_h_

#include "..\..\..\Common_h\Mission\Mission.h"


class MissionSoundEnvironmentManager;

class MissionSoundEnvironment : public MissionObject
{
	//--------------------------------------------------------------------------------------------
public:
	MissionSoundEnvironment();
	virtual ~MissionSoundEnvironment();

	//--------------------------------------------------------------------------------------------
public:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Обновить состояние
	void UpdateStates(MissionSoundEnvironmentManager * mng);
	//Применить параметры среды
	bool ApplyParams(ISoundScene::Enveronment & env);


	//--------------------------------------------------------------------
private:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Нарисовать детектор
	void _cdecl EditModeDraw(float dltTime, long level);
	//Получить менеджер
	MissionSoundEnvironmentManager * GetManager();

	//--------------------------------------------------------------------------------------------
private:
	ISoundScene::Enveronment * params;
	IMissionQTObject * finder;
	ConstString envName;
};

class MissionSoundEnvironmentManager : public MissionObject
{
	friend class MissionSoundEnvironment;

	struct ActiveBox
	{
		MissionSoundEnvironment * env;
		dword isUse;
	};

public:
	MissionSoundEnvironmentManager();

public:
	//Текущий активный
	bool IsActiveBox(MissionSoundEnvironment * se);
	//Найти пресет по имени
	ISoundScene::Enveronment * FindPreset(const ConstString & name);
	
	//--------------------------------------------------------------------------------------------
private:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
	virtual void PostCreate();
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

	//--------------------------------------------------------------------
private:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Проверить на единственную копию объекта
	bool UniqueObjectCheck();
	//Общая инициализация
	void Init(MOPReader & reader);

private:
	//Посчитать текущий активный детектор
	void _cdecl MissionSoundEnvironmentManager::UpdateScene(float dltTime, long level);
	//Сбросить состояние ящиков
	void BoxesUnuse();
	//Устонавить состояние ящика в используемое
	bool BoxesUse(MissionSoundEnvironment * env);
	//Удалить неиспользуемые ящики
	bool BoxesRemoveUnuse();

private:
	//Список ящиков в месте слушателя
	array<ActiveBox> boxes;	
	//Матрица слушателя
	Matrix listener;
	//Время подмешивания
	float blendTime;
	//Массив пресетов
	ISoundScene::Enveronment envSetups[20];
	ConstString evtNames[20];
};

#endif
