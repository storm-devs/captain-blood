//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Animation service
//============================================================================================
// Animation
//============================================================================================

#ifndef _IAnimation_h_
#define _IAnimation_h_

#include "Math3D.h"
#include "core.h"
#include "templates\array.h"


//Обработчик событий анимации
class IAnimation;
class IAnimationListener;
class IAniBlendStage;

//Описание функции принимающей события анимации
typedef void (_cdecl IAnimationListener::* AniEvent)(IAnimation * ani, const char * name, const char ** params, dword numParams);

enum IAnimationType
{
	anitype_transform,
	anitype_animation,
	anitype_procedural,
};


//Анимация матриц
class IAnimationTransform
{
protected:
	virtual ~IAnimationTransform(){};

//--------------------------------------------------------------------------------------------
//Утилитные функции
//--------------------------------------------------------------------------------------------
public:
	//Получить имя анимации
	virtual const char * GetName() = null;

//--------------------------------------------------------------------------------------------
//Управление копиями
//--------------------------------------------------------------------------------------------
public:
	//Копировать интерфейс
	virtual IAnimationTransform * Clone() = null;
	//Удалить интерфейс
	virtual void Release() = null;

//--------------------------------------------------------------------------------------------
//Доступ к костям анимации
//--------------------------------------------------------------------------------------------
public:
	//Получить количество костей в анимации
	virtual long GetNumBones() = null;
	//Получить имя кости
	virtual const char * GetBoneName(long index) = null;
	//Получить индекс родительской кости
	virtual long GetBoneParent(long index) = null;
	//Найти по имени кость
	virtual long FindBone(const char * boneName, bool shortName = false) = null;
	//Найти по имени кость (короткое имя)
	virtual long FindBoneUseHash(const char * boneName, dword hash) = null;

	//Получить матрицу кости с учётом иерархии
	virtual const Matrix & GetBoneMatrix(long index) = null;
	//Получить массив матриц с учётом иерархии
	virtual const Matrix * GetBoneMatrices() = null;

	virtual bool Is(IAnimationType type)
	{
		return type == anitype_transform;
	}

};



//Анимация описанная в файле
class IAnimation : public IAnimationTransform
{
public:
	class EventID
	{
		friend IAnimation;
	public:
		EventID()
		{
			id = null;
		}
	private:
		const char * id;
	};

	struct GraphNativeAccessor
	{
	protected:
		dword accessorType;
	public:
		inline dword GetType()
		{
			return accessorType;
		};
	};

//--------------------------------------------------------------------------------------------
//Управление проигрыванием
//--------------------------------------------------------------------------------------------
public:
	//Начать проигрывание анимации с заданного нода
	virtual bool Start(const char * nodeName = null, bool isInstant = false) = null;
	//Перейти на заданный нод с текущего
	virtual bool Goto(const char * nodeName, float blendTime) = null;
	//Активировать линк
	virtual bool ActivateLink(const char * command, bool forceApply = false) = null;
	//Проверить, можно ли в текущий момент активировать линк
	virtual bool IsCanActivateLink(const char * command) = null;
	//Установить анимацию на паузу
	virtual void Pause(bool isPause) = null;
	//Анимацию на паузе или работает
	virtual bool IsPause() = null;
	//Являеться ли текущий нод стоповым
	virtual bool CurrentNodeIsStop() = null;
	//Установить скорость проигрывания анимации
	virtual void SetPlaySpeed(float k) = null;
	//Получить скорость проигрывания анимации
	virtual float GetPlaySpeed() = null;
	//Выбрать случайную позицию проигрывания
	virtual void RandomizePosition() = null;

//--------------------------------------------------------------------------------------------
//Утилитные функции
//--------------------------------------------------------------------------------------------
public:
	//Получить имя текущего нода
	virtual const char * CurrentNode() = null;
	//Доступиться до константы типа string
	virtual const char * GetConstString(const char * constName, const char * nodeName = null) = null;
	//Доступиться до константы типа float
	virtual float GetConstFloat(const char * constName, const char * nodeName = null) = null;
	//Доступиться до константы типа blend
	virtual float GetConstBlend(const char * constName) = null;

	//Получить внутреннюю информацию графа
	virtual bool GetNativeGraphInfo(GraphNativeAccessor & accessor) const = null;

//--------------------------------------------------------------------------------------------
//Доступ к костям анимации
//--------------------------------------------------------------------------------------------
public:
	//Пересчитать состояние кости перед считыванием параметров
	__forceinline void UpdateBone(long index)
	{
		GetBoneMatrix(index);
	}
	//Получить текущее в анимации сстояние кости
	virtual void GetAnimationBone(long index, Quaternion & q, Vector & p) = null;
	//Получить ориентацию кости с учётом блендеров, без иерархии
	virtual const Quaternion & GetBoneRotate(long index) = null;
	//Получить позицию кости с учётом блендеров, без иерархии
	virtual const Vector & GetBonePosition(long index) = null;
	//Получить масштаб кости с учётом блендеров, без иерархии
	virtual const Vector & GetBoneScale(long index) = null;

	//Получить текущую дельту смещения
	virtual void GetMovement(Vector & deltaPos) = null;

	//Зарегистрировать стадию блендинга скелета
	virtual void RegistryBlendStage(IAniBlendStage * stage, long level) = null;
	//Освободить стадию блендинга
	virtual void UnregistryBlendStage(IAniBlendStage * stage) = null;
	//Проверить что блендер уже отрелижен
	virtual void UnregistryBlendStageCheck(IAniBlendStage * stage) = null;

//--------------------------------------------------------------------------------------------
//Система сообщений
//--------------------------------------------------------------------------------------------
public:
	//Установить функцию обработчика на исполнение
	virtual bool SetEventHandler(IAnimationListener * listener, AniEvent func, const char * eventName) = null;
	//Удалить функцию обработчика
	virtual void DelEventHandler(IAnimationListener * listener, AniEvent func, const char * eventName) = null;
	//Удалить все функции обработчика
	virtual void DelAllEventHandlers(IAnimationListener * listener) = null;
	//Установить обработчик временных интервалов в анимации
	virtual void SetTimeEventHandler(IAnimationListener * listener, float timeStep) = null;
	//Получить шаг временных интервалов в анимации, если меньше 0, то неустановлен режим
	virtual float GetTimeEventStep() = null;

	virtual bool Is(IAnimationType type)
	{
		if(type == anitype_animation) return true;
		return IAnimationTransform::Is(type);
	}


protected:
	void AddAnimationToListener(IAnimationListener * listener);
	void RemoveAnimationFromListener(IAnimationListener * listener);
	void RemoveAnimationFromBlendStage(IAniBlendStage * stage);
	const char * & AccessToEventID(EventID & id);
};

//Процедурная анимация (не участвует в многопоточности)
class IAnimationProcedural : public IAnimationTransform
{
public:
	//Установить пользовательскую матрицу кости
	virtual void SetBoneMatrix(long index, const Matrix & mtx) = null;
	//Установить нулевую матрицу кости
	virtual void CollapseBone(long index) = null;

	virtual bool Is(IAnimationType type)
	{
		if(type == anitype_procedural) return true;
		return IAnimationTransform::Is(type);
	}
};

//Обработчик событий анимации
class IAnimationListener
{
	friend IAnimation;
public:
	IAnimationListener() : ani(_FL_, 4)
	{
	};

	virtual ~IAnimationListener()
	{
		for(long i = 0; i < ani; i++)
		{
			ani[i]->DelAllEventHandlers(this);
		}
		ani.Empty();
		UnregistryCheck();
	};

	//Прошёл заданный временной промежуток в анимации
	virtual void TimeEvent(IAnimation * a){};

protected:
	virtual void AddHandlers(IAnimation * anx)
	{
		anx->SetEventHandler(this, null, null);
	};
	virtual void DelHandlers(IAnimation * anx)
	{
		anx->DelEventHandler(this, null, null);
	};
	//Обновился список анимаций
	virtual void StartUpdateAnimationsList(){};
	virtual void EndUpdateAnimationsList(){};

private:
	void UnregistryCheck();


protected:
	array<IAnimation *> ani;
};

inline void IAnimation::AddAnimationToListener(IAnimationListener * listener)
{
	if(!listener) return;
	for(long i = 0; i < listener->ani; i++)
	{
		if(listener->ani[i] == this) return;
	}
	listener->StartUpdateAnimationsList();
	listener->ani.Add(this);
	listener->EndUpdateAnimationsList();
}

inline void IAnimation::RemoveAnimationFromListener(IAnimationListener * listener)
{
	if(!listener) return;
	listener->StartUpdateAnimationsList();
	listener->ani.Del(this);
	listener->EndUpdateAnimationsList();
}

inline const char * & IAnimation::AccessToEventID(EventID & id)
{
	return id.id;
}

//Стиадия блендинга
class IAniBlendStage
{
	friend IAnimation;
public:
	IAniBlendStage()
	{
		ani = null;
	}

	IAniBlendStage(IAnimation * _ani, long level)
	{
		ani = _ani;
		RegistryBlendStage(level);
	}
	
	
	//(вызывает синхронизацию)
	virtual ~IAniBlendStage()
	{
		UnregistryCheck();
	}
	//Получить необходимость в данных предыдущих стадий (многопоточная функция)
	virtual bool NeedPrevStageData(long boneIndex)
	{
		return false;
	}
	//Получить коэфициент блендинга для интересующей кости (многопоточная функция)
	virtual float GetBoneBlend(long boneIndex) = null;
	//Получить трансформацию кости (многопоточная функция)
	virtual void GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale) = null;
	//Обновить (только основной поток, второй спит вне цикла исполнения)
	virtual void Update(float dltTime){};

public:
	//Установить анимацию
	inline void SetAnimation(IAnimation * _ani)
	{
		if(ani == _ani) return;
		if(ani)
		{
			UnregistryBlendStage();
		}
		ani = _ani;
	}
	//Получить анимацию
	inline IAnimation * Ani()
	{
		return ani;
	}

	//Зарегистрировать стадию блендинга скелета (вызывает синхронизацию)
	inline void RegistryBlendStage(long level)
	{
		if(ani)
		{
			ani->RegistryBlendStage(this, level);
		}
	}

	//Освободить стадию блендинга (вызывает синхронизацию)
	inline void UnregistryBlendStage()
	{
		if(ani)
		{
			ani->UnregistryBlendStage(this);
		}
	}

private:
	void UnregistryCheck();

private:
	IAnimation * ani;
};

inline void IAnimation::RemoveAnimationFromBlendStage(IAniBlendStage * stage)
{
	stage->ani = null;
}


//Анимационная сцена
class IAnimationScene
{
protected:
	virtual ~IAnimationScene(){};
public:
	struct Bone
	{
		Bone()
		{
			parentIndex = -1;
			name = null;
		};

		long parentIndex;	//Индекс родительской кости
		const char * name;	//Имя кости
		Matrix mtx;		//Трансформация для процедурной анимации
	};

public:
	//Удалить сцену
	virtual void Release() = null;
public:
	//Создать анимацию
	virtual IAnimation * Create(const char * path, const char * cppFile, long cppLine) = null;
	//Создать пустую анимацию
	virtual IAnimation * CreateEmpty(const Bone * skeleton, dword count, const char * cppFile, long cppLine) = null;
	//Создать процедурную анимацию
	virtual IAnimationProcedural * CreateProcedural(const Bone * skeleton, dword count, const char * cppFile, long cppLine) = null;

};


//Сервис анимации
class IAnimationService : public Service
{
public:
	virtual ~IAnimationService(){};

public:
	//Создать сцену с анимациями
	virtual IAnimationScene * CreateScene(const char * cppFile, long cppLine) = null;
	//Обновить анимационную сцену
	virtual void Update(IAnimationScene * scene, float dltTime) = null;
#ifndef STOP_DEBUG
	//Проверить на отписаность блендер и листенер (если не отписан то вызывает исключение)
	virtual void UnregistryCheck(IAniBlendStage * bs, IAnimationListener * lis) = null;
#endif
public:
	//Получить занимаемую анимацией память
	virtual dword GetUsageMemory() = null;
};

__forceinline void IAnimationListener::UnregistryCheck()
{
#ifndef STOP_DEBUG
	IAnimationService * s = (IAnimationService *)api->GetService("AnimationService");
	Assert(s);
	s->UnregistryCheck(null, this);
#endif
}

__forceinline void IAniBlendStage::UnregistryCheck()
{
#ifndef STOP_DEBUG
	IAnimationService * s = (IAnimationService *)api->GetService("AnimationService");
	Assert(s);
	s->UnregistryCheck(this, null);
#endif
}

#endif

