//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnimationService	
//============================================================================================

#ifndef _AnimationService_h_
#define _AnimationService_h_


#include "Animation.h"
#include "AnimationProcedural.h"
#include "IAnxEditorServiceAccess.h"

#ifndef STOP_DEBUG

//#define DEBUG_PIX_ENABLE
//#define DEBUG_THREADS

#endif


class IFileService;


//Анимационная сцена
class AnimationScene : public IAnimationScene
{
	enum CommandId
	{
		cmd_empty,
		cmd_pause,
		cmd_start,
		cmd_goto,
		cmd_activate_link,
		cmd_rand_pos,
	};

	struct Command
	{
		word id;
		word intData;
		const char * strData;
		float floatData;
		Animation * ani;
	};

public:
	AnimationScene(AnimationService & s, const char * _cppFile, long _cppLine);
	virtual ~AnimationScene();

	//Удалить сцену
	virtual void Release();
	//Принудительное удаление
	dword ForceRelease();
	//Создать анимацию
	virtual IAnimation * Create(const char * path, const char * cppFile, long cppLine);
	//Создать пустую анимацию
	virtual IAnimation * CreateEmpty(const Bone * skeleton, dword count, const char * cppFile, long cppLine);
	//Создать процедурную анимацию
	virtual IAnimationProcedural * CreateProcedural(const Bone * skeleton, dword count, const char * cppFile, long cppLine);
	//Обновить состояние сцены
	void UpdateScene(float dltTime);
	//Удалить анимацию из списка
	void Delete(Animation * ani);
	//Удалить процедурную анимацию из списка
	void Delete(AnimationProcedural * ani);
	//Установить анимацию на обновление
	void SetForUpdate(Animation * ani, bool isUpdate);

public:
	void Command_Start(Animation * ani, const char * nameId);
	void Command_Goto(Animation * ani, const char * nameId, float blendTime, long clipIndex);
	void Command_ActivateLink(Animation * ani, const char * nameId, bool forceApply);
	void Command_Pause(Animation * ani, bool isPause);
	void Command_RandomizePosition(Animation * ani);
	void Command_ClearQueue(const Animation * ani);


#ifndef STOP_DEBUG
	//Проверить не отписанный блендер и листенер
	void UnregistryCheck(IAniBlendStage * bs, IAnimationListener * lis);
#endif

#ifndef NO_TOOLS
	//Создать анимацию основываясь на данных редактора
	Animation * Editor_Create(const void * data, dword size);
#endif

private:
	AnimationService & service;
	//Закреплёные за сценой анимации	
	array<Animation *> animations;
	//Включенные анимации
	array<Animation *> updatingAnimations;
	//Процедурные анимации, работающие по сильно упрощёной схеме
	array<AnimationProcedural *> procedural;
	//Буфер отложенных команд
	array<Command> commands;
	//Происходит ли обновление сцены сейчас
	dword isUpdateProcess;
	//Исходный Файл в котором был создан объект
	const char * cppFile;
	//Строка от куда был создан объект
	long cppLine;
	//Занимаемая всеми анимациями память
	dword debugUsageMemory;
};



class AnimationService : public IAnimationService  
{
public:
	struct AnimationData
	{		
		dword hash;
		dword len;
		long next;
		long prev;
		long refCounter;
		AnxData * data;
	};

//--------------------------------------------------------------------------------------------
//Инициализация
//--------------------------------------------------------------------------------------------
public:
	AnimationService();
	virtual ~AnimationService();

private:
	virtual bool Init();
	//Вызываеться перед удалением сервисов
	void PreRelease();


//--------------------------------------------------------------------------------------------
//Создание, удаление сцен и анимаций
//--------------------------------------------------------------------------------------------
public:
	//Создать анимационные данные из файла
	AnxData * Create(const char * path);
	//Создать уникальные данные для пустой анимации
	AnxData * CreateEmpty(const IAnimationScene::Bone * skeleton, dword count);
	//Освободить анимационные данные
	void ReleaseData(AnxData * data);
	//Создать сцену с анимациями
	virtual IAnimationScene * CreateScene(const char * cppFile, long cppLine);
	//Удалить анимационную сцену из списка
	void ReleaseScene(AnimationScene * scene);
	//Получить объем анимации
	dword GetAnimationsDataSize();

private:	
	array<AnimationScene *> scenes;			//Сцены	
	array<AnimationData> anxData;			//Загруженные анимационные данные
	long firstFreeData;						//Первый свободный для использования элемент
	string buffer;							//Буфер для работы с именами
	long entryData[1024];					//Входная таблица для поиска анимаций по имени

//--------------------------------------------------------------------------------------------
//Управление дополнительным потоком
//--------------------------------------------------------------------------------------------
public:
	//Добавить анимацию для обсчёта в потоке на следующем кадре
	void ThreadAddAnimationToUpdate(Animation * ani);
	//Извлечь анимацию из очереди обновления, если невозможно то остановить поток
	void ThreadRemoveAnimationToUpdate(Animation * ani);

private:
	//Завершить цикл обсчёта матриц
	void ThreadStart();
	//Завершить поток обсчёта матриц
	void ThreadStop();
	//Приостановить цикл обсчёта матриц
	void ThreadPause();
	//Возобновить цикл обсчёта матриц
	void ThreadResume();
	//Извлечь анимацию из очереди обновления, если невозможно то остановить поток
	void ThreadPrepareQueue();
	//Поток обновления матриц
	static DWORD WINAPI ThreadMatricesUpdater(LPVOID lpParameter);

private:
	HANDLE threadUpdater;		//Поток апдейта матриц
	HANDLE eventProcess;		//Событие разрешающее работу апдейтера
	HANDLE eventReady;			//Событие сигнализирующие состояния потока апдейтера
	volatile bool threadWork;	//Флаг сигнализирующий работу потока
	volatile bool threadPause;	//Флаг требующий паузы от потока
	CritSection changeIndex;	//Секция изменения индекса

	//Очередь анимаций для обновления в потоке
	array<Animation *> threadAnimationsQueue;
	//Индекс текущей обновляемой в сцене анимации
	volatile dword threadAnimationIndex;
	//Будующая очередь анимаций
	array<Animation *> newThreadAnimationsQueue;


//--------------------------------------------------------------------------------------------
//Общее
//--------------------------------------------------------------------------------------------
public:
	//Исполнение в конце кадра
	virtual void EndFrame(float dltTime);
	//Обновить анимационную сцену
	virtual void Update(IAnimationScene * scene, float dltTime);
	//Получить занимаемую анимацией память
	virtual dword GetUsageMemory();
	//Разрешён ли трейс об незначительных ошибках
	bool IsEnableTrace();

public:
	//Разрешено выводить сообщения
	bool enableTrace;
	//Файловый сервис
	IFileService * fs;


//--------------------------------------------------------------------------------------------
//Функции для редактора
//--------------------------------------------------------------------------------------------
#ifndef NO_TOOLS
public:
	//В редакторе может быть только однопоточный режим
	void Editor_NoThread();
#endif


//--------------------------------------------------------------------------------------------
//Оптимизация, отладка
//--------------------------------------------------------------------------------------------
#ifndef STOP_DEBUG
public:
	//Добавить счётчик промахнувшегося вычисления
	void AddMissGetBoneMatrices();
	//Добавить счётчик промахнувшегося вычисления отдельной кости
	void AddMissGetSingleBoneMatrix();
	//Добавить счётчик вычисления на дополнительном потоке
	void AddThreadUpdateBoneMatricesCounter();
	//Проверить не отписанный блендер и листенер
	virtual void UnregistryCheck(IAniBlendStage * bs, IAnimationListener * lis);

private:
	long missGetBoneMatricesCounter;		//Счётчик анимаций обсчитанных в основном потоке	
	long missGetSingleBoneMatrixCounter;	//Счётчик анимаций обсчитанных  в основном потоке	
	long threadUpdateBoneMatricesCounter;	//Счётчик анимаций обсчитанных в основном потоке	
	ICoreStorageLong * bonesCounter;		//Переменная для отладочной информации	
#endif
	dword debugUsageMemory;					//Общая занимаемая память


};

#ifndef NO_TOOLS

class AnxEditorServiceAccess : public IAnxEditorServiceAccess
{
public:
	//Запретить потоки
	virtual void DisableThreads();
	//Создать анимацию из памяти
	virtual IAnimation * CreateAnimation(IAnimationScene * scene, const void * data, unsigned long size);
};

#endif

//Разрешён ли трейс об незначительных ошибках
__forceinline bool AnimationService::IsEnableTrace()
{
	return enableTrace;
}

#endif

