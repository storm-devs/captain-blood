//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxAnimation system
//============================================================================================
// AnxAnimation
//============================================================================================

#ifndef _AnxAnimation_h_
#define _AnxAnimation_h_

#include "AnxPlayer.h"


//Описание функции принимающей события анимации
typedef void (__fastcall * AnimationTimeEvent)(void * pointer, AnxAnimation * ani);
//
typedef void (__fastcall * AnimationExecuteEvent)(void * pointer, AnxAnimation * ani, const char * eventName, const char ** params, dword numParams);

class AnxBlendStage
{
public:
	virtual ~AnxBlendStage(){};
	//Получить необходимость в данных предыдущих стадий
	virtual bool NeedPrevStageData(long boneIndex) = null;
	//Получить коэфициент блендинга для интересующей кости
	virtual float GetBoneBlend(long boneIndex) = null;
	//Получить трансформацию кости
	virtual void GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale) = null;

};

class AnxAnimation
{
	friend class AnxPlayer;
	struct Bone
	{		
		Quaternion aniq;		//Анимированный угол поворота кости
		Vector4 anip;			//Анимированный позиция кости
		Quaternion resq;		//Результирующий угол поворота кости
		Vector4 resp;			//Результирующая позиция кости
		Vector4 ress;			//Результирующий масштаб кости
		dword update;			//Текущие состояние кости
		long parent;			//Индекс родителя
		AnxBone * desk;			//Описание кости
		long index;				//Индекс кости
	};

//--------------------------------------------------------------------------------------------
public:
	AnxAnimation(AnxData & _data);
	virtual ~AnxAnimation();

//-----------------------------------------------------------------
public:
	//Получить имя анимации
	const char * GetName();

	//Начать проигрывание анимации с заданного нода
	bool Start(const char * nodeNameId = null, long clipIndex = -1);
	//Перейти на заданный нод с текущего
	bool Goto(const char * nodeNameId, float blendTime, long clipIndex);
	//Активировать линк
	bool ActivateLink(const char * commandId, bool forceApply, bool onlyCheck);
	//Выбрать случайную позицию проигрывания
	void RandomizePosition();
	
//-----------------------------------------------------------------
public:
	//Получить имя текущего нода
	const char * CurrentNode();
	//Доступиться до константы типа string
	const char * GetConstString(const char * constName, const char * nodeName = null);
	//Доступиться до константы типа float
	float GetConstFloat(const char * constName, const char * nodeName = null);
	//Доступиться до константы типа blend
	float GetConstBlend(const char * constName);

//--------------------------------------------------------------------------------------------
public:
	//Получить количество костей в анимации
	long GetNumBones();
	//Получить имя кости
	const char * GetBoneName(long index);
	//Получить индекс родительской кости
	long GetBoneParent(long index);
	//Найти по имени кость
	long FindBone(const char * boneName, bool shortName);
	//Найти по имени кость (короткое имя)
	long FindBoneUseHash(const char * boneName, unsigned long hash);

	//Получить текущее в анимации сстояние кости
	void GetAnimationBone(long index, Quaternion & q, Vector & p);
	//Получить ориентацию кости с учётом блендеров, без иерархии
	const Quaternion & GetBoneRotate(long index);
	//Получить позицию кости с учётом блендеров, без иерархии
	const Vector & GetBonePosition(long index);
	//Получить масштаб кости с учётом блендеров, без иерархии
	const Vector & GetBoneScale(long index);
	//Получить матрицу кости
	const Matrix & GetBoneMatrix(long index);
	//Получить массив матриц
	const Matrix * GetBoneMatrices();

	//Получить текущую дельту смещения
	void GetMovement(Vector & deltaPos);

	//Являеться ли текущий нод стоповым
	bool CurrentNodeIsStop();

	
//--------------------------------------------------------------------------------------------
//Функиции для редактора
//--------------------------------------------------------------------------------------------
public:
	//Установить текущий кадр основного проигрывателя
	void SetCurrentFrame(float frame);
	//Получить текущий кадр основного проигрывателя
	float GetCurrentFrame();
	//Получить количество кадров текущего клипа основного проигрывателя
	dword GetNumberOfFrames();
	//Использует ли кость основного проигрывателя глобальные позиции в данный момент
	bool IsBoneUseGlobalPos(long boneIndex);
	//Установить скорость проигрывания
	void SetFPS(float fps);
	//Заблокировать переход по линкам графа
	void BlockGraphLinks(bool isBlock);

//--------------------------------------------------------------------------------------------
public:
	//Получить количество событий в анимации
	long GetNumEvents();
	//Получить имя и параметры события
	const char * GetEvent(long index, const char ** & params, dword & numParams);


//--------------------------------------------------------------------------------------------
public:
	//Расчитать новую позицию
	void Update(float dltTime);
	//Установить обработчик эвентов
	void SetAnimationExecuteEvent(AnimationExecuteEvent event, void * pointer);
	//Установить временной обработчик
	void SetTimeEvent(AnimationTimeEvent event, void * pointer, float timeStep);
private:
	//Расчитать новую позицию
	void UpdateStep(float dltTime);

//--------------------------------------------------------------------------------------------
private:
	//Добавить события в буфер ожидания отправки
	void AddEventToTransferBuffer(AnxEvent & evt);
	//Найти нод
	AnxNode * FindNode(const char * nodeName);
	//Найти нод по идентификатору
	AnxNode * FindNodeById(const char * nodeNameId);
	//Обновить состояние кости без учета иерархии
	void UpdateBone(long index);
	//Просчитать иерархию для кости
	void HeirarchyBoneUpdate(long index);
	//Событие о просшествии временого отрезка
	void TimeEvent(AnxPlayer * pl);
	//Получить временной интервал
	float GetTimeEventStep(AnxPlayer * pl);
	//Проверить список миксеров
	void CheckMixers();

//--------------------------------------------------------------------------------------------
public:
	//Получить данные
	const AnxData & GetConstData() const;
	//Получить индекс родительской кости
	long GetBoneParentNoCheck(long index);
	//Узнать обновлены все кости или нет
	bool IsAllBonesUpdated();
	//Узнать обновлена ли кость
	bool IsBonesUpdated(long index);

//--------------------------------------------------------------------------------------------
public:
	array<AnxEvent *> events;		//Буферизированные события
	array<AnxBlendStage *> extMixer;//Внешний миксер


	AnxHeader & header;				//Заголовок анимационных данных
	AnxData & data;					//Контейнер анимационных данных

//--------------------------------------------------------------------------------------------
private:	
	Bone * bones;					//Информация, описывающая кость
	dword numBones;					//Количество костей в анимации
	Matrix * matrices;				//Результирующие матрицы
	AnxPlayer * mixer[4];			//Порядок смешения анимаций
	dword mixers;					//Количество миксеров, используемых в анимации
	dword updateCount;				//Счётчик исполнений		
	byte * buffer;					//Единый буфер для костей и матриц
	AnxPlayer players[4];			//Плееры, проигрывающии анимацию	
	AnxLink gotoLink;				//Линк для програмно управляемых переходов
	long gotoLinkSelectClip;		//Индекс выбраного клипа при переходе на нод
	AnimationTimeEvent timeEvent;	//Обработчик временного события
	void * timeEventPointer;		//Параметр обработчика
	float timeEventStep;			//Временной итнтервал с которым надо вызывать события
	AnimationExecuteEvent event;	//Немедленное исполнение эвентов
	void * eventPointer;			//Параметр обработчика эвентов
	bool isAllBonesUpdated;			//Устанавливается, когда все кости были посчитаны
	bool isBlockGraphLinks;			//Блокировать работу линков графа
};

//Получить данные
inline const AnxData & AnxAnimation::GetConstData() const
{
	return data;
}

//Получить индекс родительской кости
inline long AnxAnimation::GetBoneParentNoCheck(long index)
{
	return bones[index].parent;
}

//Получить текущий кадр основного проигрывателя
inline float AnxAnimation::GetCurrentFrame()
{
	if(!mixers) return 0.0f;
	Assert(mixer[0]);
	return mixer[0]->GetCurrentFrame();
}

//Установить текущий кадр основного проигрывателя
inline void AnxAnimation::SetCurrentFrame(float frame)
{
	if(!mixers) return;
	Assert(mixer[0]);
	mixer[0]->SetCurrentFrame(frame);
	Update(0.0f);
	updateCount++;
}

//Получить временной интервал
inline float AnxAnimation::GetTimeEventStep(AnxPlayer * pl)
{
	if(!mixers || mixer[0] != pl || !timeEvent) return 0.0f;
	return timeEventStep;
}

//Узнать обновлены все кости или нет
inline bool AnxAnimation::IsAllBonesUpdated()
{
	return isAllBonesUpdated;
}

//Узнать обновлена ли кость
inline bool AnxAnimation::IsBonesUpdated(long index)
{
	return (bones[index].update == updateCount);
}

#endif

