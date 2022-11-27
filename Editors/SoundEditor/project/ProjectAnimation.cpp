

#include "ProjectAnimation.h"
#include "..\SndOptions.h"
#include "..\..\..\common_h\AnimationNativeAccess.h"


dword ProjectAnimation::magicStage[movie_stage_last + 1] = {88912238, 3781213, 942416, 12095678, 78224951, 9212313};
const char * ProjectAnimation::fileGlobalId = "glb_";
const char * ProjectAnimation::fileNodeId = "nod_";
const char * ProjectAnimation::fileMovieId = "mov_";
const char * ProjectAnimation::fileEventId = "evt_";

ProjectAnimation::PrjAnxNode::PrjAnxNode() : events(_FL_)
{
}

ProjectAnimation::PrjAnxNode::~PrjAnxNode()
{
}

ProjectAnimation::Movie::Movie(const char * movieName) : nodes(_FL_)
{
	memset(nameData, 0, c_nameLen);
	crt_strncpy(nameData, c_nameLen, movieName, c_nameLen - 1);
	name.Init(nameData);
	memset(misName, 0, c_nameLen);
	stage = movie_stage_movie;
}

ProjectAnimation::Movie::~Movie()
{
}


ProjectAnimation::ProjectAnimation(const UniqId & folderId) : ProjectObject(folderId, c_nameLen, ".txt", true),
										movies(_FL_),
										nodes(_FL_),
										nodeNames(_FL_, 256)
{
	Assert(sizeof(UniqId) == c_uid_size);
	memset(anxName, 0, sizeof(anxName));
	memset(anxPath, 0, sizeof(anxPath));
	ani = null;
	flags = flags_init;
	validateCounter = 1;
	scene = null;
	memset(locatorName, 0, sizeof(locatorName));
}

ProjectAnimation::~ProjectAnimation()
{
	ReleaseAnimation();
	for(dword i = 0; i < movies.Size(); i++)
	{
		delete movies[i];
	}
	movies.Empty();
}

ErrorId ProjectAnimation::SetAnimation(const char * anxFileName)
{
	//Проверяем имя
	dword nameLen = string::Len(anxFileName);
	Assert(nameLen > 0 && nameLen < c_nameLen);
	//Ищим анимационный файл по всем папкам
	string anxFileNameWithExt = anxFileName;
	anxFileNameWithExt.AddExtention(".anx");
	IFinder * finder = options->fileService->CreateFinder("Resource", anxFileNameWithExt.c_str(), find_no_mirror_files | find_no_files_from_packs, _FL_);
	Assert(finder);
	if(finder->Count() == 0)
	{
		finder->Release();
		return options->ErrorOut(null, true, "%s%s", options->GetString(SndOptions::s_ani_er_file_not_found), anxFileNameWithExt.c_str());
	}
	if(finder->Count() != 1)
	{
		finder->Release();
		return options->ErrorOut(null, true, "%s%s", options->GetString(SndOptions::s_ani_er_files_to_many), anxFileNameWithExt.c_str());
	}
	//Получаем относительный путь	
	string basePath;
	options->fileService->BuildPath("", basePath);
	string path = finder->Path(0);
	path.GetRelativePath(basePath);
	Assert(path.Len() < c_pathLen);
	crt_strncpy(anxPath, c_pathLen, path.c_str(), c_pathLen - 1);
	const char * title = finder->Title(0);
	Assert(string::Len(title) < c_nameLen);
	crt_strncpy(anxName, c_nameLen, title, c_nameLen - 1);
	finder->Release();
	finder = null;
	//Чтение anx файла
	LoadAnimation();
	if(!ani)
	{
		return options->ErrorOut(null, true, "%s%s", options->GetString(SndOptions::s_ani_er_cant_open), anxName);
	}
	//Разбираем его структуру и копируем к себе данные
	AGNA_GlobalInfo globalInfo;
	bool res = ani->GetNativeGraphInfo(globalInfo);
	Assert(res);
	Assert(nodes.Size() == 0);
	for(dword i = 0; i < globalInfo.numNodes; i++)
	{
		//Информация о ноде
		AGNA_NodeInfo nodeInfo(i);
		res = ani->GetNativeGraphInfo(nodeInfo);
		Assert(res);
		//Сохраняем имя
		dword len = string::Len(nodeInfo.name) + 1;
		long nameOffset = nodeNames.Size();
		nodeNames.AddElements(len);
		memcpy(&nodeNames[nameOffset], nodeInfo.name, len);
		//На каждый клип добавляем по записи
		for(dword j = 0; j < nodeInfo.numClips; j++)
		{
			PrjAnxNode & node = nodes[nodes.Add()];
			node.nameOffset = nameOffset;
			node.nameLen = len;
			node.clipIndex = j;			
			node.clipsCount = nodeInfo.numClips;
			node.isValidate = true;
			node.nodeId.Build();
			//Перебираем все клипы
			for(dword k = 0; k < globalInfo.numClips; k++)
			{
				//Получаем описание клипа
				AGNA_ClipInfo clipInfo(i, k);
				res = ani->GetNativeGraphInfo(clipInfo);
				Assert(res);
				//Смотрим принадлежность
				if(clipInfo.nodeIndex == nodeInfo.index)
				{
					if(clipInfo.clipIndex == j)
					{
						//Сохраняем найденые параметры
						node.clipFrames = clipInfo.frames;
						node.fps = clipInfo.fps;
						break;
					}
				}
			}
			Assert(k < globalInfo.numClips);
		}
	}
	SaveChanges();
	return ErrorId::ok;
}


//Сохранить изменения
void ProjectAnimation::SaveChanges()
{
	SetToSave();
}

//Получить имя анимации
const char * ProjectAnimation::GetAnimationName()
{
	return anxName;
}

//Получить путь до анимации
const char * ProjectAnimation::GetAnimationPath()
{
	return anxPath;
}

//Проэкспортировать события в ини файл и сохранить путь экспотра для информирования
bool ProjectAnimation::ExportEventsToIni(string & exportPath, bool ignoreStages)
{
	//Если анимация ещё не обновленна, то сделаем это сейчас
	CheckAnimation();
	//Путь 
	exportPath = options->pathExportEvents;
	exportPath += GetName().str;
	exportPath += ".ini";
	//Буфер
	string data("", 4096);
	//Заголовок	
	data += "\r\n\r\n;******************************************************\r\n;Animation events exported from sound editor\r\n;******************************************************\r\n;Export time: ";
	__time64_t ltime;
	_time64(&ltime);
	struct tm today;
	crt_localtime64(&today, &ltime);
	char tmpbuf[128];
	strftime(tmpbuf, sizeof(tmpbuf), "%d %B %Y, %H:%M:%S", &today);
	data += tmpbuf;
	data += "\r\n;******************************************************\r\n";
	//Число определяющие в каком режиме выгрузили файл
	long magic = rand() & 0x00001fff;
	magic |= (rand() & 0xffff) << 18;
	magic ^= GetTickCount() << 10;
	magic &= 0x0ffffc00;
	magic |= GetName().hash & 0x3ff;
	if(ignoreStages)
	{
		magic |= (1 << 13);
		data += ";Preview mode. Not all events have done stage!!!\r\n;******************************************************\r\n";
	}else{
		magic &= ~(1 << 13);
	}
	data += "\r\n\r\nflags = ";
	data += magic;
	data += "\r\n\r\n";
	//Проходим по всем нодам, собирая эвенты
	dword eventsCount = 1;
	for(dword i = 0; i < nodes.Size(); i++)
	{
		PrjAnxNode & node = nodes[i];
		const char * nodeName = &nodeNames[node.nameOffset];
		if(!node.isValidate)
		{
			continue;
		}
		for(dword j = 0; j < node.events.Size(); j++)
		{
			//Эвенты непроапрувленых роликов пропускаем
			Event & evt = node.events[j];			
			if(!evt.movie) continue;
			if(!ignoreStages && evt.movie->stage != movie_stage_done)
			{
				continue;
			}
			//Добавляем секцию с описанием эвента
			data += "\r\n[event_";
			data += eventsCount++;
			data += "]\r\nnode = ";
			data += nodeName;
			data += "\r\nclip = ";
			data += node.clipIndex;
			data += "\r\nframe = ";
			data += evt.frame;
			data += "\r\nsound = ";
			data += evt.sound;
			if(evt.eventParams & event_flags_local)
			{				
				const char * locator = evt.locator;
				if(locator[0] == '#')
				{
					data += "\r\nobject = ";
					for(dword c = 1; c < c_nameLen - 1; c++)
					{
						if(locator[c] == ':')
						{
							locator = &locator[c + 1];
							break;
						}						
						data += locator[c];
					}
				}
				data += "\r\nlocator = ";
				data += locator;
			}
			if(evt.eventParams & event_flags_bind)
			{
				data += "\r\nbind = 1";
				data += "\r\nfadeout = ";
				data += evt.fadeTime;
			}			
			if(evt.eventParams & event_flags_voice)
			{
				data += "\r\nvoice = 1";
			}
			data += "\r\n\r\n";
		}
	}
	data += "\r\n\r\n";
	bool isOk = options->fileService->SaveData(exportPath.c_str(), data.GetBuffer(), data.Len());
	return isOk;
}

//Загрузить анимацию
IAnimation * ProjectAnimation::LoadAnimation()
{
	if(ani)
	{
		return ani;
	}
	IMirrorPath * path = options->fileService->CreateMirrorPath(anxPath, "Resource\\Animation", _FL_);
	ani = options->aniScene->Create(anxName, _FL_);
	if(path)
	{
		path->Release();
		path = null;
	}	
	return ani;
}

//Выгрузить анимацию
void ProjectAnimation::ReleaseAnimation()
{
	if(ani)
	{
		ani->Release();
		ani = null;
	}
}

//Проверить анимацию на соответствие с текущим состоянием
void ProjectAnimation::CheckAnimation()
{
	//Сообщим что возможно было утеряно значение
	validateCounter++;
	//Проверяем состояние
	if((flags & flags_need_animation_check) == 0)
	{
		return;
	}
	flags &= ~flags_need_animation_check;
	//Загружаем анимацию
	LoadAnimation();
	//Сверяем структуру анимационного файла с текущим состоянием
	bool isChange = false;
	if(ani)
	{
		AGNA_GlobalInfo globalInfo;
		bool res = ani->GetNativeGraphInfo(globalInfo);
		Assert(res);
		dword nodesCountForProcess = nodes.Size();
		for(dword i = 0; i < globalInfo.numNodes; i++)
		{
			//Информация о ноде
			AGNA_NodeInfo nodeInfo(i);
			res = ani->GetNativeGraphInfo(nodeInfo);
			Assert(res);
			dword len = string::Len(nodeInfo.name) + 1;
			options->collection.Empty();
			//Ищим ноды совпадающие по имени, составляя список
			long nameOffset = -1;
			long nameLen = 0;
			for(dword j = 0; j < nodesCountForProcess; j++)
			{
				PrjAnxNode & node = nodes[j];
				if(node.nameLen == len)
				{
					const char * nodeName = &nodeNames[node.nameOffset];
					if(string::IsEqual(nodeInfo.name, nodeName))
					{
						if(nameOffset < 0)
						{
							nameOffset = node.nameOffset;
							nameLen = node.nameLen;
						}else{
							//Проверяем, что имена не дублируються
							//Assert(nameOffset == node.nameOffset); При загрузке из файла имена не сливаються
							Assert(nameLen == node.nameLen);
						}
						SndOptions::Collection c;
						c.ptr = null;
						c.index = j;
						options->collection.Add(c);
					}
				}
			}
			if(nameOffset < 0)
			{
				//Новый анимационный нод
				Assert(options->collection.Size() == 0);
				//Сохраняем имя
				nameLen = string::Len(nodeInfo.name) + 1;
				nameOffset = nodeNames.Size();
				nodeNames.AddElements(nameLen);
				memcpy(&nodeNames[nameOffset], nodeInfo.name, nameLen);
			}
			//Перебираем по количеству нодов клипы сверяя с нодами проекта
			for(dword k = 0; k < nodeInfo.numClips; k++)
			{				
				//Получаем описание клипа
				AGNA_ClipInfo clipInfo(i, k);
				res = ani->GetNativeGraphInfo(clipInfo);
				Assert(res);
				//Просматриваем ноды из сформированного списка
				dword findCount = 0;
				for(dword j = 0; j < options->collection.Size(); j++)
				{
					dword nodeIndex = options->collection[j].index;
					PrjAnxNode & node = nodes[nodeIndex];
					if(node.clipIndex == k)
					{
						findCount++;
						node.isValidate = true;
						if(clipInfo.frames != node.clipFrames || fabsf(clipInfo.fps - node.fps) > 1e-8f)
						{
							isChange = true;
							//Маркируем клипы с этим нодом как требующие проверку
							MoviesMarkStage(nodeIndex, movie_stage_check);
						}
						//Копируем параметры
						node.clipFrames = clipInfo.frames;
						node.fps = clipInfo.fps;
						//Убираем из массива проверенный нод
						options->collection.DelIndex(j);
						j--;
					}
				}
				Assert(findCount <= 1);
				if(!findCount)
				{
					//Добавляем новый нод для нового клипа
					PrjAnxNode & node = nodes[nodes.Add()];
					Assert(nameOffset >= 0);
					node.nameOffset = nameOffset;
					node.nameLen = nameLen;
					node.clipIndex = k;
					node.clipFrames = clipInfo.frames;
					node.clipsCount = nodeInfo.numClips;
					node.fps = clipInfo.fps;
					node.nodeId.Build();
					node.isValidate = true;
				}
			}
			nameOffset = -1;
			//Ноды, для которых потерялось соответствие в анимации
			for(dword j = 0; j < options->collection.Size(); j++)
			{
				isChange = true;
				dword nodeIndex = options->collection[j].index;
				MoviesMarkStage(nodeIndex, movie_stage_damage);
			}
			options->collection.Empty();
		}
	}
	//Отгружаем анимацию
	ReleaseAnimation();
	if(isChange)
	{
		SaveChanges();
	}
}

//Получить наимение готовое состояние рогика или же ошибочное
dword ProjectAnimation::GetAnimationStage()
{
	dword stage = movie_stage_done;
	for(dword i = 0; i < movies.Size(); i++)
	{
		Movie * movie = movies[i];
		//Проверяем на наличие ошибок-изменений
		if(movie->stage >= movie_stage_check)
		{
			if(stage < movie->stage)
			{
				stage = movie->stage;
				if(stage == movie_stage_last)
				{
					//Хуже некуда, больше не проверяем
					break;
				}
			}
		}
		//Проверяем на самый незаконченый таск
		if(stage <= movie_stage_done)
		{
			if(stage > movie->stage)
			{
				stage = movie->stage;
			}
		}
	}
	return stage;
}

//Добавить ролик
dword ProjectAnimation::AddMovie(const char * name, long copyFrom)
{
	Assert(!string::IsEmpty(name));
	Assert(string::Len(name) < c_nameLen);
	Movie * sourceMovie = null;
	if(copyFrom >= 0)
	{
		sourceMovie = movies[copyFrom];
	}
	Movie * movie = NEW Movie(name);
	movie->movieId.Build();
	dword index = movies.Add(movie);
	if(sourceMovie)
	{
		memcpy(movie->misName, sourceMovie->misName, c_nameLen);
		movie->nodes.DelAll();
		movie->nodes.AddElements(sourceMovie->nodes.Size());
		for(dword i = 0; i < sourceMovie->nodes.Size(); i++)
		{
			movie->nodes[i] = sourceMovie->nodes[i];
		}
	}
	SaveChanges();
	return index;
}

//Удалить ролик
void ProjectAnimation::DeleteMovie(dword index)
{
	movies[index]->nodes.DelAll();
	for(dword i = 0; i < nodes.Size(); i++)
	{
		DeleteEvents(i, index);
	}	
	delete movies[index];
	movies.DelIndex(index);
	SaveChanges();
}

//Проверить имя ролика на уникальность и правильность
bool ProjectAnimation::CheckMovieName(const char * checkName, long skipIndex)
{
	if(string::IsEmpty(checkName))
	{
		return false;
	}
	if(string::Len(checkName) >= c_nameLen)
	{
		return false;
	}
	ExtName en;
	en.Init(checkName);
	for(dword i = 0; i < movies.Size(); i++)
	{
		ExtName & mn = movies[i]->name;
		if(mn.hash == en.hash && mn.len == en.len)
		{
			if(string::IsEqual(mn.str, en.str))
			{
				return false;
			}
		}
	}
	return true;
}

//Переименовать ролик
void ProjectAnimation::RenameMovie(dword index, const char * newName)
{
	Assert(CheckMovieName(newName, index));
	memset(movies[index]->nameData, 0, c_nameLen);
	crt_strncpy(movies[index]->nameData, c_nameLen, newName, c_nameLen - 1);
	movies[index]->name.Init(movies[index]->nameData);
	SaveChanges();
}

//Получить количество роликов
dword ProjectAnimation::GetMovieCount()
{
	return movies.Size();
}

//Установить миссию для предпросмотра
void ProjectAnimation::SetMission(dword index, const char * missionName)
{
	Assert(string::Len(missionName) < c_nameLen);
	memset(movies[index]->misName, 0, c_nameLen);
	crt_strncpy(movies[index]->misName, c_nameLen, missionName, c_nameLen - 1);
	SaveChanges();
}

//Получить имя ролика
ExtName & ProjectAnimation::GetMovieName(dword index)
{
	return movies[index]->name;
}

//Получить имя миссии
const char * ProjectAnimation::GetMissionName(dword index)
{
	return movies[index]->misName;
}

//Получить массив нодов ролика
array<ProjectAnimation::Node> & ProjectAnimation::GetMovieNodes(dword index)
{
	return movies[index]->nodes;
}

//Получить стадию ролика
dword ProjectAnimation::GetMovieStage(dword index)
{
	return movies[index]->stage;
}

//Установить стадию ролика
void ProjectAnimation::SetMovieStage(dword index, dword stage)
{
	Assert(stage >= movie_stage_first);
	Assert(stage <= movie_stage_last);
	movies[index]->stage = stage;
	ChangeMovieStageLog(index, "Set stage by user");
	SaveChanges();
}

//Получить количество анимационных нодов
dword ProjectAnimation::GetNodesCount()
{
	return nodes.Size();
}

//Получить анимационный нод
const ProjectAnimation::PrjAnxNode & ProjectAnimation::GetNode(dword nodeIndex)
{
	return nodes[nodeIndex];
}

//Получить имя нода
void ProjectAnimation::GetNodeName(dword nodeIndex, string & name)
{
	PrjAnxNode & node = nodes[nodeIndex];
	const char * str = &nodeNames[node.nameOffset];
	name = str;
	if(node.clipsCount > 1)
	{
		name += ": clip ";
		name += node.clipIndex;
	}
}

//Получить имя нода в анимации
const char * ProjectAnimation::GetAnxNodeName(dword nodeIndex)
{
	PrjAnxNode & node = nodes[nodeIndex];
	const char * str = &nodeNames[node.nameOffset];
	return str;
}

//Узнать валидность нода
bool ProjectAnimation::CheckNode(dword nodeIndex)
{
	PrjAnxNode & node = nodes[nodeIndex];
	return node.isValidate;
}

//Добавить событие
dword ProjectAnimation::AddEvent(dword nodeIndex, dword frame, dword movieIndex, const UniqId * copyFromEventId)
{
	Assert(frame < nodes[nodeIndex].clipFrames);
	dword index = nodes[nodeIndex].events.Add();
	Event & evt = nodes[nodeIndex].events[index];	
	evt.frame = frame;
	evt.position = 0;
	evt.eventParams = event_flags_local | event_flags_bind;
	evt.fadeTime = 0.01f;
	evt.soundId.Reset();
	evt.previewSound = null;
	memset(evt.sound, 0, c_nameLen);
	memcpy(evt.locator, locatorName, c_nameLen);
	//memset(evt.locator, 0, c_nameLen);
	evt.eventId.Build();
	evt.movie = movies[movieIndex];
	//Если надо копировать, то ищим эвент и копируем с него параметры
	if(copyFromEventId && copyFromEventId->IsValidate())
	{
		for(dword i = 0; i < nodes.Size(); i++)
		{
			PrjAnxNode & node = nodes[i];
			for(dword j = 0; j < node.events.Size(); j++)
			{
				if(node.events[j].eventId == *copyFromEventId)
				{
					Event & nevt = node.events[j];
					evt.eventParams = nevt.eventParams;
					evt.fadeTime = nevt.fadeTime;
					memcpy(evt.sound, nevt.sound, c_nameLen);
					memcpy(evt.locator, nevt.locator, c_nameLen);
				}
			}
		}
	}
	SaveChanges();
	return index;
}

//Переместить событие на новый кадр
void ProjectAnimation::MoveEvent(dword nodeIndex, dword eventIndex, dword frame)
{
	Assert(frame < nodes[nodeIndex].clipFrames);
	Event & evt = nodes[nodeIndex].events[eventIndex];
	evt.frame = frame;
	SaveChanges();
}

//Поменять эвенты местами
void ProjectAnimation::ExchangeEvents(dword nodeIndex, dword eventIndex1, dword eventIndex2)
{
	Event e = nodes[nodeIndex].events[eventIndex1];
	nodes[nodeIndex].events[eventIndex1] = nodes[nodeIndex].events[eventIndex2];
	nodes[nodeIndex].events[eventIndex2] = e;
	SaveChanges();
}

//Установить позицию рисования эвента
void ProjectAnimation::SetEventPosition(dword nodeIndex, dword eventIndex, dword position)
{
	nodes[nodeIndex].events[eventIndex].position = position;
}

//Получить позицию рисования эвента
dword ProjectAnimation::GetEventPosition(dword nodeIndex, dword eventIndex)
{
	return nodes[nodeIndex].events[eventIndex].position;
}

//Получить диапазон времён проигрывания волн эвента
bool ProjectAnimation::GetEventSoundTime(dword nodeIndex, dword eventIndex, float & minTime, float & maxTime)
{
	//Если имя пустое то времено 0
	minTime = 0.0f;
	maxTime = 0.0f;	
	Event & evt = nodes[nodeIndex].events[eventIndex];
	ProjectSound * ps = EventGetSound(evt);
	if(ps)
	{
		ps->GetPlayTimes(minTime, maxTime);
		return true;
	}
	return false;
}


//Подготовить к проигрыванию звуки
void ProjectAnimation::NodePrepareForSoundPlay(dword nodeIndex, dword prepareFrames)
{
	array<Event> & events = nodes[nodeIndex].events;
	for(dword i = 0; i < events.Size(); i++)
	{		
		Event & evt = events[i];
		if(evt.frame < prepareFrames)
		{
			EventPrepareSound(evt, true);
		}
	}
}

//Подготовить к проигрыванию звук
void ProjectAnimation::EventPrepareForSoundPlay(dword nodeIndex, dword eventIndex)
{
	Event & evt = nodes[nodeIndex].events[eventIndex];
	EventPrepareSound(evt, true);
}

//Проиграть звук
void ProjectAnimation::EventSoundPlay(dword nodeIndex, dword eventIndex)
{
	Event & evt = nodes[nodeIndex].events[eventIndex];
	if(!evt.previewSound)
	{
		EventPrepareSound(evt, true);
		if(!evt.previewSound)
		{
			return;
		}
	}
	evt.previewSound->Play();
}

//Остановить проигрывание звука
void ProjectAnimation::EventSoundStop(dword nodeIndex, dword eventIndex)
{
	Event & evt = nodes[nodeIndex].events[eventIndex];
	if(evt.previewSound)
	{
		evt.previewSound->Stop();
	}
}

//Закончился нод
void ProjectAnimation::EventSoundEndNode(dword nodeIndex, dword eventIndex)
{
	Event & evt = nodes[nodeIndex].events[eventIndex];
	if(evt.previewSound && (evt.eventParams & event_flags_bind))
	{
		evt.previewSound->FadeOut(evt.fadeTime);
	}
}

//Установить позицию звука для эвента
void ProjectAnimation::EventSoundPosition(dword nodeIndex, dword eventIndex, const Vector & pos)
{
	Event & evt = nodes[nodeIndex].events[eventIndex];
	if((evt.eventParams & event_flags_local) == 0)
	{
		return;
	}
	if(!evt.previewSound)
	{
		EventPrepareSound(evt, true);
		if(!evt.previewSound)
		{
			return;
		}
	}
	((ISound3D *)evt.previewSound)->SetPosition(pos);
}

//Нарисовать звуки для внутренней звуковой сцены
void ProjectAnimation::EventSoundDraw()
{
	ISoundService::DebugLevel level = options->soundService->GetDebugLevel();
	dword view = options->soundService->GetDebugView();
	options->soundService->SetDebugLevel(ISoundService::dl_maximum);
	options->soundService->SetDebugView(ISoundService::dv_all);
	options->soundService->DebugDraw();
	options->soundService->SetDebugLevel(level);
	options->soundService->SetDebugView(view);
}

//Назначен зацикленный звук или нет
bool ProjectAnimation::EventSoundIsLoop(dword nodeIndex, dword eventIndex)
{
	Event & evt = nodes[nodeIndex].events[eventIndex];
	EventPrepareSound(evt, false);
	return (evt.eventParams & event_tmp_flag_isloop) != 0;
}

//Получить имя локатора и тип звука
const char * ProjectAnimation::GetEventSoundParams(dword nodeIndex, dword eventIndex, bool * is3D, bool * isBind, bool * isVoice)
{
	Event & evt = nodes[nodeIndex].events[eventIndex];
	if(is3D)
	{
		*is3D = (evt.eventParams & event_flags_local) != 0;
	}
	if(isBind)
	{
		*isBind = (evt.eventParams & event_flags_bind) != 0;
	}
	if(isVoice)
	{
		*isVoice = (evt.eventParams & event_flags_voice) != 0;
	}
	if((evt.eventParams & event_flags_local) != 0)
	{
		return evt.locator;
	}
	return null;
}

//Установить параметры слушателя
void ProjectAnimation::EventsSetListener(const Vector & playerPos, const Vector & cameraPos)
{
	if(!scene) return;
	Matrix mtx;
	mtx.vz = (playerPos - cameraPos).GetXZ();
	float len = mtx.vz.NormalizeXZ();
	if(len < 1e-10f || !mtx.BuildOrient(mtx.vz, Vector(0.0f, 1.0f, 0.0f)))
	{
		scene->GetListenerMatrix(mtx);
	}
	mtx.pos = playerPos;
	scene->SetListenerMatrix(mtx);
}

//Подготовить звуковую сцену для работы
void ProjectAnimation::EventsPrepareSounds()
{
	if(scene)
	{
		EventsDeleteSounds();
	}
	Assert(!scene);
	scene = options->soundService->CreateScene(SoundSceneName, _FL_);
}

//Удалить все звуки, созданные событиями
void ProjectAnimation::EventsDeleteSounds()
{
	for(dword i = 0; i < nodes.Size(); i++)
	{
		array<Event> & events = nodes[i].events;
		for(dword j = 0; j < events.Size(); j++)
		{
			Event & evt = events[j];
			EventDeleteSound(evt);
		}
	}
	if(scene)
	{
		scene->Release();
		scene = null;
	}
}

//Получить звук для события
ProjectSound * ProjectAnimation::EventGetSound(Event & evt)
{
	if(string::IsEmpty(evt.sound))
	{
		EventDeleteSound(evt);
		return null;
	}
	ProjectSound * sound = null;
	if(evt.eventParams & event_tmp_flag_snd_id)
	{
		//Пробуем получить кэшированное значение
		sound = project->SoundGet(evt.soundId);
	}
	if(!sound)
	{
		//Ищем среди существующих звуков
		sound = project->SoundFind(evt.sound);
		if(sound)
		{
			evt.soundId = sound->GetId();
		}
	}
	return sound;
}

//Загрузить звук для события
void ProjectAnimation::EventPrepareSound(Event & evt, bool isCreateISound)
{	
	//Получение идентификатора звука и его параметров
	ProjectSound * sound = null;
	if(evt.eventParams & event_tmp_flag_snd_id)
	{
		sound = project->SoundGet(evt.soundId);
	}
	if(!sound)
	{
		evt.soundId.Reset();
		evt.eventParams &= ~(event_tmp_flag_snd_id | event_tmp_flag_isloop);
		sound = EventGetSound(evt);
		if(!sound)
		{
			evt.soundId.Reset();
			return;
		}
		evt.eventParams |= event_tmp_flag_snd_id;
		const SoundBaseParams * curBp = sound->GetCurrentBaseParams();
		if(curBp)
		{
			if(curBp->loopMode != sbpc_loop_disable)
			{
				evt.eventParams |= event_tmp_flag_isloop;
			}
		}
	}
	Assert(sound);
	if(isCreateISound)
	{
		if(evt.previewSound)
		{
			return;
		}
		if(!scene)
		{
			return;
		}
		if(evt.eventParams & event_tmp_flag_isloop)
		{
			return;
		}		
		if(sound->MakePreview())
		{
			if(evt.eventParams & event_flags_local)
			{
				evt.previewSound = scene->Create3D(sound->GetName().str, Vector(0.0f), _FL_, false, false);
			}else{
				evt.previewSound = scene->Create(sound->GetName().str, _FL_, false, false);
			}
			Assert(evt.previewSound);
		}
	}
}

//Удалить звук события
void ProjectAnimation::EventDeleteSound(Event & evt)
{
	if(evt.previewSound)
	{
		evt.previewSound->Release();
		evt.previewSound = null;
		ProjectSound * psound = project->SoundGet(evt.soundId);
		Assert(psound);
		psound->StopPreview();
	}
	if(evt.eventParams & event_tmp_flag_snd_id)
	{
		evt.eventParams &= ~event_tmp_flag_snd_id;
		evt.soundId.Reset();
	}	
}

//Получить идентификатор эвента по индексу
bool ProjectAnimation::GetEventUIDbyIndex(dword nodeIndex, dword eventIndex, UniqId & uid)
{
	uid.Reset();
	if(nodeIndex >= nodes.Size()) return false;
	if(eventIndex >= nodes[nodeIndex].events.Size()) return false;
	uid = nodes[nodeIndex].events[eventIndex].eventId;
	return true;
}

//Установить имя звука
bool ProjectAnimation::SetEventSoundName(dword nodeIndex, UniqId evtId, const char * name)
{
	dword len = string::Len(name) + 1;
	Assert(len > 1 && len <= c_nameLen);
	array<Event> & events = nodes[nodeIndex].events;
	for(dword i = 0; i < events.Size(); i++)
	{
		Event & evt = events[i];
		if(evt.eventId == evtId)
		{
			//Выгрузить звук
			EventDeleteSound(evt);
			//Поменять имя
			memset(evt.sound, 0, c_nameLen);
			memcpy(evt.sound, name, len);
			events[i].soundId.Reset();
			SaveChanges();
			return true;
		}
	}
	return false;
}

//Получить имя звука
const char * ProjectAnimation::GetEventSoundName(dword nodeIndex, UniqId evtId)
{
	array<Event> & events = nodes[nodeIndex].events;
	for(dword i = 0; i < events.Size(); i++)
	{
		if(events[i].eventId == evtId)
		{			
			return events[i].sound;
		}
	}
	return null;
}

//Установить имя локатора
bool ProjectAnimation::SetEventLocator(dword nodeIndex, UniqId evtId, const char * locator)
{
	dword len = string::Len(locator) + 1;
	Assert(len > 1 && len <= c_nameLen);
	array<Event> & events = nodes[nodeIndex].events;
	for(dword i = 0; i < events.Size(); i++)
	{
		if(events[i].eventId == evtId)
		{
			memset(events[i].locator, 0, c_nameLen);
			memcpy(events[i].locator, locator, len);
			memcpy(locatorName, events[i].locator, c_nameLen);
			SaveChanges();
			return true;
		}
	}
	return false;
}


//Получить имя локатора
const char * ProjectAnimation::GetEventLocator(dword nodeIndex, UniqId evtId)
{
	array<Event> & events = nodes[nodeIndex].events;
	for(dword i = 0; i < events.Size(); i++)
	{
		if(events[i].eventId == evtId)
		{
			return events[i].locator;
		}
	}
	return null;
}

//Установить время фэйда
bool ProjectAnimation::SetEventFadeTime(dword nodeIndex, UniqId evtId, float timeInSec)
{
	array<Event> & events = nodes[nodeIndex].events;
	for(dword i = 0; i < events.Size(); i++)
	{
		if(events[i].eventId == evtId)
		{
			events[i].fadeTime = timeInSec;
			SaveChanges();
			return true;
		}
	}
	return false;
}

//Получить время фэйда
float ProjectAnimation::GetEventFadeTime(dword nodeIndex, UniqId evtId)
{
	array<Event> & events = nodes[nodeIndex].events;
	for(dword i = 0; i < events.Size(); i++)
	{
		if(events[i].eventId == evtId)
		{
			return events[i].fadeTime;
		}
	}
	return 0.0f;
}

//Установить/сбросить заданные флажки
bool ProjectAnimation::SetEventFlags(dword nodeIndex, UniqId evtId, dword flags, bool isSet)
{
	array<Event> & events = nodes[nodeIndex].events;
	for(dword i = 0; i < events.Size(); i++)
	{
		if(events[i].eventId == evtId)
		{
			bool isNeedLoadSound = false;
			dword newFlag = isSet ? flags : ~flags;
			if((newFlag ^ events[i].eventParams) & event_flags_local)
			{
				if(events[i].previewSound != null)
				{
					isNeedLoadSound = true;
					EventDeleteSound(events[i]);
					Assert(!events[i].previewSound);
				}
			}
			if(isSet)
			{				
				events[i].eventParams |= flags;
			}else{
				events[i].eventParams &= ~flags;
			}
			if(isNeedLoadSound)
			{
				EventPrepareSound(events[i], true);
			}
			SaveChanges();
			return true;
		}
	}
	return false;
}

//Получить флажки
dword ProjectAnimation::GetEventFlags(dword nodeIndex, UniqId evtId)
{
	array<Event> & events = nodes[nodeIndex].events;
	for(dword i = 0; i < events.Size(); i++)
	{
		if(events[i].eventId == evtId)
		{
			return events[i].eventParams;
		}
	}
	return invalidate_dword;
}


//Удалить событие
void ProjectAnimation::DeleteEvent(dword nodeIndex, dword eventIndex)
{
	nodes[nodeIndex].events.DelIndex(eventIndex);
	SaveChanges();
}

//Удалить события не принадлежащие ролику
void ProjectAnimation::DeleteEvents(dword nodeIndex, dword movieIndex)
{	
	//Параметры ролика
	Movie * movie = movies[movieIndex];	
	Node * movieNodes = movies[movieIndex]->nodes.GetBuffer();
	dword nodesCount = movies[movieIndex]->nodes.Size();
	//Максимальное количество кадров для данного нода
	dword maxFrames = 0;
	for(dword i = 0; i < nodesCount; i++)
	{
		if(movieNodes[i].index == nodeIndex)
		{
			if(maxFrames < movieNodes[i].framesCount)
			{
				maxFrames = movieNodes[i].framesCount;
			}
		}
	}
	//Собираем эвенты, которые нужно будет удалить
	PrjAnxNode & node = nodes[nodeIndex];
	for(dword i = 0; i < node.events.Size(); i++)
	{
		Event & evt = node.events[i];
		if(evt.movie == movie && evt.frame >= maxFrames)
		{
			node.events.DelIndex(i);
		}else{
			i++;
		}
	}
	SaveChanges();
}

//Сохранить объект
ErrorId ProjectAnimation::OnSaveObject(const char * filePath)
{
	options->saveBuffer.Empty();
	//Шапка и глобальные данные
	SaveRaw(" *** DON'T EDIT THIS FILE!!! ***\r\n\r\n", 36);
	SaveRaw(fileGlobalId, 4);
	SaveUId(GetId());
	dword encode = magicStage[0];
	SaveDword(file_actual_version, magic_global1, magic_global2, encode);
	SaveData(anxName, c_nameLen, magic_global1, magic_global2, encode);
	SaveData(anxPath, c_pathLen, magic_global1, magic_global2, encode);	
	SaveDword(magic_stop, magic_global1, magic_global2, encode);
	SaveRaw("\r\n", 2);
	//Сохраняем ноды
	SaveRaw("\r\n *** Animation nodes ***\r\n\r\n", 30);
	for(dword i = 0; i < nodes.Size(); i++)
	{		
		SaveNode(i, nodes[i]);
	}
	//Сохраняем ролики
	SaveRaw("\r\n *** Movies ***\r\n\r\n", 21);
	for(dword i = 0; i < movies.Size(); i++)
	{
		SaveMovie(*movies[i]);
	}
	//Сохраняем события
	SaveRaw("\r\n *** Events ***\r\n\r\n", 21);
	for(dword i = 0; i < nodes.Size(); i++)
	{
		array<Event> & events = nodes[i].events;
		const UniqId & id = nodes[i].nodeId;
		for(dword j = 0; j < events.Size(); j++)
		{
			SaveEvent(events[j], id);
		}		
	}
	//Закончим файл
	SaveRaw("\r\n\r\n\r\n", 4);
	//Сохраняем буфер на диск
	IFile * file = project->FileOpenBin(filePath, SndProject::poff_isWaitWhenOpen, null, _FL_, 100);
	ErrorId errorId;
	if(!file)
	{
		return options->ErrorOut(null, true, "Animation save: can't open file \"%s\"", filePath);
	}
	bool IO_ERROR_CantTruncateMovieFile = file->Truncate();
	Assert(IO_ERROR_CantTruncateMovieFile);
	bool IO_ERROR_CantWriteDataToMovieFile = (file->Write(options->saveBuffer.GetBuffer(), options->saveBuffer.Size()) == options->saveBuffer.Size());
	Assert(IO_ERROR_CantWriteDataToMovieFile);
	project->FileCloseBin(file, 100);	
	options->saveBuffer.Empty();
	return ErrorId::ok;
}

//Сохранить анимационный нод
__forceinline void ProjectAnimation::SaveNode(dword index, const PrjAnxNode & node)
{	
	SaveRaw(fileNodeId, 4);
	dword encode = magicStage[0];
	SaveData(&node.nodeId, sizeof(node.nodeId), magic_node1, magic_node2, encode);
	SaveDword(node.clipIndex, magic_node1, magic_node2, encode);
	SaveDword(node.clipsCount, magic_node1, magic_node2, encode);
	SaveDword(node.clipFrames, magic_node1, magic_node2, encode);
	SaveData(&node.fps, sizeof(float), magic_node1, magic_node2, encode);
	SaveDword(node.nameLen, magic_node1, magic_node2, encode);
	const char * name = &nodeNames[node.nameOffset];
	SaveData(name, node.nameLen, magic_node1, magic_node2, encode);
	SaveDword(magic_stop, magic_node1, magic_node2, encode);
	SaveRaw("\r\n", 2);
}

//Сохранить ролик
__forceinline void ProjectAnimation::SaveMovie(const Movie & movie)
{
	//Сохраняем ролик
	SaveRaw(fileMovieId, 4);
	SaveUId(movie.movieId);
	dword encode = magicStage[movie.stage];
	SaveData(&movie.movieId, sizeof(movie.movieId), magic_movie1, magic_movie2, encode);
	SaveData(&movie.nameData, c_nameLen, magic_movie1, magic_movie2, encode);
	SaveData(&movie.misName, c_nameLen, magic_movie1, magic_movie2, encode);	
	SaveDword(movie.nodes.Size(), magic_movie1, magic_movie2, encode);
	for(dword i = 0; i < movie.nodes.Size(); i++)	
	{
		const Node & n = movie.nodes[i];
		SaveData(&nodes[n.index].nodeId, sizeof(nodes[n.index].nodeId), magic_movie1, magic_movie2, encode);
		SaveDword(n.framesCount, magic_movie1, magic_movie2, encode);
		SaveDword(n.isEnable ? 1 : 0, magic_movie1, magic_movie2, encode);
	}
	//Сохраняем идентификаторы событий ролика для каждого нода
	options->collection.Empty();
	const array<Node> & mnodes = movie.nodes;
	for(dword i = 0; i < mnodes.Size(); i++)
	{
		dword framesCount = mnodes[i].framesCount;
		dword nodeIndex = mnodes[i].index;
		array<Event> & nevents = nodes[nodeIndex].events;
		for(dword j = 0; j < nevents.Size(); j++)
		{
			Event & evt = nevents[j];
			if(evt.movie == &movie)
			{
				SndOptions::Collection c;
				c.ptr = &evt;
				c.index = j;
				options->collection.Add(c);
			}
		}
	}
	SaveDword(options->collection.Size(), magic_movie1, magic_movie2, encode);
	for(dword i = 0; i < options->collection.Size(); i++)	
	{
		//Сохраняем параметры события, чтобы после подтвертить их правильность
		SndOptions::Collection & c = options->collection[i];
		Event * evt = (Event *)c.ptr;		
		SaveData(&evt->eventId, sizeof(evt->eventId), magic_movie1, magic_movie2, encode);
		SaveDword(evt->frame, magic_movie1, magic_movie2, encode);
		SaveDword(evt->eventParams, magic_movie1, magic_movie2, encode);
		SaveData(&evt->fadeTime, sizeof(evt->fadeTime), magic_movie1, magic_movie2, encode);
		SaveData(evt->sound, c_nameLen, magic_movie1, magic_movie2, encode);
		SaveData(evt->locator, c_nameLen, magic_movie1, magic_movie2, encode);
	}	
	options->collection.Empty();
	SaveDword(magic_stop, magic_movie1, magic_movie2, encode);
	SaveRaw("\r\n", 2);
}

//Сохранить событие
__forceinline void ProjectAnimation::SaveEvent(const Event & evt, const UniqId & nodeId)
{
	SaveRaw(fileEventId, 4);
	SaveUId(evt.eventId);
	dword encode = magicStage[0];
	SaveData(&evt.eventId, sizeof(evt.eventId), magic_event1, magic_event2, encode);
	SaveData(&nodeId, sizeof(nodeId), magic_event1, magic_event2, encode);
	SaveData(&evt.movie->movieId, sizeof(evt.movie->movieId), magic_event1, magic_event2, encode);
	SaveDword(evt.frame, magic_event1, magic_event2, encode);
	SaveDword(evt.position, magic_event1, magic_event2, encode);
	SaveDword(evt.eventParams, magic_event1, magic_event2, encode);
	SaveData(&evt.fadeTime, sizeof(float), magic_event1, magic_event2, encode);
	SaveData(evt.sound, c_nameLen, magic_event1, magic_event2, encode);
	SaveData(evt.locator, c_nameLen, magic_event1, magic_event2, encode);
	SaveDword(magic_stop, magic_event1, magic_event2, encode);
	SaveRaw("\r\n", 2);
}

//Сохранить непреобразованные данные
__forceinline void ProjectAnimation::SaveRaw(const char * src, dword size)
{
	Assert(size > 0);
	dword idx = options->saveBuffer.Size();
	options->saveBuffer.AddElements(size);
	byte * dst = &options->saveBuffer[idx];
	for(; size; size--)
	{
		*dst++ = *src++;
	}
}

//Сохранить непреобразованный идентификатор c подчёркеванием для чтения
__forceinline void ProjectAnimation::SaveUId(const UniqId & id)
{
	const char * sid = id.ToString();
	dword len = string::Len(sid);
	dword idx = options->saveBuffer.Size();
	options->saveBuffer.AddElements(len + 1);
	byte * dst = &options->saveBuffer[idx];
	memcpy(dst, sid, len);
	dst[len] = '_';
}

//Сохранить число
__forceinline void ProjectAnimation::SaveDword(dword d, dword enc1, dword enc2, dword & counter)
{
	SaveData(&d, sizeof(dword), enc1, enc2, counter);
}

//Сохранить данные
__forceinline void ProjectAnimation::SaveData(const void * ptr, dword size, dword enc1, dword enc2, dword & counter)
{
	if(size)
	{
		dword idx = options->saveBuffer.Size();
		options->saveBuffer.AddElements(size*2);
		const byte * src = (const byte *)ptr;
		byte * dst = &options->saveBuffer[idx];
		for(; size; size--)
		{
			byte number = *src++;
			byte c = EncodeStep(number, enc1, enc2, counter);
			EncodeConvert(dst, c);
			dst += 2;
		}
	}
}

//Сделать шаг кодировки
__forceinline byte ProjectAnimation::EncodeStep(byte b, const dword enc1, const dword enc2, dword & counter)
{
	byte c = b ^ byte(counter >> 16);
	counter = counter*enc1 + enc2 + b;
	return c;
}

//Сделать шаг разкодировки
__forceinline byte ProjectAnimation::DecodeStep(byte b, const dword enc1, const dword enc2, dword & counter)
{
	byte c = b ^ byte(counter >> 16);
	counter = counter*enc1 + enc2 + c;
	return c;
}

//Сконвертировать число в текст
__forceinline void ProjectAnimation::EncodeConvert(byte * dst, byte b)
{
	dst[0] = 'z' - (b & 0xf);
	dst[1] = 'a' + ((b >> 4) & 0xf);
}

//Сконвертировать текст (2 символа) в число
__forceinline bool ProjectAnimation::DecodeConvert(byte * data, byte & b)
{
	byte b1 = data[0];
	byte b2 = data[1];
	if(b1 < 'a') b1 += 'a' - 'A';
	if(b2 < 'a') b2 += 'a' - 'A';
	b1 = 'z' - b1;
	b2 = b2 - 'a';
	if((b1 | b2) & 0xf0) return false;
	b = b1 | (b2 << 4);
	return true;
}


//Первоначальная загрузка при старте редактора
ErrorId ProjectAnimation::OnLoadObject(const char * defaultFilePath)
{
	ErrorId errorId;
	//Проверка на готовность к загрузке
	Assert(nodes.Size() == 0);
	Assert(movies.Size() == 0);
	Assert(nodeNames.Size() == 0);
	Assert(ani == null);
	flags |= flags_need_animation_check;
	//Загружаем файл
	ILoadBuffer * buffer = options->fileService->LoadData(defaultFilePath, _FL_);
	if(!buffer)
	{
		return options->ErrorOut(null, true, "Error load movie: Can't open file \"%s\"", defaultFilePath);
	}
	//Разбираем данные по чанкам
	byte * data = (byte *)buffer->Buffer();
	const byte * end = data + buffer->Size();
	array<LoadPtr> ptr(_FL_, 1024);
	dword globalsCount = 0;
	dword nodesCount = 0;
	dword moviesCount = 0;
	while(data < end)
	{
		//Выделяем строку
		LoadPtr p;
		p.data = data;
		p.index = 0;
		p.type = file_error;
		for(; data < end; data++)
		{
			if(*data == '\r' || *data == '\n') break;
		}
		//Проверяем идентификатор		
		if(data - p.data <= 4)
		{
			data++;
			continue;
		}
		if(*(dword *)p.data == *(dword *)fileEventId)
		{
			p.type = file_event;
			p.data += 4 + c_vis_uid_size;
		}else
		if(*(dword *)p.data == *(dword *)fileMovieId)
		{
			p.type = file_movie;
			p.data += 4 + c_vis_uid_size;
			moviesCount++;
		}else
		if(*(dword *)p.data == *(dword *)fileNodeId)
		{
			p.type = file_node;
			p.data += 4;
			nodesCount++;
		}else
		if(*(dword *)p.data == *(dword *)fileGlobalId)
		{
			p.type = file_global;
			p.data += 4;
			globalsCount++;
		}else{
			continue;
		}		
		p.size = data - p.data;
		ptr.Add(p);
	}
	//Анализируем подготовленные чанки
	if(globalsCount != 1)
	{
		buffer->Release();
		return options->ErrorOut(null, true, "Error load movie: Movie data is corrupted (global chunk). File \"%s\"", defaultFilePath);
	}
	//Глобальный чанк
	for(dword i = 0; i < ptr.Size(); i++)
	{
		LoadPtr & lprt = ptr[i];
		if(lprt.type == file_global)
		{
			errorId = LoadGlobal(lprt, GetName().str, defaultFilePath);
			if(errorId.IsError())
			{
				buffer->Release();
				return errorId;
			}
			break;
		}
	}
	//Ноды
	for(dword i = 0; i < ptr.Size(); i++)
	{
		LoadPtr & lprt = ptr[i];
		if(lprt.type == file_node)
		{
			errorId = LoadNode(lprt, defaultFilePath);
			if(errorId.IsError())
			{
				buffer->Release();
				return errorId;
			}
		}
	}
	//Ролики
	for(dword i = 0; i < ptr.Size(); i++)
	{
		LoadPtr & lprt = ptr[i];
		if(lprt.type == file_movie)
		{
			errorId = LoadMovie(lprt, defaultFilePath);
			if(errorId.IsError())
			{
				buffer->Release();
				return errorId;
			}
		}
	}
	//События
	for(dword i = 0; i < ptr.Size(); i++)
	{
		LoadPtr & lprt = ptr[i];
		if(lprt.type == file_event)
		{
			errorId = LoadEvent(lprt, defaultFilePath);
			if(errorId.IsError())
			{
				buffer->Release();
				return errorId;
			}
		}
	}
	//Проверяем корректность эвентов, подвязаных к ролику
	for(dword i = 0; i < movies.Size(); i++)
	{
		dword & stage = movies[i]->stage;
		if(stage != movie_stage_approval && stage != movie_stage_done)
		{
			//Пропускаем те ролики которые пока что не требуют аппрува
			continue;
		}
		//Ролик и контрольная информация по эвентам
		void * lptrPtr = movies[i]->loadTmp;
		dword index = ((LoadPtr *)lptrPtr) - ptr.GetBuffer();
		LoadPtr & lprt = ptr[index];
		Assert(lprt.type == file_movie);
		//Проверяем эвенты на соответствие
		if(!LoadMovieEventsCheck(lprt, i))
		{
			//Несоответствие! Сменяем состояние на новое...
			if(stage == movie_stage_done)
			{
				stage = movie_stage_check;
				ChangeMovieStageLog(i, "LoadMovieEventsCheck -> Events will be changes");
			}else
			if(stage == movie_stage_approval)
			{
				stage = movie_stage_events;
				ChangeMovieStageLog(i, "LoadMovieEventsCheck -> Events will be changes");
			}
		}
		movies[i]->loadTmp = null;
	}	
	buffer->Release();
	return ErrorId::ok;
}


//Загрузить имя и путь
ErrorId ProjectAnimation::LoadGlobal(LoadPtr & lprt, const char * name, const char * defaultFilePath)
{
	//Зачитаем uid
	UniqId id;
	if(!id.FromString((const char *)lprt.data))
	{
		return options->ErrorOut(null, true, "Error load movie: Damage UID (%s). File \"%s\"", id.ToString(), defaultFilePath);
	}
	lprt.data += c_vis_uid_size;
	lprt.size -= c_vis_uid_size;
	if(project->ObjectFind(id, SndProject::pot_any_object))
	{
		return options->ErrorOut(null, true, "Error load movie: Repeat UID (%s). File \"%s\"", id.ToString(), defaultFilePath);
	}
	ReplaceId(id);
	dword stage = LoadDecode(lprt, magic_global1, magic_global2, 0, 0);
	if(stage != 0)
	{
		return options->ErrorOut(null, true, "Error load movie: Error check sum of global data chunk. File \"%s\"", defaultFilePath);
	}
	dword & ver = *(dword *)lprt.data;
	if(ver != file_actual_version)
	{
		return options->ErrorOut(null, true, "Error load movie: Not supported file version. File \"%s\"", defaultFilePath);
	}
	memcpy(anxName, lprt.data + sizeof(dword), c_nameLen);
	memcpy(anxPath, lprt.data + sizeof(dword) + c_nameLen, c_pathLen);
	if(!string::IsEqual(name, anxName))
	{
		return options->ErrorOut(null, true, "Error load movie: Global data chunk is corrupt. File \"%s\"", defaultFilePath);
	}
	return ErrorId::ok;
}

//Загрузить анимационный нод
ErrorId ProjectAnimation::LoadNode(LoadPtr & lprt, const char * defaultFilePath)
{
	dword stage = LoadDecode(lprt, magic_node1, magic_node2, 0, 0);
	if(stage != 0)
	{
		api->Trace("ProjectAnimation::LoadNode -> error check sum");
		flags |= flags_lost_data_when_load;
		return ErrorId::ok;
	}
	PrjAnxNode node;
	node.isValidate = false;
	bool isError = !LoadData(lprt, &node.nodeId, sizeof(node.nodeId));
	isError |= !LoadDword(lprt, node.clipIndex);
	isError |= !LoadDword(lprt, node.clipsCount);
	isError |= !LoadDword(lprt, node.clipFrames);
	isError |= !LoadData(lprt, &node.fps, sizeof(node.fps));
	isError |= !LoadDword(lprt, node.nameLen);
	if(isError || node.nameLen <= 1 || node.nameLen > 4096)
	{
		api->Trace("ProjectAnimation::LoadNode -> data is damaged (change format?)");
		flags |= flags_lost_data_when_load;
		return ErrorId::ok;
	}
	node.nameOffset = nodeNames.Size();
	nodeNames.AddElements(node.nameLen);
	if(!LoadData(lprt, &nodeNames[node.nameOffset], node.nameLen))
	{
		api->Trace("ProjectAnimation::LoadNode -> cant read name (change format?)");
		flags |= flags_lost_data_when_load;
		return ErrorId::ok;
	}
	for(dword i = 0; i < nodes.Size(); i++)
	{
		if(nodes[i].nodeId == node.nodeId)
		{
			api->Trace("ProjectAnimation::LoadNode -> repeat animation node, skip it...");
			flags |= flags_lost_data_when_load;
			return ErrorId::ok;
		}
	}
	nodes.Add(node);
	return ErrorId::ok;
}

//Загрузить ролик
ErrorId ProjectAnimation::LoadMovie(LoadPtr & lprt, const char * defaultFilePath)
{
	dword stage = LoadDecode(lprt, magic_movie1, magic_movie2, movie_stage_first, movie_stage_last);
	if(stage < movie_stage_first || stage > movie_stage_last)
	{
		api->Trace("ProjectAnimation::LoadMovie -> error check sum");
		flags |= flags_lost_data_when_load;
		return ErrorId::ok;
	}
	Movie * movie = NEW Movie("");
	movie->stage = stage;
	movie->loadTmp = &lprt;
	bool isError = !LoadData(lprt, &movie->movieId, sizeof(movie->movieId));
	isError |= !LoadData(lprt, &movie->nameData, c_nameLen);
	isError |= !LoadData(lprt, &movie->misName, c_nameLen);
	//Ноды
	dword nodesCount = 0;
	isError |= !LoadDword(lprt, nodesCount);
	movie->nodes.AddElements(nodesCount);
	PrjAnxNode * prjNodes = nodes.GetBuffer();
	dword prjNCount = nodes.Size();
	bool isChangeMovieStage = false;
	for(dword i = 0; i < nodesCount; i++)	
	{
		Node & n = movie->nodes[i];
		//Индекс нода
		n.index = invalidate_dword;
		UniqId uniqId;
		isError |= !LoadData(lprt, &uniqId, sizeof(uniqId));
		for(dword j = 0; j < prjNCount; j++)
		{
			if(prjNodes[j].nodeId == uniqId)
			{
				n.index = j;
				break;
			}
		}
		//Количество кадров
		isError |= !LoadDword(lprt, n.framesCount);
		//Флаги
		dword flags = 1;
		isError |= !LoadDword(lprt, flags);
		n.isEnable = (flags & 1) ? true : false;
		if(n.index == invalidate_dword)
		{
			n.isEnable = false;
			movie->stage = movie_stage_damage;
			isChangeMovieStage = true;
		}
	}
	//Проверка
	if(isError)
	{
		api->Trace("ProjectAnimation::LoadMovie -> data is damaged (change format?)");
		delete movie;
		flags |= flags_lost_data_when_load;
		return ErrorId::ok;
	}
	//Инициализируем имя
	movie->name.Init(movie->nameData);
	//Проверяем на уникальность
	for(dword i = 0; i < movies.Size(); i++)
	{
		if(movies[i]->movieId == movie->movieId)
		{
			api->Trace("ProjectAnimation::LoadMovie -> repeat movie, skip it %s", movie->movieId.ToString());
			delete movie;
			flags |= flags_lost_data_when_load;
			return ErrorId::ok;
		}
	}
	//Добавляем в общий список
	movies.Add(movie);
	if(isChangeMovieStage)
	{
		ChangeMovieStageLog(movies.Size() - 1, "LoadMovie -> movie data will be changed");
	}
	return ErrorId::ok;
}

//Загрузить эвент
ErrorId ProjectAnimation::LoadEvent(LoadPtr & lprt, const char * defaultFilePath)
{
	dword stage = LoadDecode(lprt, magic_event1, magic_event2, 0, 0);
	if(stage != 0)
	{
		api->Trace("ProjectAnimation::LoadEvent -> error check sum");
		flags |= flags_lost_data_when_load;
		return ErrorId::ok;
	}
	Event evt;
	bool isError = !LoadData(lprt, &evt.eventId, sizeof(evt.eventId));
	UniqId nodeId;
	isError |= !LoadData(lprt, &nodeId, sizeof(nodeId));
	UniqId movieId;
	isError |= !LoadData(lprt, &movieId, sizeof(movieId));
	isError |= !LoadDword(lprt, evt.frame);
	isError |= !LoadDword(lprt, evt.position);
	isError |= !LoadDword(lprt, evt.eventParams);
	isError |= !LoadData(lprt, &evt.fadeTime, sizeof(evt.fadeTime));
	isError |= !LoadData(lprt, evt.sound, c_nameLen);
	isError |= !LoadData(lprt, evt.locator, c_nameLen);
	//Проверка
	if(isError)
	{
		api->Trace("ProjectAnimation::LoadEvent -> data is damaged (change format?)");
		flags |= flags_lost_data_when_load;
		return ErrorId::ok;
	}
	//Ищим ролик, которому принадлежит эвент
	evt.movie = null;
	for(dword i = 0; i < movies.Size(); i++)
	{
		if(movies[i]->movieId == movieId)
		{
			evt.movie = movies[i];
			break;
		}
	}
	if(!evt.movie)
	{
		string movieIdStr = movieId.ToString();
		api->Trace("ProjectAnimation::LoadEvent -> movie not found (%s), skip event (%s)",movieIdStr.c_str(), evt.eventId.ToString());
		flags |= flags_lost_data_when_load;
		return ErrorId::ok;
	}
	//Несохраняемые параметры
	evt.soundId.Reset();
	evt.eventParams &= ~event_tmp_flags_mask;
	evt.previewSound = null;
	//Помещаем эвент в нод
	for(dword i = 0; i < nodes.Size(); i++)
	{
		if(nodes[i].nodeId == nodeId)
		{
			//Нод нашли, добавим
			nodes[i].events.Add(evt);
			return ErrorId::ok;
		}
	}
	//Нод не найден
	api->Trace("ProjectAnimation::LoadEvent -> animation nod not found, skip event (%s)", evt.eventId.ToString());
	flags |= flags_lost_data_when_load;
	return ErrorId::ok;
}

//Декодировать блок данных, возвращает стадию апрува
__forceinline dword ProjectAnimation::LoadDecode(LoadPtr & lprt, const dword enc1, const dword enc2, const dword stateFrom, const dword stateTo)
{
	if(lprt.size < 8 || (lprt.size & 1) == 1)
	{
		return invalidate_dword;
	}
	//Декодируем по всем веткам, выясняя текущую стадию
	dword encs[movie_stage_last + 1];
	dword stopDword[movie_stage_last + 1];
	for(dword j = stateFrom; j <= stateTo; j++)
	{
		encs[j] = magicStage[j];
		stopDword[j] = 0;
	}
	for(dword i = 0; i < lprt.size; i += 2)
	{
		byte b;
		if(!DecodeConvert(&lprt.data[i], b))
		{
			return invalidate_dword;
		}
		for(dword j = stateFrom; j <= stateTo; j++)
		{
			dword & sd = stopDword[j];
			dword res = DecodeStep(b, enc1, enc2, encs[j]);
			sd >>= 8;
			res <<= 24;
			sd = sd | (res & 0xff000000);
		}		
	}
	for(dword j = stateFrom; j <= stateTo; j++)
	{
		if(stopDword[j] == magic_stop)
		{
			break;
		}
	}
	if(j > stateTo)
	{
		return invalidate_dword;
	}
	dword counter = magicStage[j];
	for(dword i = 0, c = 0; i < lprt.size; i += 2, c++)
	{
		byte b;		
		if(!DecodeConvert(&lprt.data[i], b))
		{
			Assert(false);
		}
		lprt.data[c] = DecodeStep(b, enc1, enc2, counter);
	}
	lprt.size /= 2;
	return j;
}

//Забрать из чанка число
__forceinline bool ProjectAnimation::LoadDword(LoadPtr & lprt, dword & dw)
{
	return 	LoadData(lprt, &dw, sizeof(dw));
}

//Забрать из чанка данные
__forceinline bool ProjectAnimation::LoadData(LoadPtr & lprt, void * to, dword size)
{
	if(lprt.index + size > lprt.size) return false;
	const byte * src = lprt.data + lprt.index;
	byte * dst = (byte *)to;		
	const byte * end = lprt.data + lprt.index + size;
	lprt.index += size;
	while(src < end) *dst++ = *src++;
	return true;
}

//Поверить, были ли изменены события
bool ProjectAnimation::LoadMovieEventsCheck(LoadPtr & lprt, dword movieIndex)
{
	//Ролик
	Movie * movie = movies[movieIndex];
	UniqId & mid = movie->movieId;
	//Количество событий присоеденённых
	dword eventsCount = 0;
	if(!LoadDword(lprt, eventsCount))
	{
		api->Trace("ProjectAnimation::LoadMovieEventsCheck -> Movie load data (%s, %s) have invalidate",  movie->nameData, mid.ToString());
		return false;
	}
	//Собираем все загруженые события ролика		
	options->collection.Empty();
	const array<Node> & mnodes = movie->nodes;
	for(dword i = 0; i < mnodes.Size(); i++)
	{
		const Node & mnode = mnodes[i];
		const array<Event> & events = nodes[mnode.index].events;
		for(dword j = 0; j < events.Size(); j++)
		{
			if(events[j].movie == movie)
			{
				//Данное событие принадлежит ролику
				SndOptions::Collection & c = options->collection[options->collection.Add()];
				c.ptr = (void *)&events[j];
			}
		}
	}
	//Проверяем на совпадение по количеству
	if(eventsCount != options->collection.Size())
	{
		api->Trace("ProjectAnimation::LoadMovieEventsCheck -> Movie's events is changed (%s, %s)", movie->nameData, mid.ToString());
		return false;
	}
	//Проходим по сохранёным событиям, убирая из массива найденные
	for(dword i = 0; i < eventsCount; i++)
	{
		EventDataForCheck testEvent;
		memset(&testEvent, 0xff, sizeof(testEvent));
		//Зачитываем сохранённые параметры
		LoadData(lprt, &testEvent.eventId, sizeof(testEvent.eventId));
		//Перебираем список просматривая идентификаторы
		for(dword j = 0; j < options->collection.Size(); j++)
		{
			EventDataForCheck * evt = (EventDataForCheck *)options->collection[j].ptr;
			if(evt && evt->eventId == testEvent.eventId)
			{
				//Сверяем параметры события на соответствие				
				LoadDword(lprt, testEvent.frame);
				LoadDword(lprt, testEvent.eventParams);
				LoadData(lprt, &testEvent.fadeTime, sizeof(testEvent.fadeTime));
				LoadData(lprt, &testEvent.sound, c_nameLen);
				LoadData(lprt, &testEvent.locator, c_nameLen);
				//Событие должно соответствовать копии
				const byte * cur = (const byte *)evt;
				const byte * test = (const byte *)&testEvent;
				for(dword c = 0; c < sizeof(EventDataForCheck); c++, cur++, test++)
				{
					if(*cur != *test)
					{
						api->Trace("ProjectAnimation::LoadMovieEventsCheck -> Movie's events is changed (%s, %s)", movie->nameData, mid.ToString());
						return false;
					}
				}
				break;
			}
		}
		if(j >= options->collection.Size())
		{
			api->Trace("ProjectAnimation::LoadMovieEventsCheck -> Movie's events is changed (%s, %s)", movie->nameData, mid.ToString());
			return false;
		}
	}
	return true;
}

//Изменить стадию ролика на требующую проверки
void ProjectAnimation::MoviesMarkStage(dword nodeIndex, dword stage)
{
	for(dword i = 0; i < movies.Size(); i++)
	{
		Movie * m = movies[i];
		if(m->stage == movie_stage_damage)
		{
			continue;
		}
		if(m->stage == movie_stage_check && stage < movie_stage_check)
		{
			continue;
		}
		for(dword j = 0; j < m->nodes.Size(); j++)
		{
			if(m->nodes[j].index == nodeIndex)
			{
				m->stage = stage;
				ChangeMovieStageLog(i, "Animation nodes will be changed");
				break;
			}
		}
	}
}

//Найти в ноде эвент
ProjectAnimation::Event * ProjectAnimation::FindEvent(dword nodeIndex, const UniqId & eventId)
{
	PrjAnxNode & node = nodes[nodeIndex];
	for(dword i = 0; i < node.events.Size(); i++)
	{
		if(node.events[i].eventId == eventId)
		{
			return &node.events[i];
		}
	}
	return null;
}

//Написать сообщение об изменении стадии ролика
void ProjectAnimation::ChangeMovieStageLog(dword movieIndex, const char * fromChange)
{
	FILE * f = crt_fopen(options->pathStageLog.c_str(), "a+b");
	if(!f) return;
	const char * st = "unknown";
	switch(movies[movieIndex]->stage)
	{
	case movie_stage_movie:
		st = "movie";
		break;
	case movie_stage_events:
		st = "events";
		break;
	case movie_stage_approval:
		st = "approval";
		break;
	case movie_stage_done:
		st = "done";
		break;
	case movie_stage_check:
		st = "check";
		break;
	case movie_stage_damage:
		st = "damage";
		break;
	};
	__time64_t ltime;
	_time64(&ltime);
	struct tm today;
	crt_localtime64(&today, &ltime);
	char tmpbuf[128];
	strftime(tmpbuf, sizeof(tmpbuf), "%d %B %Y, %H:%M:%S", &today);
	fprintf(f, "[%s] Animation: %s, movie: %s, change state to %s (%s)\r\n", tmpbuf, GetName().str, movies[movieIndex]->name.str, st, fromChange);
	fclose(f);
}

