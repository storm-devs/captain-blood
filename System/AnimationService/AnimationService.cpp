//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// AnimationService	
//============================================================================================
			
#include "AnimationService.h"
#include "..\..\Common_h\FileService.h"
#include "..\..\Common_h\SetThreadName.h"

//============================================================================================


CREATE_SERVICE(AnimationService, 30)


//#define DEBUG_THREADS

//============================================================================================


AnimationScene::AnimationScene(AnimationService & s, const char * _cppFile, long _cppLine) : service(s),
																			animations(_FL_, 1024),
																			updatingAnimations(_FL_, 1024),
																			procedural(_FL_, 256),
																			commands(_FL_)
{
	isUpdateProcess = false;
	cppFile = _cppFile;
	cppLine = _cppLine;
	debugUsageMemory = 0;
}

AnimationScene::~AnimationScene()
{
	service.ReleaseScene(this);
	while(animations.Size() > 0)
	{
		animations[0]->ForceRelease();
	}
	while(procedural.Size() > 0)
	{
		procedural[0]->ForceRelease();
	}	
	
}

//Удалить сцену
void AnimationScene::Release()
{
	AssertCoreThread
	delete this;
}

//Принудительное удаление
dword AnimationScene::ForceRelease()
{
	dword count = animations.Size() + procedural.Size();
	Release();
	return count;
}

//Создать анимацию
IAnimation * AnimationScene::Create(const char * path, const char * cppFile, long cppLine)
{
#ifdef DEBUG_THREADS
	api->Trace("AnimationScene::Create; Scene = %.8x", this);
#endif
	AssertCoreThread
	AnxData * data = service.Create(path);
	if(!data)
	{
		return null;
	}
	Animation * ani = NEW Animation(*data, service, *this, path, cppFile, cppLine);
	animations.Add(ani);	
	return ani;
}

//Создать пустую анимацию
IAnimation * AnimationScene::CreateEmpty(const Bone * skeleton, dword count, const char * cppFile, long cppLine)
{
#ifdef DEBUG_THREADS
	api->Trace("AnimationScene::CreateEmpty; Scene = %.8x", this);
#endif
	AssertCoreThread
	AnxData * data = service.CreateEmpty(skeleton, count);
	Assert(data);
	Animation * ani = NEW Animation(*data, service, *this, data->GetName(), cppFile, cppLine);
	animations.Add(ani);
	return ani;
}

//Создать процедурную анимацию
IAnimationProcedural * AnimationScene::CreateProcedural(const Bone * skeleton, dword count, const char * cppFile, long cppLine)
{
	AssertCoreThread
	AnimationProcedural * ap = NEW AnimationProcedural(*this, skeleton, count, cppFile, cppLine);	
	procedural.Add(ap);
	return ap;
}

//Обновить состояние сцены
__forceinline void AnimationScene::UpdateScene(float dltTime)
{
	//Дополнительный поток уже гарантированно остановлен
	AssertCoreThread	
#ifdef DEBUG_THREADS
	api->Trace("AnimationScene::UpdateScene -> srtart update; Scene = %.8x", this);
#endif
	//Исполняем накопленные команды
	for(long i = 0; i < commands; i++)
	{
		Command & cmd = commands[i];
		AnxAnimation * anx = cmd.ani->GetAnx();
		switch(cmd.id)
		{
		case cmd_pause:
			SetForUpdate(cmd.ani, cmd.intData == 0);
			break;
		case cmd_start:
			anx->Start(cmd.strData);
			break;
		case cmd_goto:
			anx->Goto(cmd.strData, cmd.floatData, (short)cmd.intData);
			break;
		case cmd_activate_link:
			{
				bool res = anx->ActivateLink(cmd.strData, cmd.intData != 0, false);
				if(!res && service.IsEnableTrace())
				{
					const char * node = anx->CurrentNode() ? anx->CurrentNode() : "<no node>";
					if(!cmd.strData) cmd.strData = "<none>";
					api->Error("Animation <%s>: link not been activated!\n    Node: %s. Link: %s", cmd.ani->GetName(), node, cmd.strData);
				}
			}
			break;
		case cmd_rand_pos:
			anx->RandomizePosition();
			break;
		}
	}
	commands.Empty();
	isUpdateProcess = true;
	//Процессируем анимации
	for(long i = 0; i < updatingAnimations; i++)
	{
		updatingAnimations[i]->UpdateAnimation(dltTime);
	}
	//Процессируем стадии блендинга
	for(long i = 0; i < updatingAnimations; i++)
	{
		updatingAnimations[i]->UpdateStages(dltTime);
	}
	//Отсылаем сообщения
	for(long i = 0; i < updatingAnimations; i++)
	{
		updatingAnimations[i]->SendEvents();
	}
	isUpdateProcess = false;
#ifdef DEBUG_THREADS
	api->Trace("AnimationScene::UpdateScene -> continue; Scene = %.8x", this);
#endif
}

//Удалить анимацию из списка
void AnimationScene::Delete(Animation * ani)
{
#ifdef DEBUG_THREADS
	api->Trace("AnimationScene::Delete(%0.8x); Scene = %.8x", ani, this);
#endif
	AssertCoreThread
	service.ThreadRemoveAnimationToUpdate(ani);
	Assert(isUpdateProcess == 0);
	animations.Del(ani);
	updatingAnimations.Del(ani);
	for(long i = 0; i < commands; i++)
	{
		Command & cmd = commands[i];
		if(cmd.ani == ani)
		{
			cmd.id = cmd_empty;
			cmd.ani = null;
		}
	}
}

//Удалить процедурную анимацию из списка
void AnimationScene::Delete(AnimationProcedural * ani)
{
	AssertCoreThread
	Assert(isUpdateProcess == 0);
	procedural.Del(ani);
}

//Установить анимацию на обновление
void AnimationScene::SetForUpdate(Animation * ani, bool isUpdate)
{
	AssertCoreThread
	Assert(isUpdateProcess == 0);
	if(isUpdate)
	{
		for(dword i = 0; i < updatingAnimations.Size(); i++)
		{
			if(updatingAnimations[i] == ani)
			{
				return;
			}
		}
		updatingAnimations.Add(ani);
	}else{
		updatingAnimations.Del(ani);
	}
}


void AnimationScene::Command_Start(Animation * ani, const char * nameId)
{
	Command & cmd = commands[commands.Add()];
	cmd.id = cmd_start;
	cmd.strData = nameId;
	cmd.ani = ani;
}

void AnimationScene::Command_Goto(Animation * ani, const char * nameId, float blendTime, long clipIndex)
{
	Command & cmd = commands[commands.Add()];
	cmd.id = cmd_goto;
	cmd.intData = (word)clipIndex;
	cmd.strData = nameId;
	cmd.floatData = blendTime;
	cmd.ani = ani;
}

void AnimationScene::Command_ActivateLink(Animation * ani, const char * nameId, bool forceApply)
{
	Command & cmd = commands[commands.Add()];
	cmd.id = cmd_activate_link;
	cmd.strData = nameId;
	cmd.intData = forceApply ? 1 : 0;
	cmd.ani = ani;
}

void AnimationScene::Command_Pause(Animation * ani, bool isPause)
{
	Command & cmd = commands[commands.Add()];
	cmd.id = cmd_pause;
	cmd.intData = isPause ? 1 : 0;
	cmd.ani = ani;
}

void AnimationScene::Command_RandomizePosition(Animation * ani)
{
	Command & cmd = commands[commands.Add()];
	cmd.id = cmd_rand_pos;
}

void AnimationScene::Command_ClearQueue(const Animation * ani)
{
	if(!ani)
	{
		commands.Empty();
	}
	for(dword i = 0; i < commands.Size(); )
	{
		if(commands[i].ani == ani && commands[i].id != cmd_pause)
		{
			commands.DelIndex(i);
		}else{
			i++;
		}
	}	
}


#ifndef STOP_DEBUG

//Проверить не отписанный блендер и листенер
__forceinline void AnimationScene::UnregistryCheck(IAniBlendStage * bs, IAnimationListener * lis)
{
	for(dword i = 0; i < animations.Size(); i++)
	{
		animations[i]->UnregistryCheck(bs, lis);
	}
}

#endif


#ifndef NO_TOOLS

//Создать анимацию основываясь на данных редактора
Animation * AnimationScene::Editor_Create(const void * data, dword size)
{
	AnxData * adata = NEW AnxData();
	adata->SetData(data);
	adata->SetUserData(-1);
	Animation * ani = NEW Animation(*adata, service, *this, "*Editor animation*", _FL_);
	animations.Add(ani);
	return ani;
}

#endif


//============================================================================================


AnimationService::AnimationService() : scenes(_FL_, 32),
										anxData(_FL_, 256),




										threadAnimationsQueue(_FL_, 256),
										newThreadAnimationsQueue(_FL_, 256)
{
	//Создание, удаление сцен и анимаций
	firstFreeData = -1;
	for(long i = 0; i < ARRSIZE(entryData); i++)
	{
		entryData[i] = -1;
	}
	//Управление дополнительным потоком
	threadUpdater = null;
	eventProcess = null;
	eventReady = null;	
	threadWork = false;
	threadPause = false;
	threadAnimationIndex = 0;
	//Общее	
	fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	IIniFile * pIni = fs->SystemIni();
	if(pIni)
	{
		enableTrace = pIni->GetLong("Animation", "Trace", 0) != 0;
	}else{
		enableTrace = false;
	}
#ifndef STOP_DEBUG
	//Оптимизация, отладка
	missGetBoneMatricesCounter = 0;
	missGetSingleBoneMatrixCounter = 0;
	threadUpdateBoneMatricesCounter = 0;
	bonesCounter = null;
#endif
	debugUsageMemory = 0;
	//Если нужно запускаем дополнительный поток
	if(api->GetThreadingInfo() != ICore::mt_none)
	{
		ThreadStart();
	}
}

AnimationService::~AnimationService()
{
	ThreadStop();
	long count = 0;
	while(scenes.Size() > 0)
	{
		count += scenes[0]->ForceRelease();
	}
	for(long i = 0; i < anxData; i++)
	{
		AnimationData & d = anxData[i];
		if(d.data)
		{
			delete d.data;
			d.data = null;
		}
	}
	if(count)
	{
		api->Trace("AnimationService: total leaks = %i.", count);
	}else{
		api->Trace("AnimationService released successfully.");
	}
	threadUpdater = null;
	eventProcess = null;
	eventReady = null;
#ifndef STOP_DEBUG
	RELEASE(bonesCounter);
#endif
}

bool AnimationService::Init()
{
	api->SetEndFrameLevel(this, Core_DefaultExecuteLevel + 0x1);
#ifndef STOP_DEBUG
	bonesCounter = api->Storage().GetItemLong("statistics.totalBonesCount", _FL_);
#endif
	return true;
}


//Вызываеться перед удалением сервисов
void AnimationService::PreRelease()
{
	ThreadStop();
}

//============================================================================================
//Создание, удаление сцен и анимаций
//============================================================================================

//Создать анимацию
AnxData * AnimationService::Create(const char * path)
{
	//Корректность имени
	if(!path || !path[0]) return null;
	//Переводим путь в имя
	buffer.Empty();
	long lastChar = string::Len(path);
	for(long startName = lastChar; startName >= 0; startName--)
	{
		if(path[startName] == '\\' || path[startName] == '/')
		{
			break;
		}
	}
	startName++;
	for(long endName = lastChar; endName >= 0; endName--)
	{
		if(path[endName] == '.')
		{
			break;
		}
	}
	if(endName < 0)
	{
		endName = lastChar;
	}
	buffer.Reserve(endName - startName + 1);
	for(long i = startName; i < endName; i++)
	{
		buffer += path[i];
	}
	buffer.Lower();
	//Ищем среди загруженных
	dword len = 0;
	dword hash = string::Hash(buffer.c_str(), len);	
	dword index = hash & (ARRSIZE(entryData) - 1);
	for(long i = entryData[index]; i >= 0; )
	{
		AnimationData & d = anxData[i];
		if(d.hash == hash)
		{
			if(d.len == len)
			{
				if(buffer == d.data->GetName())
				{
					d.refCounter++;
					return d.data;
				}
			}
		}
		i = d.next;
	}
	//Надо загрузить новую анимацию
	AnxData * anx = NEW AnxData();
	anx->SetName(buffer.c_str());
	buffer = "Resource\\Animation\\";
	buffer += path;
	buffer.CheckPath().Lower().AddExtention(".anx");
	IDataFile * df = fs->OpenDataFile(buffer.c_str(), file_open_default, _FL_);
	if(!df)
	{
		if(enableTrace)
		{
			api->Trace("Animation -> Can't open animation file: %s", buffer.c_str());
		}
		delete anx;
		return null;
	}
	if(!anx->LoadData(df))
	{
		df->Release();
		if(enableTrace)
		{
			api->Trace("Animation -> Damaged animation file: %s", buffer.c_str());
		}
		delete anx;
		return null;
	}
	df->Release();
	//Добавляем запись в таблицу
	if(firstFreeData >= 0)
	{
		i = firstFreeData;
		firstFreeData = anxData[i].next;
	}else{
		i = anxData.Add();
	}
	AnimationData & d = anxData[i];
	d.hash = hash;
	d.len = len;
	d.next = entryData[index];
	if(d.next >= 0)
	{
		anxData[d.next].prev = i;
	}
	entryData[index] = i;
	d.prev = -1;
	d.refCounter = 1;
	d.data = anx;
	anx->SetUserData(i);
	if(d.next >= 0)
	{
		Assert(d.prev != d.next);
	}
	debugUsageMemory += anx->GetDataSize();
	return anx;
}

//Создать пустую анимацию
AnxData * AnimationService::CreateEmpty(const IAnimationScene::Bone * skeleton, dword count)
{
	FixMe
	return null;
}

//Освободить анимационные данные
void AnimationService::ReleaseData(AnxData * data)
{
	//Получаем запись
	long i = (long)data->GetUserData();
	if(i < 0)
	{
		//Это анимация редактора
		delete data;
		return;
	}
	AnimationData & d = anxData[i];	
	d.refCounter--;
	if(d.refCounter > 0)
	{
		return;
	}
	//Удаляем данные
	debugUsageMemory -= d.data->GetDataSize();
	d.refCounter = 0;
	delete d.data;
	d.data = null;
	//Переносим из именной цепочки в список свободных
	if(d.next >= 0)
	{
		anxData[d.next].prev = d.prev;
	}
	if(d.prev >= 0)	
	{
		anxData[d.prev].next = d.next;
	}else{
		Assert(entryData[(d.hash & (ARRSIZE(entryData) - 1))] == i);
		entryData[d.hash & (ARRSIZE(entryData) - 1)] = d.next;
	}
	d.prev = -1;
	d.next = firstFreeData;
	firstFreeData = i;
}

//Создать сцену с анимациями
IAnimationScene * AnimationService::CreateScene(const char * cppFile, long cppLine)
{
	AnimationScene * scene = NEW AnimationScene(*this, cppFile, cppLine);
	scenes.Add(scene);
	return scene;
}

//Удалить анимационную сцену из списка
void AnimationService::ReleaseScene(AnimationScene * scene)
{
	scenes.Del(scene);
}



//============================================================================================
//Управление дополнительным потоком
//============================================================================================


//Добавить анимацию для обсчёта в потоке на следующем кадре
void AnimationService::ThreadAddAnimationToUpdate(Animation * ani)
{
#ifdef DEBUG_THREADS
	api->Trace("AnimationService::ThreadAddAnimationToUpdate, ani = %.8x", ani);
#endif
	for(long i = 0; i < newThreadAnimationsQueue; i++)
	{
		if(newThreadAnimationsQueue[i] == ani)
		{
			return;
		}
	}
	newThreadAnimationsQueue.Add(ani);	
}

//Извлечь анимацию из очереди обновления, если невозможно то остановить поток
void AnimationService::ThreadRemoveAnimationToUpdate(Animation * ani)
{
#ifdef DEBUG_PIX_ENABLE
	PIXBeginNamedEvent(0, "Ani-remove");
#endif
	newThreadAnimationsQueue.Del(ani);
	for(long i = 0; i < threadAnimationsQueue; i++)
	{
		if(threadAnimationsQueue[i] == ani)
		{
			bool isNeedPause = false;
			changeIndex.Enter();
#ifdef DEBUG_THREADS
			api->Trace("changeIndex.Enter()");
#endif
			if(threadAnimationIndex != i)
			{
				threadAnimationsQueue[i] = null;
			}else{
				isNeedPause = true;
			}
#ifdef DEBUG_THREADS
			api->Trace("changeIndex.Leave()");
#endif
			changeIndex.Leave();
			if(isNeedPause)
			{
#ifdef DEBUG_THREADS
				api->Trace("AnimationService::ThreadRemoveAnimationToUpdate; Cant remove ani from queue, pause thread, ani = %.8x", ani);
#endif
				ThreadPause();
			}else{
#ifdef DEBUG_THREADS
				api->Trace("AnimationService::ThreadRemoveAnimationToUpdate; Sucsesful remove ani from queue, ani = %.8x", ani);
#endif
			}
#ifdef DEBUG_PIX_ENABLE
			PIXEndNamedEvent();	
#endif
			return;
		}
	}
#ifdef DEBUG_THREADS
	api->Trace("AnimationService::ThreadRemoveAnimationToUpdate; Can't find ani in queue, ani = %.8x", ani);
#endif
#ifdef DEBUG_PIX_ENABLE
	PIXEndNamedEvent();	
#endif
}


//Запустить поток обсчёта матриц
void AnimationService::ThreadStart()
{
	AssertCoreThread
#ifdef DEBUG_THREADS
	api->Trace("AnimationService::ThreadStart() -> Start");
#endif
	if(threadUpdater)
	{
		return;
	}
	threadPause = false;
	//Создаём эвенты
	eventProcess = ::CreateEvent(null, false, false, null);
	if(!eventProcess)
	{
		return;
	}
	eventReady = ::CreateEvent(null, true, false, null);
	if(!eventReady)
	{
		::CloseHandle(eventProcess);
		return;
	}
	//Создаём поток апдейта матриц
	threadAnimationIndex = 0;
	DWORD dwAnimThreadID = 0;
	threadUpdater = ::CreateThread(null, 0, ThreadMatricesUpdater, this, CREATE_SUSPENDED, &dwAnimThreadID);
	if(!threadUpdater)
	{
		::CloseHandle(eventProcess);
		::CloseHandle(eventReady);
#ifdef DEBUG_THREADS
		api->Trace("AnimationService::ThreadStart() -> Error");
#endif
		return;
	}
#ifdef _XBOX
	::SetThreadPriority(threadUpdater, THREAD_PRIORITY_TIME_CRITICAL);
	::XSetThreadProcessor(threadUpdater, 1);
#endif
	XSetThreadName(dwAnimThreadID, "Animation::MatricesUpdater");
	::ResumeThread(threadUpdater);
	//Переводим поток в паузу
	ThreadPause();
#ifdef DEBUG_THREADS
	api->Trace("AnimationService::ThreadStart() -> Ready");
#endif
};

//Завершить поток обсчёта матриц
void AnimationService::ThreadStop()
{
	AssertCoreThread
#ifdef DEBUG_THREADS
	api->Trace("AnimationService::ThreadStop() -> Start");
#endif
	if(!threadUpdater)
	{
#ifdef DEBUG_THREADS
		api->Trace("AnimationService::ThreadStop() -> Already stoped");
#endif
		return;
	}
	Sleep(1);
	//Сообщаем что среду пора остановится
	threadWork = false;
	threadPause = false;
	//Ждём пока поток не закончил работу
	do{
		::SetEvent(eventProcess);
	}while(::WaitForSingleObject(eventReady, 1) == WAIT_TIMEOUT);
	threadUpdater = null;
	if(eventProcess)
	{
		::CloseHandle(eventProcess);
		eventProcess = null;
	}
	if(eventReady)
	{
		::CloseHandle(eventReady);
		eventReady = null;
	}
#ifdef DEBUG_THREADS
	api->Trace("AnimationService::ThreadStop() -> Stop");
#endif
};

//Приостановить цикл обсчёта матриц
void AnimationService::ThreadPause()
{
	if(!threadUpdater) return;
	AssertCoreThread
	if(threadPause)
	{
#ifdef DEBUG_THREADS
		if(threadUpdater)
		{
			api->Trace("AnimationService::ThreadPause() -> Already paused");
		}else{
			api->Trace("AnimationService::ThreadPause() -> Can't pause, thread is stoped");
		}		
#endif
		return;
	}
#ifdef DEBUG_PIX_ENABLE	
	PIXBeginNamedEvent(0, "Ani-pause");
#endif
	//Укажем потоку что нужна пауза
	threadPause = true;
	//Ждём пока поток перейдёт в режим ожидания	
	while(::WaitForSingleObject(eventReady, 0) == WAIT_TIMEOUT)
	{
		Sleep(0);
	}	
	threadAnimationsQueue.Empty();
#ifdef DEBUG_THREADS
	api->Trace("AnimationService::ThreadPause() -> Pause");
#endif
#ifdef DEBUG_PIX_ENABLE	
	PIXEndNamedEvent();
#endif
}

//Возобновить цикл обсчёта матриц
void AnimationService::ThreadResume()
{
	if(!threadUpdater) return;
	AssertCoreThread
	//Сначала ставим поток на паузу для достижения точки синхронизации
	ThreadPause();
#ifdef DEBUG_PIX_ENABLE	
	PIXBeginNamedEvent(0, "Ani-resume");
#endif
	//Укажем что пауза больше не нужна
	threadPause = false;
	//Снимаем сигнал паузы
	::ResetEvent(eventReady);
	//Запускаем поток
	::SetEvent(eventProcess);
#ifdef DEBUG_THREADS
	api->Trace("AnimationService::ThreadResume() -> Resume thread");
#endif
#ifdef DEBUG_PIX_ENABLE
	PIXEndNamedEvent();
#endif
}

//Извлечь анимацию из очереди обновления, если невозможно то остановить поток
void AnimationService::ThreadPrepareQueue()
{
	ThreadPause();
#ifdef DEBUG_THREADS
	api->Trace("AnimationService --============= Prepare thread queue =============--");
#endif
	threadAnimationsQueue.Empty();
	threadAnimationsQueue.AddElements(newThreadAnimationsQueue.Size());
	for(long i = 0; i < newThreadAnimationsQueue; i++)
	{
		threadAnimationsQueue[i] = newThreadAnimationsQueue[i];
	}
	newThreadAnimationsQueue.Empty();
	threadAnimationIndex = 0;
#ifdef DEBUG_THREADS
	api->Trace("AnimationService --============= Second frame =============--");
#endif
	ThreadResume();
}

//Поток обновления матриц
DWORD WINAPI AnimationService::ThreadMatricesUpdater(LPVOID lpParameter)
{
	AnimationService * service = (AnimationService *)lpParameter;
	service->threadWork = true;
	while(service->threadWork)
	{
		//Проходим по всем сценам и по всем анимациям обновляя их
		if(!service->threadPause && service->threadAnimationIndex < service->threadAnimationsQueue.Size())
		{
			Animation * ani = service->threadAnimationsQueue[service->threadAnimationIndex];
			if(ani)
			{
#ifdef DEBUG_THREADS
				api->Trace("Thread update matrices for ani = %.8x", ani);
#endif
				ani->ThreadUpdateBones();
			}
#ifdef DEBUG_PIX_ENABLE
			PIXBeginNamedEvent(0, "Ani-threadAnimationIndex++");
#endif
			service->changeIndex.Enter();
#ifdef DEBUG_THREADS
			api->Trace("service->changeIndex.Enter()");
#endif
			service->threadAnimationIndex++;
#ifdef DEBUG_THREADS
			api->Trace("service->changeIndex.Leave()");
#endif
			service->changeIndex.Leave();
#ifdef DEBUG_PIX_ENABLE
			PIXEndNamedEvent();
#endif
		}else{
#ifdef DEBUG_THREADS
			api->Trace("AnimationService::ThreadMatricesUpdater() -> Activate pause");
#endif
			//Ждём паузу
#ifdef DEBUG_PIX_ENABLE
			PIXBeginNamedEvent(0, "Ani-wait new work");
#endif
			::SignalObjectAndWait(service->eventReady, service->eventProcess, INFINITE, false);
#ifdef DEBUG_PIX_ENABLE
			PIXEndNamedEvent();
#endif
#ifdef DEBUG_THREADS
			api->Trace("AnimationService::ThreadMatricesUpdater() -> Resume from pause, queue size = %u", service->threadAnimationsQueue.Size());
#endif
		}
	}
	//Сообщим что сред более не опасен
	::SetEvent(service->eventReady);
	return 0;
}


//============================================================================================
//Общее
//============================================================================================

//Исполнение
void AnimationService::EndFrame(float dltTime)
{
	//Прекращаем работу потока, если он досих пор активен
	ThreadPause();	
	//Обновляем очередь исполняемой анимации
#ifndef STOP_PROFILES
	//dword queuesize = newThreadAnimationsQueue.Size();
	//dword tubmc = threadUpdateBoneMatricesCounter;
	//threadUpdateBoneMatricesCounter = 0;
#endif
	ThreadPrepareQueue();
#ifndef STOP_PROFILES	
	//const char * bonesCount = "Bones count";
	//api->SetPerformanceCounter(bonesCount, float(manager.bonesCounter));
	//bonesCounter->Set(manager.bonesCounter);
	//manager.bonesCounter = 0;
	//	PIXAddNamedCounter(float(queuesize), "Ani - queue");
	//	PIXAddNamedCounter(float(missGetBoneMatricesCounter), "Ani - all cals in main thread");
	//	PIXAddNamedCounter(float(missGetSingleBoneMatrixCounter), "Ani - bone cals in main thread");
	//	PIXAddNamedCounter(float(tubmc), "Ani - all calc in add thread");
	//missGetBoneMatricesCounter = 0;
	//missGetSingleBoneMatrixCounter = 0;

#endif
}

//Обновить анимационную сцену
void AnimationService::Update(IAnimationScene * scene, float dltTime)
{
	if(!scene) return;
	ThreadPause();
	((AnimationScene *)scene)->UpdateScene(dltTime);
}

//Получить занимаемую анимацией память
dword AnimationService::GetUsageMemory()
{
	return debugUsageMemory;
}

//============================================================================================
//Функции для редактора
//============================================================================================

#ifndef NO_TOOLS
//В редакторе может быть только однопоточный режим
void AnimationService::Editor_NoThread()
{
	ThreadStop();
}

/*
Animation * AnimationService::FindAnimation(const char * fileName)
{
	Assert(fileName);
	for(long i = 0; i < animations; i++)
	{
		if(animations[i]->deleted) continue;
		if(crt_stricmp(animations[i]->GetAnx()->GetName(), fileName) == 0) return (Animation *)animations[i];
	}
	return null;
}

bool AnimationService::ForceSetAnxFile(AnxFile * forceFile)
{
	this->forceFile = forceFile;
	bool cr = creationResult;
	creationResult = true;
	return cr;
}

void AnimationService::DisableCreate(bool isDisable)
{
	this->isDisable = isDisable;
}
*/

#endif

//============================================================================================
//Оптимизация, отладка
//============================================================================================


#ifndef STOP_DEBUG

//Добавить счётчик промахнувшегося вычисления
void AnimationService::AddMissGetBoneMatrices()
{
	missGetBoneMatricesCounter++;
}

//Добавить счётчик промахнувшегося вычисления отдельной кости
void AnimationService::AddMissGetSingleBoneMatrix()
{
	missGetSingleBoneMatrixCounter++;
}

//Добавить счётчик вычисления на дополнительном потоке
void AnimationService::AddThreadUpdateBoneMatricesCounter()
{
	threadUpdateBoneMatricesCounter++;
}

//Проверить не отписанный блендер и листенер
void AnimationService::UnregistryCheck(IAniBlendStage * bs, IAnimationListener * lis)
{
	//Проверяем в сценах
	for(dword i = 0; i < scenes.Size(); i++)
	{
		scenes[i]->UnregistryCheck(bs, lis);
	}
}

#endif


#ifndef NO_TOOLS

CREATE_SERVICE(AnxEditorServiceAccess, 1000)

//Запретить потоки
void AnxEditorServiceAccess::DisableThreads()
{
	AnimationService * serv = (AnimationService *)api->GetService("AnimationScene");
	if(serv)
	{
		serv->Editor_NoThread();
	}
}

//Создать анимацию из памяти
IAnimation * AnxEditorServiceAccess::CreateAnimation(IAnimationScene * scene, const void * data, unsigned long size)
{
	return ((AnimationScene *)scene)->Editor_Create(data, size);
}


#endif