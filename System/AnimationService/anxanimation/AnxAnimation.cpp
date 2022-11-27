//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// AnxAnimation system
//============================================================================================
// AnxAnimation
//============================================================================================

#include "AnxAnimation.h"
#include "AnxData.h"
#include "..\..\..\common_h\Animation.h"

#ifndef _XBOX
#include <xmmintrin.h>
#endif

//============================================================================================

AnxAnimation::AnxAnimation(AnxData & _data) : data(_data), 
												header(_data.GetHeader()),
												events(_FL_),
												extMixer(_FL_)
{
	events.Reserve(16);
	Assert(header.numBones > 0);
	numBones = header.numBones;
	dword globalBufferSize = numBones*(sizeof(Bone) + sizeof(Matrix) + 0x10);
	globalBufferSize += AnxPlayer::GetCacheBuffersSize(ARRSIZE(players), numBones);
	buffer = NEW byte[globalBufferSize];
	bones = (Bone *)AlignPtr(buffer);
	AssertAling16(bones);
	Assert((sizeof(Bone) & 0xf) == 0);
	matrices = (Matrix *)(bones + numBones);
	for(dword i = 0; i < numBones; i++)
	{
		Bone & b = bones[i];		
		b.aniq.SetIdentity();
		b.anip = Vector4(0.0f);
		b.resq.SetIdentity();
		b.resp = Vector4(0.0f);
		b.ress = Vector4(1.0f);
		b.update = 0;
		b.parent = header.bones.ptr[i].parent;
		b.desk = &header.bones.ptr[i];
		b.index = i;
		matrices[i].SetIdentity();
	}
	Assert(ARRSIZE(players) == ARRSIZE(mixer));
	byte * playersBuffer = (byte *)(matrices + numBones);
	for(i = 0; i < ARRSIZE(players); i++)
	{
		players[i].Init(this, playersBuffer, i, numBones);
		mixer[i] = null;
	}
	memset(&gotoLink, 0, sizeof(gotoLink));
	gotoLink.name.ptr = "goto system llnk";
	gotoLink.toNode.ptr = null;
	gotoLink.kBlendTime = 1.0f;
	gotoLink.syncPos = -1.0f;
	mixers = 0;
	updateCount = 1;
	timeEvent = null;
	timeEventPointer = null;
	timeEventStep = 0.0f;
	event = null;
	eventPointer = null;
	isAllBonesUpdated = false;
	isBlockGraphLinks = false;
	Start();
	GetBoneMatrices();
}

AnxAnimation::~AnxAnimation()
{
	delete buffer;
}


//============================================================================================

//Получить имя анимации
const char * AnxAnimation::GetName()
{
	return data.GetName();
}

//Начать проигрывание анимации с заданного нода
bool AnxAnimation::Start(const char * nodeNameId, long clipIndex)
{
	//Очистим буффер сообщений
	events.Empty();
	//Сбрасываем миксер
	for(dword i = 0; i < ARRSIZE(mixer); i++) mixer[i] = null;
	for(i = 0; i < ARRSIZE(players); i++) players[i].Stop();
	//Сбрасываем текущее состояние анимации
	for(i = 0; i < numBones; i++)
	{
		Bone & bone = bones[i];
		bone.aniq.SetIdentity();
		bone.anip = Vector4(0.0f);
		bone.resq.SetIdentity();
		bone.resp = Vector4(0.0f);
		bone.ress = Vector4(1.0f);
		bone.update = 0;
	}
	mixers = 0;
	updateCount = 1;
	isAllBonesUpdated = false;
	//Стартовый нод
	AnxNode * node = nodeNameId ?  FindNodeById(nodeNameId) : header.startNode.ptr;
	if(!node)
	{
		if(!nodeNameId) nodeNameId = "<null>";
		api->Trace("Animation not started, node \"%s\" not found, start default node", nodeNameId);
		Start(null);
		return false;
	}
	gotoLink.toNode.ptr = null;
	mixer[0] = &players[0];
	mixers = 1;
	mixer[0]->Start(node, clipIndex);
	//if(node->isLoop && !nodeName) mixer[0]->RandPosition();	
	return true;
}

//Перейти на заданный нод с текущего
bool AnxAnimation::Goto(const char * nodeNameId, float blendTime, long clipIndex)
{
	//Если нет миксеров, то стартуем новый нод
	if(!mixers || blendTime < 0.005f)
	{
		return Start(nodeNameId, clipIndex);
	}
	//Настраиваем активируемый линк
	AnxNode * node = FindNodeById(nodeNameId);
	if(!node) return false;
	gotoLink.toNode.ptr = node;	
	gotoLink.kBlendTime = 1.0f/blendTime;
	gotoLinkSelectClip = clipIndex;
	return true;
}

//Активировать линк
bool AnxAnimation::ActivateLink(const char * commandId, bool forceApply, bool onlyCheck)
{
	if(!mixer[0]) return false;
	//Ищим линк с нужным именем
	AnxNode * node = mixer[0]->Node();
	Assert(node);
	for(dword i = 0; i < node->numLinks; i++)
	{
		AnxLink & link = node->links.ptr[i];
		//if(string::IsEqual(link.name.ptr, command))
		if(link.name.ptr == commandId)
		{
			//Проверяем время активации
			if(!link.IsActivateRange())
			{
				bool res = mixer[0]->SelectLink(&link, forceApply, onlyCheck);
				if(forceApply)
				{
					Update(0.0f);
				}
				return res;
			}else{
				dword pos = mixer[0]->CurrentFrame().pos;
				if(pos >= link.arange[0] && pos <= link.arange[1])
				{
					bool res = mixer[0]->SelectLink(&link, forceApply, onlyCheck);
					if(forceApply)
					{
						Update(0.0f);
					}
					return res;
				}
			}
		}
	}
	return false;
}

//Выбрать случайную позицию проигрывания
void AnxAnimation::RandomizePosition()
{
	if(mixers)
	{
		mixer[0]->RandPosition();
	}
}

//Получить имя текущего нода
const char * AnxAnimation::CurrentNode()
{
	return mixer[0] ? mixer[0]->Node()->name.ptr : null;
}
//Доступиться до константы типа string
const char * AnxAnimation::GetConstString(const char * constName, const char * nodeName)
{
	AnxNode * node = mixer[0] ? mixer[0]->Node() : null;
	if(nodeName) node = FindNode(nodeName);
	if(!node) return null;
	AnxConst * c = node->FindConst(constName);
	if(!c || c->type != AnxConst::ct_string) return null;
	return c->svalue.ptr;
}

//Доступиться до константы типа float
float AnxAnimation::GetConstFloat(const char * constName, const char * nodeName)
{
	AnxNode * node = mixer[0] ? mixer[0]->Node() : null;
	if(nodeName) node = FindNode(nodeName);
	if(!node) return 0.0f;
	AnxConst * c = node->FindConst(constName);
	if(c && c->type == AnxConst::ct_float)
	{
		return c->fvalue;
	}
	return 0.0f;
}

//Доступиться до константы типа blend
float AnxAnimation::GetConstBlend(const char * constName)
{
	float value = 0.0f;
	for(long i = mixers - 1; i >= 0; i--)
	{
		AnxConst * c = mixer[i]->Node()->FindConst(constName);
		if(c && c->type == AnxConst::ct_blend)
		{
			value += (c->fvalue - value)*mixer[i]->GetKBlend();
		}else{
			value += (0.0f - value)*mixer[i]->GetKBlend();
		}
	}
	return value;
}


//============================================================================================

//Получить количество костей в анимации
long AnxAnimation::GetNumBones()
{
	return numBones;
}

//Получить имя кости
const char * AnxAnimation::GetBoneName(long index)
{
	Assert(index >= 0 && index < long(numBones));
	return header.bones.ptr[index].name.ptr;
}

//Получить индекс родительской кости
long AnxAnimation::GetBoneParent(long index)
{
	Assert(index >= 0 && index < long(numBones));
	return bones[index].parent;
}

//Найти по имени кость
long AnxAnimation::FindBone(const char * boneName, bool shortName)
{
	if(!boneName || !boneName[0]) return -1;
	dword hash = string::HashNoCase(boneName);
	if(shortName)
	{		
		return header.FindBoneShort(boneName, hash);
	}	
	return header.FindBone(boneName, hash);
}

//Найти по имени кость (короткое имя)
long AnxAnimation::FindBoneUseHash(const char * boneName, unsigned long hash)
{
	//return header.FindBone(boneName, hash);
	return header.FindBoneShort(boneName, hash);
}

//Получить текущее в анимации сстояние кости
void AnxAnimation::GetAnimationBone(long index, Quaternion & q, Vector & p)
{
	Assert(index >= 0 && index < long(numBones));
	q = bones[index].aniq;
	p = bones[index].anip.v;
}

//Получить ориентацию кости с учётом блендеров, без иерархии
const Quaternion & AnxAnimation::GetBoneRotate(long index)
{
	Assert(index >= 0 && index < long(numBones));
	return bones[index].resq;
}

//Получить позицию кости с учётом блендеров, без иерархии
const Vector & AnxAnimation::GetBonePosition(long index)
{
	Assert(index >= 0 && index < long(numBones));
	return bones[index].resp.v;
}

//Получить масштаб кости с учётом блендеров, без иерархии
const Vector & AnxAnimation::GetBoneScale(long index)
{
	Assert(index >= 0 && index < long(numBones));
	return bones[index].ress.v;
}

//Получить матрицу кости
const Matrix & AnxAnimation::GetBoneMatrix(long index)
{
	Assert(index >= 0 && index < long(numBones));
	//Обновление состояния кости
	if(bones[index].update != updateCount)
	{
		//Пересчитываем кости
		for(long i = index; i >= 0; i = bones[i].parent)
		{
			UpdateBone(i);
		}
		//Расчитываем иерархию
		HeirarchyBoneUpdate(index);
	}
	return matrices[index];
}

//Получить массив матриц
const Matrix * AnxAnimation::GetBoneMatrices()
{
	//Если всё посчитано, то просто возвращаем указатель
	if(isAllBonesUpdated)
	{
//		api->Trace("AnxAnimation::GetBoneMatrices(%.8x) -> skip calculations", this);
		return matrices;
	}
//	api->Trace("AnxAnimation::GetBoneMatrices(%.8x) -> update calculations", this);


	//Обновление состояния всех костей
	for(dword i = 0; i < numBones; i++)
	{
		if(bones[i].update != updateCount)
		{
			//Пересчитываем кость
			UpdateBone(i);
			//Учтём родителя
			if(bones[i].parent >= 0)
			{
				Assert(long(i) > bones[i].parent);
				matrices[i] = Matrix(matrices[i], matrices[bones[i].parent]);
			}
		}
	}
	isAllBonesUpdated = true;
	return matrices;
}


//Получить текущую дельту смещения
void AnxAnimation::GetMovement(Vector & deltaPos)
{
	if(mixer[0])
	{
		mixer[0]->Movement(deltaPos);
	}else{
		deltaPos = 0.0f;
	}
}

//Являеться ли текущий нод стоповым
bool AnxAnimation::CurrentNodeIsStop()
{
	if(mixer[0])
	{
		return mixer[0]->CurrentNodeIsStop();
	}
	return true;
}

//Получить количество эвентов в анимации
long AnxAnimation::GetNumEvents()
{
	return header.numEvents;
}

//Получить имя и параметры события
const char * AnxAnimation::GetEvent(long index, const char ** & params, dword & numParams)
{
	if(index < 0 || index >= (long)header.numEvents)
	{
		params = null;
		numParams = 0;
		return null;
	}
	params = header.events.ptr[index].params.ptr;
	numParams = header.events.ptr[index].numParams;
	return header.events.ptr[index].name.ptr;
}


//Расчитать новую позицию
void AnxAnimation::Update(float dltTime)
{
	if(!timeEvent)
	{
		UpdateStep(dltTime);
	}else{
		while(true)
		{
			float step = timeEventStep;
			if(dltTime <= timeEventStep)
			{
				UpdateStep(dltTime);
				break;
			}else{
				if(timeEvent)
				{
					UpdateStep(timeEventStep);
					dltTime -= timeEventStep;
				}else{
					UpdateStep(dltTime);
					break;
				}
			}
		}
	}
}

//Расчитать новую позицию
void AnxAnimation::UpdateStep(float dltTime)
{
	//Процессируем плееры
	for(dword i = 0; i < mixers; i++)
	{
		if(mixer[i]->Update(dltTime))
		{
			mixer[i]->Stop();
			mixer[i] = null;
			mixers--;
			for(dword j = i; j < mixers; j++)
			{
				mixer[j] = mixer[j + 1];
			}
			i--;
		}
	}
	if(mixers > 0)
	{
		AnxLink * link = mixer[0]->GetLink();
		if(isBlockGraphLinks)
		{
			link = null;
		}
		long selClip = -1;
		if(gotoLink.toNode.ptr)
		{
			link = &gotoLink;
			selClip = gotoLinkSelectClip;
		}
		if(link)
		{
			//Останавливаем последний плеер
			if(mixers == ARRSIZE(mixer))
			{
				mixer[mixers - 1]->Stop();
				mixer[mixers - 1] = null;
				mixers--;
			}
			//Смещаем элементы миксерного буфера
			for(dword i = mixers; i > 0; i--)
			{
				mixer[i] = mixer[i - 1];
			}
			//Добавляем новый плеер
			for(i = 0; i < ARRSIZE(players); i++)
			{
				if(players[i].IsFree()) break;
			}
			Assert(i < ARRSIZE(players));
			mixer[0] = &players[i];
			mixer[0]->Start(link, selClip, mixer[1] ? mixer[1]->GetPosition() : 0.0f);
			mixers++;
		}
		if(gotoLink.toNode.ptr)
		{
			gotoLink.toNode.ptr = null;
		}
	}
	isAllBonesUpdated = false;
	updateCount++;
	if(updateCount > 0x00ffffff)
	{
		updateCount = 1;
		for(dword i = 0; i < numBones; i++)
		{
			bones[i].update = 0;
		}
	}
	if(timeEvent)
	{
		timeEvent(timeEventPointer, this);
	}
}

//Установить обработчик эвентов
void AnxAnimation::SetAnimationExecuteEvent(AnimationExecuteEvent event, void * pointer)
{
	this->event = event;
	eventPointer = pointer;
}

//Установить временной обработчик
void AnxAnimation::SetTimeEvent(AnimationTimeEvent event, void * pointer, float timeStep)
{
	timeEvent = event;
	timeEventPointer = pointer;
	if(timeStep <= 1e-4f)
	{
		timeStep = 0.0f;
		timeEvent = null;
	}
	timeEventStep = timeStep;
	
}

//Добавить события в буфер ожидания отправки
void AnxAnimation::AddEventToTransferBuffer(AnxEvent & evt)
{	
	if(!(evt.flags & AnxEvent::flg_isNoBuffered) || !event)
	{
		events.Add(&evt);
	}else{
		event(eventPointer, this, evt.name.ptr, evt.params.ptr, evt.numParams);
	}
}

//Найти нод
AnxNode * AnxAnimation::FindNode(const char * nodeName)
{
	if(!nodeName) return null;
	long index = header.FindNode(nodeName);
	if(index >= 0)
	{
		return &header.nodes.ptr[index];
	}
	return null;
}

//Найти нод по идентификатору
AnxNode * AnxAnimation::FindNodeById(const char * nodeNameId)
{
	for(dword i = 0; i < header.numNodes; i++)
	{
		AnxNode & node = header.nodes.ptr[i];
		if(node.name.ptr == nodeNameId)
		{
			return &node;
		}
	}
	return null;
}

//Обновить состояние кости без учета иерархии
inline void AnxAnimation::UpdateBone(long index)
{
#ifndef STOP_PROFILES
//	data.manager.bonesCounter++;
#endif
	Quaternion stageRot;
	Vector stagePos;
	Vector stageScale;
	Quaternion & curRot = bones[index].resq;
	Vector & curPos = bones[index].resp.v;
	Vector & curScale = bones[index].ress.v;
	//Проверим возможность сокращения цепочки
	for(long i = extMixer - 1; i >= 0; i--)
	{
		if(!extMixer[i]->NeedPrevStageData(index))
		{
			if(extMixer[i]->GetBoneBlend(index) >= 1.0f)
			{
				break;			
			}
		}
	}
	//Собираем по цепочке преобразования
	Bone & bone = bones[index];
	if(i < 0)
	{
		i = 0;
		bone.aniq.SetIdentity();
		bone.anip = 0.0f;
		//Проходимся по плеерам
		for(long i = mixers - 1; i >= 0; i--)
		{
			mixer[i]->Apply(bone.aniq, bone.anip.v, index, i != long(mixers) - 1);
		}
		curRot = bone.aniq;
		curPos = bone.anip.v;
		curScale = 1.0f;
	}else{
		extMixer[i]->GetBoneTransform(index, Quaternion(), Vector(0.0f), Vector(1.0f), curRot, curPos, curScale);
	}
	//Проходим по внешним миксерам
	for(; i < extMixer; i++)
	{
		AnxBlendStage * bs = extMixer[i];
		float kBlend = bs->GetBoneBlend(index);
		if(kBlend <= 0.0f) continue;
		bs->GetBoneTransform(index, stageRot, stagePos, stageScale, curRot, curPos, curScale);
		curRot.SLerp(curRot, stageRot, kBlend);
		curPos.Lerp(curPos, stagePos, kBlend);
		curScale.Lerp(curScale, stageScale, kBlend);
	}
	//Начальная инициализация кости
	bone.update = updateCount;
	Matrix & mtx = matrices[index];

//	Matrix mtx;
	
//	curRot.GetMatrix(mtx);
//	mtx.pos = curPos;
//	mtx.Scale(curScale);
	
	//Развёрнутый вариант для скорости
	float & x = curRot.x;
	float & y = curRot.y;
	float & z = curRot.z;
	float & w = curRot.w;
	float kLen = x*x + y*y + z*z + w*w;
	if(kLen > 1e-20f)
	{
		kLen = 2.0f/kLen;
		float xx = x*x*kLen, xy = x*y*kLen, xz = x*z*kLen;
		float yy = y*y*kLen, yz = y*z*kLen;
		float zz = z*z*kLen;
		float wx = w*x*kLen, wy = w*y*kLen, wz = w*z*kLen;
		mtx.m[0][0] = (1.0f - (yy + zz))*curScale.x;
		mtx.m[0][1] = (xy + wz)*curScale.y;
		mtx.m[0][2] = (xz - wy)*curScale.z;
		mtx.m[0][3] = 0.0f;
		mtx.m[1][0] = (xy - wz)*curScale.x;
		mtx.m[1][1] = (1.0f - (xx + zz))*curScale.y;
		mtx.m[1][2] = (yz + wx)*curScale.z;
		mtx.m[1][3] = 0.0f;
		mtx.m[2][0] = (xz + wy)*curScale.x;
		mtx.m[2][1] = (yz - wx)*curScale.y;
		mtx.m[2][2] = (1.0f - (xx + yy))*curScale.z;
		mtx.m[2][3] = 0.0f;
		mtx.m[3][0] = curPos.x*curScale.x;
		mtx.m[3][1] = curPos.y*curScale.y;
		mtx.m[3][2] = curPos.z*curScale.z;
		mtx.m[3][3] = 1.0f;
	}else{
		mtx.SetZero();
		mtx.pos = curPos*curScale;
	}	
}

//Просчитать иерархию для кости
void AnxAnimation::HeirarchyBoneUpdate(long index)
{
	//Обновим родителя
	if(bones[index].parent >= 0)
	{
		HeirarchyBoneUpdate(bones[index].parent);
//		matrices[index] = Matrix(matrices[index], matrices[bones[index].parent]);
		Matrix::MultiplySIMD_4x3(matrices[index].matrix, matrices[index].matrix, matrices[bones[index].parent].matrix);
	}
}

//Проверить список миксеров
inline void AnxAnimation::CheckMixers()
{
	for(dword i = 0; i < mixers; i++)
	{
		Assert(mixer[i]);
		Assert(mixer[i]->Node());
	}
}

//Получить количество кадров текущего клипа основного проигрывателя
dword AnxAnimation::GetNumberOfFrames()
{
	if(!mixers) return 0;
	Assert(mixer[0]);
	if(!mixer[0]->CurrentFrame().clip) return 0;
	return mixer[0]->CurrentFrame().clip->frames;
}

//Использует ли кость основного проигрывателя глобальные позиции в данный момент
bool AnxAnimation::IsBoneUseGlobalPos(long boneIndex)
{
	if(!mixers) return false;
	Assert(mixer[0]);
	if(!mixer[0]->CurrentFrame().clip) return 0;
	return mixer[0]->CurrentFrame().clip->GetFrame(boneIndex, mixer[0]->CurrentFrame().pos, Quaternion(), Vector());
}

//Установить скорость проигрывания
void AnxAnimation::SetFPS(float fps)
{
	if(mixers)
	{
		Assert(mixer[0]);
		mixer[0]->SetFPS(fps);
	}
}

//Заблокировать переход по линкам графа
void AnxAnimation::BlockGraphLinks(bool isBlock)
{
	isBlockGraphLinks = isBlock;
}

