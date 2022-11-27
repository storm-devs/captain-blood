#ifndef _AnimationGraphNativeAccessor_h_
#define _AnimationGraphNativeAccessor_h_

//------------------------------------------------------------------------------------
// *  На время работы все передоваемые указатели должны быть валидны
// *  С возвращаемыми указателями можно пользоваться пока что загружена анимация
//
// Аксесоры:
//
//    AGNA_GlobalInfo   Получить общую информацию по графу
//    AGNA_NodeInfo     Получить информацию о ноде
//    AGNA_LinkInfo     Получить информацию о линке нода
//    AGNA_ClipInfo     Получить информацию об клипе
//    AGNA_EventInfo    Получить информацию об эвенте клипа
//
//------------------------------------------------------------------------------------


#include "Animation.h"

#include "templates\string.h"

enum AGNA_Types
{
	agna_unknown = 0,
	agna_global,
	agna_node_by_name,
	agna_node_by_index,
	agna_link,
	agna_clip,
	agna_event,
	agna_global_event,
	agna_get_frame,
	agna_set_frame,
	agna_get_number_of_frames,
	agna_is_bone_use_global_pos,
	agna_set_fps,
	agna_goto_nodeclip,
	agna_pause,
	agna_block_control,
};

//Получить общую информацию по графу
struct AGNA_GlobalInfo : public IAnimation::GraphNativeAccessor
{
	AGNA_GlobalInfo()
	{
		accessorType = agna_global;
		numNodes = 0;
		numLinks = 0;
		numConstatns = 0;
		numEvents = 0;
		numClips = 0;
	};

	dword numNodes;			//Количество нодов в графе
	dword numLinks;			//Общее количество линков в графе
	dword numConstatns;		//Общее количество констант в графе
	dword numEvents;		//Общее количество евентов в графе
	dword numClips;			//Общее количество клипов в графе
};

//Получить информацию о ноде
struct AGNA_NodeInfo : public IAnimation::GraphNativeAccessor
{
	AGNA_NodeInfo(const char * nodeName)
	{
		accessorType = agna_node_by_name;
		Reset();
		name = nodeName;
	};

	AGNA_NodeInfo(dword nodeIndex)
	{
		accessorType = agna_node_by_index;
		Reset();
		index = nodeIndex;
	};

	void Reset()
	{
		name = "";
		index = -1;
		numClips = 0;
		numConsts = 0;
		numLinks = 0;
		defLink = 0;
		isLoop = false;
		isChange = false;
	}

	const char * name;		//Имя нода
	dword index;			//Индекс нода
	dword numClips;			//Количество клипов в ноде
	dword numConsts;		//Количество констант
	dword numLinks;			//Количество линков
	long defLink;			//Линк по которому переходить в случае отсутствия активных ликтов
	bool isLoop;			//Зацикленно проигрывать действия
	bool isChange;			//Менять ли клип на следующем цикле
};

//Получить информацию о линке нода
struct AGNA_LinkInfo : public IAnimation::GraphNativeAccessor
{
	AGNA_LinkInfo(dword _nodeIndex, dword _linkIndex)
	{
		accessorType = agna_link;
		nodeIndex = _nodeIndex;
		linkIndex = _linkIndex;
		command = "";
		to = _nodeIndex;
		arange[0] = -1;
		arange[1] = -1;
		mrange[0] = -1;
		mrange[1] = -1;
		kBlendTime = 1.0f;
		syncPos = -1.0f;
	};
	
	dword nodeIndex;			//Индекс нода в графе
	dword linkIndex;			//Индекс линка в ноде
	const char * command;		//Команда линка (имя)
	dword to;					//На какой нод переходить (индекс нода)
	dword arange[2];			//диапазон активации	
	dword mrange[2];			//диапазона перехода
	float kBlendTime;			//Коэфициент время блендинга между нодами 1/t
	float syncPos;				//Относительная синхронизация нодов, -1 нету её
};

//Получить информацию об клипе
struct AGNA_ClipInfo : public IAnimation::GraphNativeAccessor
{
	AGNA_ClipInfo(dword _nodeIndex, dword _clipIndex)
	{
		accessorType = agna_clip;
		nodeIndex = _nodeIndex;
		clipIndex = _clipIndex;
		numEvents = 0;
		frames = 0;
		fps = 0.0f;
		probability = 0.0f;
	};

	dword nodeIndex;			//Индекс нода в графе
	dword clipIndex;			//Индекс клипа в ноде
	dword numEvents;			//Количество событий
	dword frames;				//Размер клипа в кадрах
	float fps;					//Скорость проигрывания анимации
	float probability;			//Нормализованная вероятность выбора клипа в ноде
};

//Получить информацию об эвенте клипа
struct AGNA_EventInfo : public IAnimation::GraphNativeAccessor
{
	AGNA_EventInfo(dword _nodeIndex, dword _clipIndex, dword _eventIndex)
	{
		accessorType = agna_event;
		nodeIndex = _nodeIndex;
		clipIndex = _clipIndex;
		eventIndex = _eventIndex;
		name = "";
		frame = 0;
		params = null;
		numParams = 0;
	};

	dword nodeIndex;			//Индекс нода в графе
	dword clipIndex;			//Индекс клипа в ноде
	dword eventIndex;			//Индекс эвента в клипе
	const char * name;			//Имя события
	dword frame;				//Кадр срабатывания события
	const char ** params;		//Параметры события
	dword numParams;			//Количество параметров
};

//Получить информацию об эвенте клипа
struct AGNA_GlobalEventInfo : public IAnimation::GraphNativeAccessor
{
	AGNA_GlobalEventInfo(dword _globalIndex)
	{
		accessorType = agna_global_event;
		eventIndex = _globalIndex;
		name = "";
		frame = 0;
		params = null;
		numParams = 0;
	};

	dword eventIndex;			//Индекс эвента в графе
	const char * name;			//Имя события
	dword frame;				//Кадр срабатывания события
	const char ** params;		//Параметры события
	dword numParams;			//Количество параметров
};

//Получить текущий проигрываемый кадр
struct AGNA_GetCurrentFrame : public IAnimation::GraphNativeAccessor
{
	AGNA_GetCurrentFrame()
	{
		accessorType = agna_get_frame;
		currentFrame = -1.0f;
	}

	float currentFrame;
};

//Установить текущий проигрываемый кадр
struct AGNA_SetCurrentFrame : public IAnimation::GraphNativeAccessor
{
	AGNA_SetCurrentFrame(float frame)
	{
		accessorType = agna_set_frame;
		currentFrame = frame;
	}

	float currentFrame;
};

//Получить текущий проигрываемый кадр
struct AGNA_GetNumberOfFrames : public IAnimation::GraphNativeAccessor
{
	AGNA_GetNumberOfFrames()
	{
		accessorType = agna_get_number_of_frames;
		frames = 0;
	}

	dword frames;
};

//Использует ли кость основного проигрывателя глобальные позиции в данный момент
struct AGNA_IsBoneUseGlobalPos : public IAnimation::GraphNativeAccessor
{
	AGNA_IsBoneUseGlobalPos(long boneIndex)
	{
		accessorType = agna_is_bone_use_global_pos;
		index = boneIndex;
	}

	long index;
};


struct AGNA_SetFPS : public IAnimation::GraphNativeAccessor
{
	AGNA_SetFPS(float newFPS)
	{
		accessorType = agna_set_fps;
		fps = newFPS;
	}

	float fps;
};

//Выполнить переход на нод с возможностью выбора клипа
struct AGNA_GotoNodeClip : public IAnimation::GraphNativeAccessor
{
	AGNA_GotoNodeClip(const char * _node, long _clipIndex, float _blendTime)
	{
		accessorType = agna_goto_nodeclip;
		node = _node;
		clipIndex = _clipIndex;
		blendTime = _blendTime;
	}

	const char * node;
	long clipIndex;
	float blendTime;
};

//Выполнить переход на нод с возможностью выбора клипа
struct AGNA_AnimationPause : public IAnimation::GraphNativeAccessor
{
	AGNA_AnimationPause(bool isPause)
	{
		accessorType = agna_pause;
		isAnimationPause = isPause;
	}

	bool isAnimationPause;
};


//Заблокировать стандартное управление анимацией и оставить долтуп только через аксессоры
struct AGNA_BlockControl : public IAnimation::GraphNativeAccessor
{
	AGNA_BlockControl(bool isBlock)
	{
		accessorType = agna_block_control;
		isBlockControl = isBlock;
	}

	bool isBlockControl;
};


#endif
