//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Animation service
//============================================================================================
// Animation
//============================================================================================
			
#include "Animation.h"
#include "AnimationService.h"

//============================================================================================

Animation::Animation(AnxData & data, AnimationService & aserv, AnimationScene & ascene, 
						const char * _name, const char * _cppFile, long _cppLine) : animation(data),
																					service(aserv),
																					scene(ascene),
																					stages(_FL_),
																					events(_FL_)
																					
{
#ifndef _XBOX
#ifndef NO_TOOLS
	isBlockControl = 0;
#endif
#endif
	isPause = 0;
	animation.SetAnimationExecuteEvent(Event, this);	
	name = _name;
	refCounter = 1;
	timeEvent = null;
	updateStageIndex = -1;
	timeEventStep = 0.05f;
	currentHandler = -1;
	kSpeed = 1.0f;
	cppFile = _cppFile;
	cppLine = _cppLine;
	Pause(false);
}

Animation::~Animation()
{
	scene.Delete(this);
	Assert(refCounter == 0);
	while(stages > 0)
	{
		IAniBlendStage * stage = stages[stages - 1].stage;		
		UnregistryBlendStage(stage);
		RemoveAnimationFromBlendStage(stage);
	}
	EventHandler * evs = events.GetBuffer();
	long evsCount = events.Size();
	for(long i = 0; i < evsCount; i++)
	{
		if(!evs[i].listener) continue;
		RemoveAnimationFromListener(evs[i].listener);
		for(long j = i + 1; j < evsCount; j++)
		{
			if(evs[i].listener == evs[j].listener)
			{
				evs[j].listener = null;
			}
		}
	}
	events.Empty();
	service.ReleaseData(&animation.data);
}


//============================================================================================
//Управление копиями
//============================================================================================

//Копировать интерфейс
IAnimationTransform * Animation::Clone()
{
	refCounter++;
	return this;
}

//Удалить интерфейс
void Animation::Release()
{
	refCounter--;
	if(refCounter <= 0)
	{
		delete this;
	}
}

//Удалить интерфейс принудительно
void Animation::ForceRelease()
{
	api->Trace("Animation -> Don't release animation (cpp: %s, %u)", cppFile, cppLine);
	refCounter = 1;
	Release();
}

//============================================================================================
//Управление проигрыванием
//============================================================================================

//Начать проигрывание анимации с заданного нода
bool Animation::Start(const char * nodeName, bool isInstant)
{
	if(IsBlockControl()) return false;
	const char * nameId = null;
	if(nodeName)
	{
		nameId = animation.GetConstData().GetHeader().FindName(nodeName, AnxFndName::f_node);
		if(!nameId)
		{
			return false;
		}
	}
	if(!isInstant)
	{
		isPause &= ~pf_stopnode;
		scene.Command_Pause(this, isPause != 0);
		scene.Command_Start(this, nameId);
	}else{
		service.ThreadRemoveAnimationToUpdate(this);
		return animation.Start(nameId);
	}
	return true;
}

//Перейти на заданный нод с текущего
bool Animation::Goto(const char * nodeName, float blendTime)
{
	if(IsBlockControl()) return false;
	const char * nameId = animation.GetConstData().GetHeader().FindName(nodeName, AnxFndName::f_node);
	if(!nameId)
	{
		return false;
	}
	isPause &= ~pf_stopnode;
	scene.Command_Pause(this, isPause != 0);
	scene.Command_Goto(this, nameId, blendTime, -1);
	return true;
}


//Активировать линк
bool Animation::ActivateLink(const char * command, bool forceApply)
{
	if(IsBlockControl()) return false;
	if(animation.CurrentNodeIsStop())
	{
		api->Error("Animation <%s>: link not been activated, animation not on stop node!\n    Link: %s", GetName(), command ? command : "<null>");
		return false;
	}
	if(string::IsEmpty(command))
	{
		api->Error("Animation <%s>: link not been activated, it name is empty!\n    Link: %s", GetName(), command ? command : "<null>");
		return false;		
	}
	const char * nameId = animation.GetConstData().GetHeader().FindName(command, AnxFndName::f_link);
	if(!nameId)
	{
		api->Error("Animation <%s>: link not been activated, link name is invalidate for this animation!\n    Link: %s", GetName(), command ? command : "<null>");
		return false;
	}
	scene.Command_ActivateLink(this, nameId, forceApply);
	return animation.ActivateLink(nameId, false, true);
}

//Проверить, можно ли в текущий момент активировать линк
bool Animation::IsCanActivateLink(const char * command)
{
	if(IsBlockControl()) return false;
	if(animation.CurrentNodeIsStop())
	{
		return false;
	}
	const char * nameId = animation.GetConstData().GetHeader().FindName(command, AnxFndName::f_link);
	return animation.ActivateLink(nameId, false, true);
}

//Установить анимацию на паузу
void Animation::Pause(bool isPause)
{
	if(IsBlockControl()) return;
	if(isPause)
	{
		this->isPause |= pf_pause;
	}else{
		this->isPause &= ~pf_pause;
	}	
	scene.Command_Pause(this, isPause);
}

//Анимацию на паузе или работает
bool Animation::IsPause()
{
	return (isPause & pf_pause) != 0;
}

//Являеться ли текущий нод стоповым
bool Animation::CurrentNodeIsStop()
{
	if(IsBlockControl()) return false;
	return animation.CurrentNodeIsStop();
}

//Установить скорость проигрывания анимации
void Animation::SetPlaySpeed(float k)
{
	if(IsBlockControl()) return;
	kSpeed = Clampf(k, 0.0f, 10.0f);
}

//Получить скорость проигрывания анимации
float Animation::GetPlaySpeed()
{
	return kSpeed;
}

//Обновить анимацию
void Animation::UpdateAnimation(float dltTime)
{
//	Assert(!isPause);
	if(animation.CurrentNodeIsStop())
	{
		animation.Update(0.0f);
		this->isPause |= pf_stopnode;
		scene.Command_Pause(this, isPause != 0);
		return;
	}
	dltTime *= kSpeed;
	if(dltTime <= 1e-35f)
	{
		return;
	}
	animation.Update(dltTime);
}

//Обновить стадии блендинга
void Animation::UpdateStages(float dltTime)
{
//	Assert(!isPause);
	dltTime *= kSpeed;
	if(dltTime <= 1e-35f)
	{
		return;
	}
	for(updateStageIndex = 0; updateStageIndex < stages; updateStageIndex++)
	{
		stages[updateStageIndex].stage->Update(dltTime);
	}
}

//Выбрать случайную позицию проигрывания
void Animation::RandomizePosition()
{
	scene.Command_RandomizePosition(this);
}

//Разослать события
void Animation::SendEvents()
{
//	Assert(!isPause);
	for(long i = 0; i < animation.events; i++)
	{
		AnxEvent & evt = *animation.events[i];
		//Ищим принимателей события		
		const char * name = evt.name.ptr;
		for(currentHandler = 0; currentHandler < events; currentHandler++)
		{
			EventHandler & eh = events[currentHandler];
			if(eh.eventName == name)
			{
				(eh.listener->*eh.func)(this, name, evt.params.ptr, evt.numParams);
			}
		}
	}
	animation.events.Empty();
}

//============================================================================================
//Утилитные функции
//============================================================================================

//Получить имя текущего нода
const char * Animation::CurrentNode()
{
	return animation.CurrentNode();
}

//Доступиться до константы типа string
const char * Animation::GetConstString(const char * constName, const char * nodeName)
{
	return animation.GetConstString(constName, nodeName);
}

//Доступиться до константы типа float
float Animation::GetConstFloat(const char * constName, const char * nodeName)
{
	return animation.GetConstFloat(constName, nodeName);
}

//Доступиться до константы типа blend
float Animation::GetConstBlend(const char * constName)
{
	return animation.GetConstBlend(constName);
}

//Получить имя анимации
const char * Animation::GetName()
{
	return name.c_str();
}

//Получить внутреннюю информацию графа
bool Animation::GetNativeGraphInfo(GraphNativeAccessor & accessor) const
{
	switch(accessor.GetType())
	{
	case agna_global:
		return SetNativeInfoGlobal((AGNA_GlobalInfo &)accessor);
	case agna_node_by_name:
		return SetNativeInfoNodeByName((AGNA_NodeInfo &)accessor);
	case agna_node_by_index:
		return SetNativeInfoNodeByIndex((AGNA_NodeInfo &)accessor);
	case agna_link:
		return SetNativeInfoLink((AGNA_LinkInfo &)accessor);	
	case agna_clip:
		return SetNativeInfoClip((AGNA_ClipInfo &)accessor);		
	case agna_event:
		return SetNativeInfoEvent((AGNA_EventInfo &)accessor);
	case agna_global_event:
		return SetNativeInfoEvent((AGNA_GlobalEventInfo &)accessor);		
	case agna_get_frame:
		return SetNativeInfoCurrentFrameGet((AGNA_GetCurrentFrame &)accessor);
	case agna_set_frame:
		return SetNativeInfoCurrentFrameSet((AGNA_SetCurrentFrame &)accessor);
	case agna_get_number_of_frames:
		return SetNativeInfoNumberOfFrames((AGNA_GetNumberOfFrames &)accessor);
	case agna_is_bone_use_global_pos:
		return SetNativeInfoIsBoneUseGlobalPos((AGNA_IsBoneUseGlobalPos &)accessor);
	case agna_set_fps:
		return SetNativeInfoSetFPS((AGNA_SetFPS &)accessor);
	case agna_goto_nodeclip:
		return SetNativeGotoNodeClip((AGNA_GotoNodeClip &)accessor);
	case agna_pause:
		return SetNativePause((AGNA_AnimationPause &)accessor);
	case agna_block_control:
		return SetNativeBlockControl((AGNA_BlockControl &)accessor);
	};
	return false;
}

//============================================================================================
//Доступ к костям анимации
//============================================================================================

//Получить количество костей в анимации
long Animation::GetNumBones()
{
	return animation.GetNumBones();
}

//Получить имя кости
const char * Animation::GetBoneName(long index)
{
	return animation.GetBoneName(index);
}

//Получить индекс родительской кости
long Animation::GetBoneParent(long index)
{
	return animation.GetBoneParent(index);
}

//Найти по имени кость
long Animation::FindBone(const char * boneName, bool shortName)
{
	return animation.FindBone(boneName, shortName);
}

//Найти по имени кость (короткое имя)
long Animation::FindBoneUseHash(const char * boneName, dword hash)
{
	return animation.FindBoneUseHash(boneName, (unsigned long)hash);
}


//Получить текущее в анимации сотояние кости
void Animation::GetAnimationBone(long index, Quaternion & q, Vector & p)
{
	animation.GetAnimationBone(index, q, p);
}

//Получить ориентацию кости с учётом блендеров, без иерархии
const Quaternion & Animation::GetBoneRotate(long index)
{
	return animation.GetBoneRotate(index);
}

//Получить позицию кости с учётом блендеров, без иерархии
const Vector & Animation::GetBonePosition(long index)
{
	return animation.GetBonePosition(index);
}

//Получить масштаб кости с учётом блендеров, без иерархии
const Vector & Animation::GetBoneScale(long index)
{
	return animation.GetBoneScale(index);
}

//Получить матрицу кости
const Matrix & Animation::GetBoneMatrix(long index)
{
#ifdef DEBUG_PIX_ENABLE
	PIXBeginNamedEvent(0, "Ani-GetBoneMatrix");
#endif

	synchroUpdate.Enter();
	//SyncroCode sc(synchroUpdate);
#ifndef STOP_DEBUG
	if(!animation.IsBonesUpdated(index))
	{
		service.AddMissGetSingleBoneMatrix();
	}
#endif
	const Matrix & mtx = animation.GetBoneMatrix(index);
	service.ThreadAddAnimationToUpdate(this);

#ifdef DEBUG_PIX_ENABLE
	PIXEndNamedEvent();
#endif

	synchroUpdate.Leave();
	return mtx;
}

//Получить массив матриц
const Matrix * Animation::GetBoneMatrices()
{
#ifdef DEBUG_PIX_ENABLE
	PIXBeginNamedEvent(0, "Ani-GetBoneMatrices");
#endif	
	SyncroCode sc(synchroUpdate);	
#ifndef STOP_DEBUG
	if(!animation.IsAllBonesUpdated())
	{
		service.AddMissGetBoneMatrices();
	}
#endif
	const Matrix * mtx = animation.GetBoneMatrices();
	service.ThreadAddAnimationToUpdate(this);
#ifdef DEBUG_PIX_ENABLE
	PIXEndNamedEvent();
#endif
	return mtx;	
}

//Получить текущую дельту смещения
void Animation::GetMovement(Vector & deltaPos)
{
	animation.GetMovement(deltaPos);
}

//Зарегистрировать стадию блендинга скелета
void Animation::RegistryBlendStage(IAniBlendStage * stage, long level)
{
	service.ThreadRemoveAnimationToUpdate(this);
	animation.extMixer.Empty();
	UnregistryBlendStage(stage);
	BStage bs;
	bs.stage = stage;
	bs.level = level;
	for(long i = 0; i < stages; i++)
	{
		if(stages[i].level > level)
		{
			stages.Insert(bs, i);
			if(i < updateStageIndex)
			{
				updateStageIndex++;
			}
			break;
		}
	}
	if(i >= stages)
	{
		stages.Add(bs);
	}	
	for(long i = 0; i < stages; i++)
	{
		animation.extMixer.Add(&stages[i]);
	}
}

//Освободить стадию блендинга
void Animation::UnregistryBlendStage(IAniBlendStage * stage)
{	
	if(!stage) return;
	service.ThreadRemoveAnimationToUpdate(this);
	animation.extMixer.Empty();
	for(long i = 0; i < stages; i++)
	{
		if(stages[i].stage == stage)
		{
			stages.DelIndex(i);
			if(i <= updateStageIndex)
			{
				updateStageIndex--;
			}
		}
	}
	for(long i = 0; i < stages; i++)
	{
		animation.extMixer.Add(&stages[i]);
	}
}

//Проверить что блендер уже отрелижен
void Animation::UnregistryBlendStageCheck(IAniBlendStage * stage)
{
	service.ThreadRemoveAnimationToUpdate(this);
	for(long i = 0; i < stages; i++)
	{
		if(stages[i].stage == stage)
		{
			//Перед удалением BlendStage небыла вызванна UnregistryBlendStage
			//Если удалять в деструкторе, то вызов UnregistryBlendStage должен идти первым
			Assert(false);
		}
	}
}

//--------------------------------------------------------------------------------------------
//Система сообщений
//--------------------------------------------------------------------------------------------

//Установить функцию обработчика на исполнение
bool Animation::SetEventHandler(IAnimationListener * listener, AniEvent func, const char * eventName)
{
	//Анализируем входные параметры
	if(!listener || !func) return false;
	if(!eventName || !eventName[0]) return false;
	//Преобразуем указатель в нативный
	eventName = animation.header.FindName(eventName, AnxFndName::f_event);
	//Ищим событие среди добавленных
	for(long i = 0; i < events; i++)
	{
		if(events[i].listener == listener)
		{
			if(events[i].func == func)
			{
				if(events[i].eventName == eventName)
				{
					return true;
				}
			}
		}
	}
	//Добавляем событие
	EventHandler & eh = events[events.Add()];
	eh.eventName = eventName;
	eh.listener = listener;
	eh.func = func;
	AddAnimationToListener(listener);
	return true;
}

//Удалить функцию обработчика
void Animation::DelEventHandler(IAnimationListener * listener, AniEvent func, const char * eventName)
{
	//Ищим событие среди добавленных
	for(long i = 0; i < events; i++)
	{
		if(events[i].listener == listener)
		{
			if(events[i].func == func)
			{
			//	if(events[i].eventName == eventName)
				if(string::IsEqual(events[i].eventName,eventName))
				{
					events.DelIndex(i);
					if(i <= currentHandler) currentHandler--;
					return;
				}
			}
		}
	}
}

//Удалить все функции обработчика
void Animation::DelAllEventHandlers(IAnimationListener * listener)
{
	//Ищим событие среди добавленных
	for(long i = 0; i < events; i++)
	{
		if(events[i].listener == listener)
		{
			events.DelIndex(i);
			if(i <= currentHandler) currentHandler--;
			i = -1;
		}
	}
	if(timeEvent == listener)
	{
		SetTimeEventHandler(null, 0.0f);
	}
}

//Установить обработчик временных интервалов в анимации
void Animation::SetTimeEventHandler(IAnimationListener * listener, float timeStep)
{	
	if(timeStep <= 0.0f) listener = null;
	timeEvent = listener;
	timeEventStep = timeStep;
	if(listener)
	{
		animation.SetTimeEvent(TimeEvent, this, timeEventStep);
	}else{
		animation.SetTimeEvent(null, null, 0.0f);
	}
}

//Получить шаг временных интервалов в анимации, если меньше 0, то неустановлен режим
float Animation::GetTimeEventStep()
{
	if(timeEvent)
	{
		return timeEventStep;
	}
	return -1.0f;
}

//Расчитать кости из вспомогательного потока
void Animation::ThreadUpdateBones()
{
#ifdef DEBUG_PIX_ENABLE
	PIXBeginNamedEvent(0, "Ani-ThreadUpdateBones");
#endif
	if(synchroUpdate.TryEnter())
	{
#ifndef STOP_DEBUG
		if(!animation.IsAllBonesUpdated())
		{
			service.AddThreadUpdateBoneMatricesCounter();
		}			
#endif
		animation.GetBoneMatrices();
		synchroUpdate.Leave();
	}
#ifdef DEBUG_PIX_ENABLE
	PIXEndNamedEvent();
#endif
}

//Проверить не отписанный блендер и листенер
void Animation::UnregistryCheck(IAniBlendStage * bs, IAnimationListener * lis)
{
	for(long i = 0; i < stages; i++)
	{
		Assert(stages[i].stage != bs);
	}
	for(long i = 0; i < events; i++)
	{
		Assert(events[i].listener != lis);
	}	
}


void __fastcall Animation::Event(void * pointer, AnxAnimation * ani, const char * eventName, const char ** params, dword numParams)
{
	Animation * a = (Animation *)pointer;
	Assert(a);
	for(a->currentHandler = 0; a->currentHandler < a->events; a->currentHandler++)
	{
		EventHandler & eh = a->events[a->currentHandler];
		if(eh.eventName == eventName)
		{
			(eh.listener->*eh.func)(a, eventName, params, numParams);
		}
	}
}

void __fastcall Animation::TimeEvent(void * pointer, AnxAnimation * ani)
{
	Animation * a = (Animation *)pointer;
	Assert(a);
	Assert(a->timeEvent);
	a->timeEvent->TimeEvent(a);
}

bool Animation::SetNativeInfoGlobal(AGNA_GlobalInfo & accessor) const
{
	const AnxHeader & header = animation.header;
	accessor.numNodes = header.numNodes;
	accessor.numLinks = header.numLinks;
	accessor.numConstatns = header.numConsts;
	accessor.numEvents = header.numEvents;
	accessor.numClips = header.numClips;
	return true;
}

bool Animation::SetNativeInfoNodeByName(AGNA_NodeInfo & accessor) const
{
	if(!accessor.name) return false;
	const AnxHeader & header = animation.header;
	for(dword i = 0; i < header.numNodes; i++)
	{
		if(string::IsEqual(header.nodes.ptr[i].name.ptr, accessor.name))
		{
			accessor.index = i;
			return SetNativeInfoNodeByIndex(accessor);
		}
	}
	return false;
}

bool Animation::SetNativeInfoNodeByIndex(AGNA_NodeInfo & accessor) const
{
	const AnxHeader & header = animation.header;
	if(accessor.index >= header.numNodes)
	{
		return false;
	}
	AnxNode & node = header.nodes.ptr[accessor.index];
	accessor.name = node.name.ptr;
	accessor.numClips = node.numClips;
	accessor.numConsts = node.numConsts;
	accessor.numLinks = node.numLinks;
	accessor.defLink = node.defLink;
	accessor.isLoop = (node.flags & AnxNode::isLoop) != 0;
	accessor.isChange = (node.flags & AnxNode::isChange) != 0;
	return true;
}

bool Animation::SetNativeInfoLink(AGNA_LinkInfo & accessor) const	
{
	const AnxHeader & header = animation.header;
	if(accessor.nodeIndex >= header.numNodes)
	{
		return false;
	}
	AnxNode & node = header.nodes.ptr[accessor.nodeIndex];
	if(accessor.linkIndex >= node.numLinks)
	{
		return false;
	}	
	AnxLink & link = node.links.ptr[accessor.linkIndex];
	accessor.command = link.name.ptr;
	Assert(link.toNode.ptr);
	accessor.to = link.toNode.ptr - header.nodes.ptr;
	Assert(accessor.to < header.numNodes);
	accessor.arange[0] = link.arange[0];
	accessor.arange[1] = link.arange[1];
	accessor.mrange[0] = link.mrange[0];
	accessor.mrange[1] = link.mrange[1];
	accessor.kBlendTime = link.kBlendTime;
	accessor.syncPos = link.syncPos;
	return true;
}

bool Animation::SetNativeInfoClip(AGNA_ClipInfo & accessor) const	
{
	const AnxHeader & header = animation.header;
	if(accessor.nodeIndex >= header.numNodes)
	{
		return false;
	}
	AnxNode & node = header.nodes.ptr[accessor.nodeIndex];
	if(accessor.clipIndex >= node.numClips)
	{
		return false;
	}	
	AnxClip & clip = node.clips.ptr[accessor.clipIndex];
	accessor.numEvents = clip.numEvents;
	accessor.frames = clip.frames;
	accessor.fps = clip.fps;
	accessor.probability = clip.probability;
	return true;
}

bool Animation::SetNativeInfoEvent(AGNA_EventInfo & accessor) const
{
	const AnxHeader & header = animation.header;
	if(accessor.nodeIndex >= header.numNodes)
	{
		return false;
	}
	AnxNode & node = header.nodes.ptr[accessor.nodeIndex];
	if(accessor.clipIndex >= node.numClips)
	{
		return false;
	}
	AnxClip & clip = node.clips.ptr[accessor.clipIndex];
	if(accessor.eventIndex >= clip.numEvents)
	{
		return false;
	}
	AnxEvent & evt = clip.events.ptr[accessor.eventIndex];
	accessor.name = evt.name.ptr;
	accessor.frame = evt.frame;
	accessor.params = evt.params.ptr;
	accessor.numParams = evt.numParams;
	return true;
}

bool Animation::SetNativeInfoEvent(AGNA_GlobalEventInfo & accessor) const
{
	const AnxHeader & header = animation.header;
	if(accessor.eventIndex >= header.numEvents)
	{
		return false;
	}
	AnxEvent & evt = header.events.ptr[accessor.eventIndex];
	accessor.name = evt.name.ptr;
	accessor.frame = evt.frame;
	accessor.params = evt.params.ptr;
	accessor.numParams = evt.numParams;
	return true;
}

bool Animation::SetNativeInfoCurrentFrameGet(AGNA_GetCurrentFrame & accessor) const
{
#ifndef _XBOX
	AnxAnimation & ani = (AnxAnimation &)animation;
	if(ani.CurrentNode() != null)
	{
		((AGNA_GetCurrentFrame &)accessor).currentFrame = ani.GetCurrentFrame();
		return true;
	}
#endif
	return false;
}

bool Animation::SetNativeInfoCurrentFrameSet(AGNA_SetCurrentFrame & accessor) const
{
#ifndef _XBOX
#ifndef NO_TOOLS
	scene.Command_ClearQueue(this);
	AnxAnimation & ani = (AnxAnimation &)animation;
	if(ani.CurrentNode() != null)
	{
		ani.SetCurrentFrame(((AGNA_SetCurrentFrame &)accessor).currentFrame);
		return true;
	}
#endif
#endif
	return false;
}

bool Animation::SetNativeInfoNumberOfFrames(AGNA_GetNumberOfFrames & accessor) const
{
#ifndef _XBOX
#ifndef NO_TOOLS
	AnxAnimation & ani = (AnxAnimation &)animation;
	accessor.frames = ani.GetNumberOfFrames();
	return true;
#endif
#endif
	return false;
}

bool Animation::SetNativeInfoIsBoneUseGlobalPos(AGNA_IsBoneUseGlobalPos & accessor) const
{
#ifndef _XBOX
#ifndef NO_TOOLS
	AnxAnimation & ani = (AnxAnimation &)animation;
	return ani.IsBoneUseGlobalPos(accessor.index);
#endif
#endif
	return false;
}

bool Animation::SetNativeInfoSetFPS(AGNA_SetFPS & accessor) const
{
#ifndef _XBOX
#ifndef NO_TOOLS
	AnxAnimation & ani = (AnxAnimation &)animation;
	ani.SetFPS(accessor.fps);
	return true;
#endif
#endif
	return false;
}

bool Animation::SetNativeGotoNodeClip(AGNA_GotoNodeClip & accessor) const
{
#ifndef _XBOX
#ifndef NO_TOOLS
	scene.Command_ClearQueue(this);
	if(!accessor.node)
	{
		AnxAnimation & ani = (AnxAnimation &)animation;
		return ani.Start(null, 0);
	}
	const char * nameId = animation.GetConstData().GetHeader().FindName(accessor.node, AnxFndName::f_node);
	if(nameId)
	{
		service.Editor_NoThread();
		AnxAnimation & ani = (AnxAnimation &)animation;
		return ani.Goto(nameId, accessor.blendTime, accessor.clipIndex);
	}
#endif
#endif
	return false;
}

bool Animation::SetNativePause(AGNA_AnimationPause & accessor) const
{
#ifndef _XBOX
#ifndef NO_TOOLS
	scene.Command_ClearQueue(this);
	if(accessor.isAnimationPause)
	{
		((Animation *)this)->isPause |= pf_pause;
	}else{
		((Animation *)this)->isPause &= ~pf_pause;
	}
	scene.Command_Pause(((Animation *)this), accessor.isAnimationPause);
	return true;
#endif
#endif
	return false;
}

bool Animation::SetNativeBlockControl(AGNA_BlockControl & accessor) const
{
#ifndef _XBOX
#ifndef NO_TOOLS
	scene.Command_ClearQueue(this);
	((Animation *)this)->isBlockControl = (accessor.isBlockControl) ? 1 : 0;
	AnxAnimation & ani = (AnxAnimation &)animation;
	ani.BlockGraphLinks(accessor.isBlockControl);
	return true;
#endif
#endif
	return false;
}

__forceinline bool Animation::IsBlockControl() const
{
#ifndef _XBOX
#ifndef NO_TOOLS
	return isBlockControl != 0;
#endif
#endif
	return false;
}

