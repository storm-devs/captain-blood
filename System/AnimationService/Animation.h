//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Animation service
//============================================================================================
// Animation
//============================================================================================

#ifndef _Animation_h_
#define _Animation_h_


#include "..\..\common_h\core.h"
#include "..\..\common_h\Animation.h"
#include "..\..\common_h\templates\array.h"
#include "..\..\common_h\templates\string.h"
#include "..\..\common_h\AnimationNativeAccess.h"

#include "AnxAnimation\AnxAnimation.h"
#include "AnxAnimation\AnxData.h"

class AnimationService;
class AnimationScene;

class Animation : public IAnimation  
{
	friend AnimationService;


	enum PauseFlags
	{
		pf_work = 0,
		pf_pause = 1,
		pf_stopnode = 2,
	};


	struct EventHandler
	{
		const char * eventName;
		IAnimationListener * listener;
		AniEvent func;
	};

	class BStage : public AnxBlendStage
	{
	public:

		//Получить необходимость в данных предыдущих стадий
		virtual bool NeedPrevStageData(long boneIndex)
		{
			return stage->NeedPrevStageData(boneIndex);
		}

		//Получить коэфициент блендинга для интересующей кости
		virtual float GetBoneBlend(long boneIndex)
		{
			return stage->GetBoneBlend(boneIndex);
		}

		//Получить трансформацию кости
		virtual void GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale)
		{
			return stage->GetBoneTransform(boneIndex, rotation, position, scale, prevRotation, prevPosition, prevScale);
		}

		IAniBlendStage * stage;
		long level;
	};
	

//--------------------------------------------------------------------------------------------
public:
	Animation(AnxData & data, AnimationService & aserv, AnimationScene & ascene, const char * _name, const char * _cppFile, long _cppLine);
	virtual ~Animation();

//--------------------------------------------------------------------------------------------
//Управление копиями
//--------------------------------------------------------------------------------------------
public:
	//Копировать интерфейс
	virtual IAnimationTransform * Clone();
	//Удалить интерфейс
	virtual void Release();
	//Удалить интерфейс принудительно с сообщением об ошибке
	void ForceRelease();

//--------------------------------------------------------------------------------------------
//Управление проигрыванием
//--------------------------------------------------------------------------------------------
public:
	//Начать проигрывание анимации с заданного нода
	virtual bool Start(const char * nodeName, bool isInstant);
	//Перейти на заданный нод с текущего
	virtual bool Goto(const char * nodeName, float blendTime);
	//Активировать линк
	virtual bool ActivateLink(const char * command, bool forceApply);
	//Проверить, можно ли в текущий момент активировать линк
	virtual bool IsCanActivateLink(const char * command);
	//Установить анимацию на паузу
	virtual void Pause(bool isPause);
	//Анимацию на паузе или работает
	virtual bool IsPause();
	//Являеться ли текущий нод стоповым
	virtual bool CurrentNodeIsStop();
	//Установить скорость проигрывания анимации
	virtual void SetPlaySpeed(float k);
	//Получить скорость проигрывания анимации
	virtual float GetPlaySpeed();
	//Выбрать случайную позицию проигрывания
	virtual void RandomizePosition();

	//Обновить анимацию
	void UpdateAnimation(float dltTime);
	//Обновить стадии блендинга
	void UpdateStages(float dltTime);
	//Разослать события
	void SendEvents();

//--------------------------------------------------------------------------------------------
//Утилитные функции
//--------------------------------------------------------------------------------------------
public:
	//Получить имя текущего нода
	virtual const char * CurrentNode();
	//Доступиться до константы типа string
	virtual const char * GetConstString(const char * constName, const char * nodeName);
	//Доступиться до константы типа float
	virtual float GetConstFloat(const char * constName, const char * nodeName);
	//Доступиться до константы типа blend
	virtual float GetConstBlend(const char * constName);
	//Получить имя анимации
	virtual const char * GetName();

	//Получить внутреннюю информацию графа
	virtual bool GetNativeGraphInfo(GraphNativeAccessor & accessor) const;

//--------------------------------------------------------------------------------------------
//Доступ к костям анимации
//--------------------------------------------------------------------------------------------	
public:
	//Получить количество костей в анимации
	virtual long GetNumBones();
	//Получить имя кости
	virtual const char * GetBoneName(long index);
	//Получить индекс родительской кости
	virtual long GetBoneParent(long index);
	//Найти по имени кость
	virtual long FindBone(const char * boneName, bool shortName);
	//Найти по имени кость (короткое имя)
	virtual long FindBoneUseHash(const char * boneName, dword hash);

	//Получить текущее в анимации сотояние кости
	virtual void GetAnimationBone(long index, Quaternion & q, Vector & p);
	//Получить ориентацию кости с учётом блендеров, без иерархии
	virtual const Quaternion & GetBoneRotate(long index);
	//Получить позицию кости с учётом блендеров, без иерархии
	virtual const Vector & GetBonePosition(long index);
	//Получить масштаб кости с учётом блендеров, без иерархии
	virtual const Vector & GetBoneScale(long index);
	//Получить матрицу кости
	virtual const Matrix & GetBoneMatrix(long index);
	//Получить массив матриц
	virtual const Matrix * GetBoneMatrices();

	//Получить текущую дельту смещения
	virtual void GetMovement(Vector & deltaPos);

	//Зарегистрировать стадию блендинга скелета
	virtual void RegistryBlendStage(IAniBlendStage * stage, long level);
	//Освободить стадию блендинга
	virtual void UnregistryBlendStage(IAniBlendStage * stage);
	//Проверить что блендер уже отрелижен
	virtual void UnregistryBlendStageCheck(IAniBlendStage * stage);

//--------------------------------------------------------------------------------------------
//Система сообщений
//--------------------------------------------------------------------------------------------
public:
	//Установить функцию обработчика на исполнение
	virtual bool SetEventHandler(IAnimationListener * listener, AniEvent func, const char * eventName);
	//Удалить функцию обработчика
	virtual void DelEventHandler(IAnimationListener * listener, AniEvent func, const char * eventName);
	//Удалить все функции обработчика
	virtual void DelAllEventHandlers(IAnimationListener * listener);
	//Установить обработчик временных интервалов в анимации
	virtual void SetTimeEventHandler(IAnimationListener * listener, float timeStep);
	//Получить шаг временных интервалов в анимации, если меньше 0, то неустановлен режим
	virtual float GetTimeEventStep();


//--------------------------------------------------------------------------------------------	
public:
	//Расчитать кости из вспомогательного потока
	void ThreadUpdateBones();
	//Проверить не отписанный блендер и листенер
	void UnregistryCheck(IAniBlendStage * bs, IAnimationListener * lis);
	//Получить анимацию
	AnxAnimation * GetAnx();


//--------------------------------------------------------------------------------------------	
private:
	static void __fastcall Event(void * pointer, AnxAnimation * ani, const char * eventName, const char ** params, dword numParams);
	static void __fastcall TimeEvent(void * pointer, AnxAnimation * ani);

	bool SetNativeInfoGlobal(AGNA_GlobalInfo & accessor) const;	
	bool SetNativeInfoNodeByName(AGNA_NodeInfo & accessor) const;	
	bool SetNativeInfoNodeByIndex(AGNA_NodeInfo & accessor) const;	
	bool SetNativeInfoLink(AGNA_LinkInfo & accessor) const;	
	bool SetNativeInfoClip(AGNA_ClipInfo & accessor) const;	
	bool SetNativeInfoEvent(AGNA_EventInfo & accessor) const;
	bool SetNativeInfoEvent(AGNA_GlobalEventInfo & accessor) const;
	bool SetNativeInfoCurrentFrameGet(AGNA_GetCurrentFrame & accessor) const;
	bool SetNativeInfoCurrentFrameSet(AGNA_SetCurrentFrame & accessor) const;
	bool SetNativeInfoNumberOfFrames(AGNA_GetNumberOfFrames & accessor) const;
	bool SetNativeInfoIsBoneUseGlobalPos(AGNA_IsBoneUseGlobalPos & accessor) const;
	bool SetNativeInfoSetFPS(AGNA_SetFPS & accessor) const;
	bool SetNativeGotoNodeClip(AGNA_GotoNodeClip & accessor) const;
	bool SetNativePause(AGNA_AnimationPause & accessor) const;
	bool SetNativeBlockControl(AGNA_BlockControl & accessor) const;

	bool IsBlockControl() const;

//--------------------------------------------------------------------------------------------	
private:
	AnxAnimation animation;			//Анимация
	dword isPause;					//Запаузена анимация или работает
	string name;					//Имя анимации
	long refCounter;				//Счётчик ссылок
	AnimationService & service;		//Сервис анимации
	AnimationScene & scene;			//Сцена, которой принадлежит анимация
	array<BStage> stages;			//Список стадий
	array<EventHandler> events;		//Список желающих получать события
	IAnimationListener * timeEvent;	//Обработчики временных событий
	long updateStageIndex;			//Индекс текущей исполняемой стадии
	float timeEventStep;			//Шаг, с которым работает временное событие
	long currentHandler;			//Текущий процессируемый подписчик
	float kSpeed;					//Скорость проигрывания анимации
	CritSection synchroUpdate;		//Синхронизация обновления
	const char * cppFile;			//Исходный Файл в котором был создан объект
	long cppLine;					//Строка от куда был создан объект
#ifndef _XBOX
#ifndef NO_TOOLS
	dword isBlockControl;			//Заблокировать стандартное управление анимацией
#endif
#endif

};

//Получить анимацию для редактора
inline AnxAnimation * Animation::GetAnx()
{
	return &animation;
}

#endif

