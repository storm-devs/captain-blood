//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxAnimation system
//============================================================================================
// AnxPlayer
//============================================================================================

#include "AnxAnimation.h"
#include "AnxPlayer.h"

//============================================================================================

AnxPlayer::AnxPlayer()
{
	ani = null;
	node = null;
	currFrame.clip = 0;
	currFrame.pos = 0;
	nextFrame.clip = 0;
	nextFrame.pos = 0;	
	fposition = 0.0f;
	link = null;
	kBlend = 0.0f;
	isSendEvent = true;
	isNextFrame = true;
	isActive = false;
	selSink = null;
	frameDltPos = curDltPos = oldDltPos = 0.0f;
	eventTime = 0.0f;
	cacheBones = null;
	cacheCounter = 1;
}

AnxPlayer::~AnxPlayer()
{

}

//Выделить память под кэшь
dword AnxPlayer::GetCacheBuffersSize(dword players, dword bonesCount)
{
	Assert(sizeof(Cache) == 64);
	dword size = players*bonesCount*sizeof(Cache) + 16;
	return size;
}

//Инициализировать плеер
void AnxPlayer::Init(AnxAnimation * _ani, byte * buffer, dword playerIndex, dword bonesCount)
{
	ani = _ani;
	cacheBones = (Cache *)(buffer + playerIndex*bonesCount*sizeof(Cache));
	Assert((((byte *)cacheBones - (byte *)0L) & 0xf) == 0);
	DirtyCache();
}

//============================================================================================

//Запустить плеер
void AnxPlayer::Start(AnxLink * _link, long _clip, float position)
{
	Assert(ani);
	if(_link->syncPos >= 0.0f)
	{
		position += _link->syncPos;
		position -= long(position);
	}else{
		position = -1.0f;
	}
	Start(_link->toNode.ptr, _clip, position);
	node = _link->toNode.ptr;
	link = _link;
	kBlend = 0.0f;	
}

//Запустить плеер
void AnxPlayer::Start(AnxNode * _node, long _clip, float position)
{
	Assert(ani);
	node = _node;
	link = null;
	kBlend = 1.0f;
	long iclip = (_clip >= 0 && (dword)_clip < _node->numClips) ? _clip : SelectClip(node);
	nextFrame.clip = &node->clips.ptr[iclip];
	Assert(nextFrame.clip);
	if(position >= 0.0f)
	{
		if(position > 1.0f)
		{
			position = 1.0f;
		}
		fposition = position*(nextFrame.clip->frames - 1);		
		nextFrame.pos = long(fposition);
		fposition -= nextFrame.pos;
	}else{
		nextFrame.pos = 0;
		fposition = 0.0f;
	}
	currFrame = nextFrame;
	eventTime = 0.0f;
	frameDltPos = curDltPos = oldDltPos = 0.0f;
	//Переход по линку
	selSink = null;
	isActive = false;
	isSendEvent = true;
	//Обнуляем кэшь
	DirtyCache();
	//Выделим следующий кадр
	NextFrame();
}

//Остоновить плеер
void AnxPlayer::Stop()
{
	node = null;
	link = null;
	selSink = null;
	isActive = false;
}

//Обновить состояние
bool AnxPlayer::Update(float dltTime)
{
	if(!node) return true;
	//Следующая позиция проигрывателя
	fposition += dltTime*currFrame.clip->fps;
	while(fposition >= 1.0f)
	{
		NextFrame();
		fposition -= 1.0f;
	}
	//Активация линка
	if(selSink)
	{
		if(!isActive)
		{
			if(!selSink->IsMoveRange())
			{
				isActive = true;
				isSendEvent = false;
			}else{
				if(currFrame.pos >= selSink->mrange[0] && currFrame.pos <= selSink->mrange[1])
				{
					isActive = true;
					isSendEvent = false;
				}
			}
		}
	}
	//Обновляем состояние коэфициента блендинга
	if(link)
	{
		if(link->kBlendTime > 0.0f)
		{
			kBlend += dltTime*link->kBlendTime;
			if(kBlend >= 1.0f)
			{
				kBlend = 1.0f;
				link = null;
			}
		}else{
			kBlend = 1.0f;
			link = null;
		}
	}
	return false;
}

//Применить анимацию текущего кадра
void AnxPlayer::Apply(Quaternion & q, Vector & p, long index, bool isBlend)
{
	Quaternion fq;
	Vector fp;
	//Vector fs;
	Assert(index >= 0 && index < (long)ani->numBones);
	Cache & cache = cacheBones[index];
	if(cache.needUpdate != cacheCounter)
	{
		//Превый кадр анимации
		bool isCurrGlobalPos = currFrame.clip->GetFrame(dword(index), currFrame.pos, cache.currQ, cache.currP);
		//Второй кадр анимации
		bool isNextGlobalPos = nextFrame.clip->GetFrame(dword(index), nextFrame.pos, cache.nextQ, cache.nextP);
		//Интерполяция между кадрами
		if(!(isCurrGlobalPos | isNextGlobalPos))
		{
			//Интеполируем между кадрами		
			fp.Lerp(cache.currP, cache.nextP, fposition);
			cache.globalPos = 0;
		}else{
			//Интерполируем позиции в глобальных системах
			if(!isCurrGlobalPos)
			{
				ConvertToGlobal(index, currFrame, cache.currP);
			}
			if(!isNextGlobalPos)
			{
				ConvertToGlobal(index, nextFrame, cache.nextP);
			}
			cache.globalPosFlt = 1.0f;
		}
		cache.needUpdate = cacheCounter;
	}
	fq.SLerp(cache.currQ, cache.nextQ, fposition);
	fp.Lerp(cache.currP, cache.nextP, fposition);
	if(cache.globalPos)
	{
		//Преобразуем позицию в локальную систему координат
		ConvertToLocal(index, fp);
	}
	//Учитываем влияние
	if(!isBlend || kBlend >= 1.0f)
	{
		q = fq;
		p = fp;
	}else{
		q.SLerp(q, fq, kBlend);
		p.Lerp(p, fp, kBlend);
	}
}

//Преобразовать позицию из локальной системы в глобальную
void AnxPlayer::ConvertToGlobal(long index, Frame & frame, Vector & position)
{
	Matrix mtx;
	while(true)
	{
		//Получаем матрицу родительской кости
		index = ani->GetBoneParentNoCheck(index);
		if(index < 0)
		{
			return;
		}
		//Получаем трансформацию текущей кости
		Quaternion curRot;
		Vector curPos;
		bool isGlobalPos = frame.clip->GetFrame(dword(index), frame.pos, curRot, curPos);
		curRot.GetMatrix(mtx);
		position = mtx.MulNormal(position);
		position += curPos;
		if(isGlobalPos)
		{
			return;
		}
	}
}

//Преобразовать позицию из глобальной системы коордитнат в локальную
void AnxPlayer::ConvertToLocal(long index, Vector & position)
{	
	Matrix current, parent;
	Quaternion curRot;
	Vector curPos;
	bool isEmpty = true;
	long parentIndex = index = ani->GetBoneParentNoCheck(index);
	while(index >= 0)
	{
		//Расчитываем матрицу для текущего уровня иерархии
		Apply(curRot, curPos, index, false);
		curRot.GetMatrix(parent);
		parent.pos = curPos;
		if(!isEmpty)
		{
			current = Matrix(current, parent);
		}else{
			isEmpty = false;
			current = parent;
		}
		index = ani->GetBoneParentNoCheck(index);
	}
	position = current.MulVertexByInverse(position);
}

//Применить анимацию текущего кадра
void AnxPlayer::Movement(Vector & deltaPos)
{
	Vector position = curDltPos + frameDltPos*fposition;
	deltaPos = position - oldDltPos;
	oldDltPos = position;
}

//Выбрать следующий кадр
inline void AnxPlayer::NextFrame()
{
	Assert(node);	
	//Разрешение отправки событий
	bool sendEvent = isSendEvent;
	//Кадры
	currFrame = nextFrame;
	nextFrame.pos++;
	//Система кэширования
	cacheCounter++;
	static const dword maxCacheCounter = 0x100000;
	if(cacheCounter > maxCacheCounter)
	{
		DirtyCache();
	}
	//Перемещение на кадре
	curDltPos += frameDltPos - oldDltPos; oldDltPos = 0.0f;
	currFrame.clip->GetMovement(currFrame.pos, frameDltPos);
	//Продолжаем клип если есть куда
	if(nextFrame.pos >= nextFrame.clip->frames)
	{
		//Клип закончился, надо решать что делать		
		if(node->flags & AnxNode::isLoop)
		{
			if(node->flags & AnxNode::isChange)
			{
				long iclip = SelectClip(node);
				Assert(iclip >= 0);
				nextFrame.clip = &node->clips.ptr[iclip];
				Assert(nextFrame.clip);
				nextFrame.pos = 0;
			}else{
				nextFrame.pos = 0;
			}		
		}else{
			isSendEvent = false;
			nextFrame.pos = currFrame.pos;
			if(node->numLinks > 0)
			{
				/*
				if(!selSink || (!sendEvent && !isActive))
				selSink = &node->links[node->defLink];
				*/
				if(!selSink)
				{
					selSink = &node->links.ptr[node->defLink];
				}
				isActive = true;
			}
			frameDltPos = 0.0f;
		}
		isNextFrame = true;
	}
	//Рассылаем события
	if(sendEvent && this == ani->mixer[0])
	{
		AnxEvent * events = currFrame.clip->events.ptr;
		dword num = currFrame.clip->numEvents;
		for(dword i = 0; i < num; i++)
		{
			if(events[i].frame == currFrame.pos)
			{
				//Добавляем событие на отсылку
				AnxEvent & evt = events[i];
				float realFPos = fposition;
				fposition = 0.0f;
				ani->AddEventToTransferBuffer(evt);
				fposition = realFPos;
			}
		}
	}	
}

//Испачкать кэшь
inline void AnxPlayer::DirtyCache()
{
	cacheCounter = 1;
	dword bonesCount = ani->data.GetHeader().numBones;
	for(dword i = 0; i < bonesCount; i++)
	{
		cacheBones[i].needUpdate = 0;
	}
}

//Выбрать клип в соответствии с вероятностями
long AnxPlayer::SelectClip(AnxNode * node)
{
	if(!node) return -1;
	if(node->numClips <= 1) return 0;
	float sel = Rnd();
	float p = 0.0f;
	for(dword i = 0; i < node->numClips; i++)
	{
		p += node->clips.ptr[i].probability;
		if(sel <= p) return i;
	}
	return rand() % node->numClips;
}

//Установить выбранный линк
bool AnxPlayer::SelectLink(AnxLink * link, bool forceApply, bool onlyCheck)
{
	if(selSink && !forceApply) return false;
	if(!onlyCheck)
	{
		selSink = link;
		isActive = false;
	}
	return true;
}

//Получить активированный линк
AnxLink * AnxPlayer::GetLink()
{
	if(isActive) return selSink;
	return null;
}

//Установить случайную позицию
void AnxPlayer::RandPosition()
{
	if(currFrame.clip && currFrame.clip->frames > 1)
	{
		nextFrame.pos = currFrame.pos = (long)Rnd(currFrame.clip->frames - 1.0f);
		nextFrame.clip = currFrame.clip;
		fposition = 0.0f;
	}else{
		nextFrame.pos = currFrame.pos = 0;
		nextFrame.clip = currFrame.clip;
		fposition = 0.0f;
	}
}

//Получить текущую отновительную позицию
float AnxPlayer::GetPosition()
{
	if(currFrame.clip && currFrame.clip->frames > 0)
	{
		float pos = currFrame.pos + fposition;
		if(currFrame.clip->frames > 1)
		{
			pos /= currFrame.clip->frames - 1;
		}
		return pos;
	}
	return 0.0f;
}

//Установить текущий кадр
void AnxPlayer::SetCurrentFrame(float frame)
{
	if(!currFrame.clip) return;
	if(frame < 0.0f) frame = 0.0f;
	if(frame > currFrame.clip->frames - 1) frame = currFrame.clip->frames - 1.0f;
	currFrame.pos = long(frame);
	nextFrame = currFrame;
	selSink = null;
	isActive = false;
	isSendEvent = true;
	NextFrame();
	fposition = frame - currFrame.pos;
	frameDltPos = curDltPos = oldDltPos = 0.0f;
	Update(0.0f);
}

//Получить текущий кадр
float AnxPlayer::GetCurrentFrame()
{ 
	return currFrame.pos + fposition;
}

