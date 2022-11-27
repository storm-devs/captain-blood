//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxProject
//============================================================================================


#include "AnxProject.h"
#include "Commands\Command.h"
#include "graph\GraphNodeAnimation.h"


//============================================================================================

AnxProject::TmpSaveGraphParams::~TmpSaveGraphParams()
{
	for(dword i = 0; i < events.Size(); i++)
	{
		GraphNodeAnimation::Event * evt = (GraphNodeAnimation::Event *)events[i];
		delete evt;
	}
	events.Empty();
}

//============================================================================================

AnxProject::AnxProject(AnxOptions & opt) : options(opt), graph(opt), commands(_FL_)
{
	cmdPointer = -1;
	maxUndo = 0;
	numCounter = 0;
	skeleton = null;
	//preveiwModel = "man.gmx";
	ResetCameraPosition();
	startNode = null;
	stopNode = null;
}

AnxProject::~AnxProject()
{
	for(long i = 0; i < commands; i++) delete commands[i];
	if(skeleton) delete skeleton;
	graph.Release();
	clipsRepository.Release();
}

//============================================================================================

//Отрисовка содержимого проекта
void AnxProject::Draw(IRender * render, const GUIRectangle & rect, GUICliper & clipper)
{
	//Рисуем графф
	graph.Draw(render, rect, clipper);
}


//Исполнить команду
void AnxProject::Execute(Command * cmd)
{
	Assert(cmd);
	Assert(cmd->IsEnable());
	cmd->Do();
	delete cmd;

/*
	//Из за введения хранилища антов анду убрал, чтобы не мутить поддержку не используемой дизайнерами фичи

	Assert(cmd);
	Assert(cmd->IsEnable());
	if(cmd->CanUndo() && maxUndo > 0)
	{
		if(commands >= maxUndo)
		{
			delete commands[0];
			commands.DelIndex(0);
		}
		//Отсекаем все дальнейшие действия
		while(commands > cmdPointer + 1)
		{
			delete commands[cmdPointer + 1];		//Было delete commands[cmdPointer]; падало на cmdPointer == -1
			commands.DelIndex(cmdPointer + 1);		//Было commands.DelIndex(cmdPointer);
		}
		//Добавляем новое
		commands.Add(cmd);
		cmdPointer = commands - 1;
		cmd->Do();
	}else{
		cmd->Do();
		delete cmd;
	}
*/	
}

//Создать уникальное имя для нода
void AnxProject::GenUniqueNodeName(string & uname)
{
	char buf[128];
	while(true)
	{
		crt_snprintf(buf, 128, "Node %u", numCounter++);
		if(!graph.Find(buf))
		{
			uname = buf;
			return;
		}
	}
}

//Проверить имя нода на уникальность
bool AnxProject::CheckUniqueNodeName(const string & uname)
{
	return !graph.Find(uname);
}

//Записать данные в поток
void AnxProject::Write(AnxStream & stream)
{
	Assert(skeleton);
	stream.EnterWriteSection("Anx project -> ");
	stream.WriteVersion(6);
	stream << numCounter;
	stream << preveiwModel;
	stream << camAng.x;
	stream << camAng.y;
	stream << camAng.z;
	stream << camPos.x;
	stream << camPos.y;
	stream << camPos.z;
	stream << (startNode ? startNode->name : "");
	stream << (stopNode ? startNode->name : "");
	stream << preveiwParticles;
	stream.ExitWriteSection();
	skeleton->Write(stream);
	clipsRepository.Write(stream);
	graph.Write(stream);
}

//Прочитать данные из потока
void AnxProject::Read(AnxStream & stream)
{
	for(long i = 0; i < commands; i++) delete commands[i];
	graphCopyBuffer.Reset();
	eventCopyBuffer.Reset();
	startNode = null;
	if(skeleton) delete skeleton;
	skeleton = null;
	string sid;
	string startNodeName;
	string stopNodeName;
	stream.EnterReadSection(sid);
	dword ver = 0;
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(sid != "Anx project -> ") throw "Invalidate project section id";
		ver = stream.ReadVersion();
		if(ver < 2 && ver > 6) throw "Invalidate version project section";
		stream >> numCounter;
		if(ver == 2)
		{
			float tmp;
			stream >> tmp >> tmp >> tmp;
		}		
		stream >> preveiwModel;
		stream >> camAng.x;
		stream >> camAng.y;
		stream >> camAng.z;
		stream >> camPos.x;
		stream >> camPos.y;
		stream >> camPos.z;
		stream >> startNodeName;
		if(ver > 4)
		{
			stream >> stopNodeName;
		}
		if(ver > 3)
		{
			stream >> preveiwParticles;
		}
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		api->Trace("AnxEditor project section: IORead graph error: %s", err ? err : "<no info>");
	}catch(...){
		api->Trace("AnxEditor project section: IORead graph unknow error.");
	}
#endif
	stream.ExitReadSection();
	skeleton = NEW MasterSkeleton(options);
	skeleton->Read(stream);
	if(ver > 5)
	{
		clipsRepository.Read(stream);
	}
	graph.Read(stream);
	startNode = graph.Find(startNodeName);
	stopNode = graph.Find(stopNodeName);
	UpdateParticles();
}

//Импортировать данные из потока
const char * AnxProject::Import(AnxStream & stream)
{
	static const char * error1 = "Anx file is damaged";
	static const char * error2 = "Master skeletons is not identical.";
	dword ver = 0;
	string sid;
	string startNodeName;
	for(long i = 0; i < commands; i++) delete commands[i];
	commands.Empty();
	graphCopyBuffer.Reset();
	eventCopyBuffer.Reset();
	stream.EnterReadSection(sid);
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(sid != "Anx project -> ") throw "Invalidate project section id";
		ver = stream.ReadVersion();
		if(ver < 2 && ver > 4) throw "Invalidate version project section";
		stream >> numCounter;
		if(ver == 2)
		{
			float tmp;
			stream >> tmp >> tmp >> tmp;
		}
		string stmp;
		float ftmp;
		stream >> stmp;
		stream >> ftmp;
		stream >> ftmp;
		stream >> ftmp;
		stream >> ftmp;
		stream >> ftmp;
		stream >> ftmp;
		stream >> stmp;
		if(ver > 3)
		{
			stream >> stmp;
		}
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		api->Trace("AnxEditor project section: IORead graph error: %s", err ? err : "<no info>");
		return error1;
	}catch(...){
		api->Trace("AnxEditor project section: IORead graph unknow error.");
		return error1;
	}
#endif
	stream.ExitReadSection();
	//Скипаем мастерскелет
	MasterSkeleton * tmpSkeleton = NEW MasterSkeleton(options);
	tmpSkeleton->Read(stream);
	//Сравнимаем мастер скелеты
	Assert(skeleton);
	bool isEqualSkeleton = skeleton->Compare(tmpSkeleton);
	delete tmpSkeleton;
	if(!isEqualSkeleton)
	{
		api->Trace("AnxEditor import: Current master skeleton is not identical with importing from file.");
		return error2;
	}
	clipsRepository.BeginTranslates();
	//Импорт таблицы клипов
	if(ver > 5)
	{
		clipsRepository.Import(stream);
	}
	//Импортим граф
	graph.Import(stream);
	UpdateParticles();
	clipsRepository.EndTranslates();
	return null;
}

//Импортировать звуковые события, замещая текущие
void AnxProject::ImportEvents(const char * iniFilePath, array<string> & errors)
{	
	string tmp;
	errors.DelAll();
	//Открываем ини файл
	IIniFile * ini = options.fileService->OpenIniFile(iniFilePath, _FL_);
	if(!ini)
	{
		tmp = "Can't open file: ";
		tmp += iniFilePath;
		errors.Add(tmp);
		return;
	}
	//Проверяем флаг действительности
	bool isActual = false;
	long magic = ini->GetLong(null, "flags", -1);
	if((magic & (1 << 13)) == 0)
	{
		string fullNamePath = iniFilePath;
		tmp.GetFileTitle(fullNamePath);
		dword hash = string::HashNoCase(tmp.c_str());
		hash &= 0x3ff;
		magic &= 0x3ff;
		if(hash == magic)
		{
			isActual = true;
		}
	}
	//Описание звукового эвента
	string info = "Imported sound event";
	for(dword i = 0; i < options.eventForms.Size(); i++)
	{
		if(options.eventForms[i]->name == "Snd")
		{
			info = options.eventForms[i]->descr;
			break;
		}
	}
	//Собираем секции
	array<string> sections(_FL_);
	ini->GetSections(sections);
	string nodeName;
	string soundName;
	string locatorName;
	string objectName;
	if(sections.Size() > 0)
	{
		array<GraphNodeAnimation::Clip *> clearClips(_FL_, 256);
		for(dword i = 0; i < sections.Size(); i++)
		{
			//Зачитываем параметры эвента
			const char * section = sections[i].c_str();
			if(string::IsEmpty(section)) continue;
			const char * str = ini->GetString(section, "node", null);
			if(string::IsEmpty(str))
			{
				tmp = "No set node name for section: ";
				tmp += section;
				errors.Add(tmp);
				continue;
			}
			nodeName = str;
			long clipIndex = ini->GetLong(section, "clip", -1);
			long frame = ini->GetLong(section, "frame", -1);
			str = ini->GetString(section, "sound", null);
			if(string::IsEmpty(str))
			{
				tmp = "No set sound name for section: ";
				tmp += section;
				errors.Add(tmp);
				continue;
			}
			soundName = str;
			locatorName = ini->GetString(section, "locator", "");
			objectName = ini->GetString(section, "object", "");
			if(locatorName.IsEmpty()) objectName.Empty();
			bool bind = false;
			float fadeOut = 0.0f;
			if(ini->GetLong(section, "bind", 0) != 0)
			{
				bind = true;
				fadeOut = ini->GetFloat(section, "fadeout", -1.0f);
			}
			bool isVoice = (ini->GetLong(section, "voice", 0) != 0);
			if(clipIndex < 0)
			{
				tmp = "No set clip index for section: ";
				tmp += section;
				errors.Add(tmp);
				continue;
			}
			if(frame < 0)
			{
				tmp = "No set event frame for section: ";
				tmp += section;
				errors.Add(tmp);
				continue;
			}
			if(bind && fadeOut < 0.0f)
			{
				tmp = "No set fade out time for section: ";
				tmp += section;
				errors.Add(tmp);
				continue;
			}
			//Ищим нод с нужным именем
			GraphNodeBase * baseNode = graph.Find(nodeName.c_str());
			if(!baseNode)
			{
				tmp = "Node ";
				tmp += nodeName;
				tmp += " not found. Section: ";
				tmp += section;
				errors.Add(tmp);
				continue;
			}
			//Проверяем тип
			if(baseNode->GetType() != anxnt_anode)
			{
				tmp = "Node ";
				tmp += nodeName;
				tmp += " is not animation. Section: ";
				tmp += section;
				errors.Add(tmp);
				continue;
			}
			GraphNodeAnimation * aniNode = (GraphNodeAnimation *)baseNode;
			//Проверяем клип
			if((dword)clipIndex >= aniNode->clips.Size())
			{
				tmp = "Invalidate clip index (";
				tmp += clipIndex;
				tmp += ") for node ";
				tmp += nodeName;
				tmp += ". Section: ";
				tmp += section;
				errors.Add(tmp);
				continue;				
			}
			//Удаляем звуковые эвенты
			GraphNodeAnimation::Clip * clip = aniNode->clips[clipIndex];
			Assert(clip);
			for(dword cl = 0; cl < clearClips.Size(); cl++)
			{
				if(clearClips[cl] == clip)
				{
					break;
				}
			}
			if(cl >= clearClips.Size())
			{
				clip->DeleteEventsByName("Snd");
				clearClips.Add(clip);
			}
			//Добавляем новый эвент
			GraphNodeAnimation::Event * evt = NEW GraphNodeAnimation::Event();
			evt->info = info;
			evt->name = "Snd";
			evt->frame = frame;
			evt->params.AddElements(6);
			for(dword n = 0; n < 6; n++) evt->params[n] = NEW string();
			*(evt->params[0]) = soundName;
			*(evt->params[1]) = locatorName;
			*(evt->params[2]) = objectName;
			*(evt->params[3]) = bind ? "yes" : "no";
			char buf[32];
			crt_snprintf(buf, sizeof(buf), "%f", fadeOut);
			*(evt->params[4]) = buf;
			*(evt->params[5]) = isVoice ? "yes" : "no";
			evt->noBuffered = true;
			clip->events.Add(evt);
		}
		//Удаляем устаревшие эвенты
		const char * oldEvents[] = {"SndLoc", "SndGlb", "SndVoice"};
		graph.DeleteEventsByName(oldEvents, 3);
	}else{
		tmp = "Ini file no have got sections...";
		errors.Add(tmp);
	}
	//Закрываем ини
	ini->Release();
	ini = null;
}


//Импортировать ролик
void AnxProject::ImportMovie(const char * iniFilePath, array<string> & errors)
{
	string tmp;
	errors.DelAll();
	graphCopyBuffer.Reset();
	eventCopyBuffer.Reset();
	//Получаем полный путь
	string tmpPath;
	options.fileService->BuildPath(iniFilePath, tmpPath);
	string fullSourcePath;
	fullSourcePath.GetFilePath(tmpPath);
	fullSourcePath += "\\";
	fullSourcePath.CheckPath();
	//Открываем ини файл
	IIniFile * ini = options.fileService->OpenIniFile(iniFilePath, _FL_);
	if(!ini)
	{
		tmp = "Can't open file: ";
		tmp += iniFilePath;
		errors.Add(tmp);
		return;
	}
	//Количество действий
	long count = ini->GetLong("character", "actsCount", 0);
	if(count <= 0)
	{
		tmp = "No acts in file: ";
		tmp += iniFilePath;
		errors.Add(tmp);
		ini->Release();
		return;
	}
	//Префикс для триггеров
	const char * tmpString = ini->GetString("character", "movie", null);
	if(string::IsEmpty(tmpString))
	{
		tmp = "No set movie field in file: ";
		tmp += iniFilePath;
		errors.Add(tmp);
		ini->Release();
		return;
	}
	string cutsceneName = tmpString;
	tmpString = ini->GetString("character", "name", null);
	if(string::IsEmpty(tmpString))
	{
		tmp = "No set name field in file: ";
		tmp += iniFilePath;
		errors.Add(tmp);
		ini->Release();
		return;
	}
	cutsceneName += "_";
	cutsceneName += tmpString;	
	//Режим актёра
	bool isMaster = ini->GetLong("character", "master", 0) != 0;
	//Ищим эвент активации миссионного объекта
	string eventInfo = "StartDelay";
	string eventName = "ActivateObject";
	string eventParam0;
	string eventParam1 = "Active";
	bool eventNoBuffered = true;
	for(dword i = 0; i < options.eventForms.Size(); i++)
	{
		if(options.eventForms[i]->descr == "Activate some mission object")
		{
			eventName = options.eventForms[i]->name;
			eventNoBuffered = options.eventForms[i]->noBuffered;
			if(options.eventForms[i]->params.Size() >= 2)
			{
				for(dword j = 0; j < options.eventForms[i]->params[1]->enumValue.Size(); j++)
				{
					if(options.eventForms[i]->params[1]->enumValue[j][0] == 'A' ||
						options.eventForms[i]->params[1]->enumValue[j][0] == 'a')
					{
						eventParam1 = options.eventForms[i]->params[1]->enumValue[j];
						break;
					}
				}
			}
			break;
		}
	}	
	//Загружаем действия
	array<GraphNodeAnimation *> nodes(_FL_);
	nodes.Reserve(count);
	char buf[128];
	string name;	
	long deltaX = 200;
	long deltaY = 150;
	long minX = 100;
	long maxX = 1000;
	GUIPoint nodePoint(minX, 100);
	for(dword i = 0; i < (dword)count; i++)
	{
		//Зачитываем параметры действия
		crt_snprintf(buf, sizeof(buf) - 1, "act_%i", i);
		tmpString = ini->GetString("character", buf, null);
		if(!tmpString)
		{
			tmp = "Can't read act_"; tmp += i; tmp += " from file: "; tmp += iniFilePath; tmp += ". Skip it!";
			errors.Add(tmp);
			continue;
		}
		if(graph.Find(tmpString) != null)
		{
			tmp = "Node with name \""; tmp += tmpString; tmp += "\" already exist (act_"; tmp += i; tmp += "). Delete it before import movie.";
			errors.Add(tmp);
			continue;
		}
		name = tmpString;
		crt_snprintf(buf, sizeof(buf) - 1, "ant_%i", i);
		tmpString = ini->GetString("character", buf, null);
		if(!tmpString)
		{
			tmp = "Can't read ant_"; tmp += i; tmp += " from file: "; tmp += iniFilePath; tmp += ". Info file error.";
			errors.Add(tmp);
			continue;
		}
		tmpPath = fullSourcePath;
		tmpPath += tmpString;
		//Загружаем клип
		GraphNodeAnimation::Clip * clip = NEW GraphNodeAnimation::Clip(options);
		if(!clip->data.Load(tmpPath.c_str()))
		{
			tmp = "Can't load file "; tmp += tmpPath; tmp += "for ant_"; tmp += i; tmp += ". Ini file: "; tmp += iniFilePath; tmp += ". Check it.";
			errors.Add(tmp);
			delete clip;
			continue;
		}
		clip->data.ApplyMasterSkeleton();		
		//Добавим триггерное событие
		if(isMaster)
		{
			eventParam0 = cutsceneName; eventParam0 += "_"; eventParam0 += name; eventParam0 += "_StartDelay";
			GraphNodeAnimation::Event * evt = NEW GraphNodeAnimation::Event();
			evt->info = eventInfo;
			evt->name = eventName;
			evt->frame = clip->data.GetNumFrames(false) - 1;
			evt->params.AddElements(2);
			evt->params[0] = NEW string(eventParam0);
			evt->params[1] = NEW string(eventParam1);
			evt->noBuffered = eventNoBuffered;
			clip->events.Add(evt);
		}
		//Добавляем в граф нод с действием
		GraphNodeAnimation * node = NEW GraphNodeAnimation(options);
		nodes.Add(node);
		node->name = name;
		node->isLoop = false;
		node->isChange = false;
		node->isMovement = false;
		node->isGlobalPos = true;
		node->clips.Add(clip);
		node->rect.pos = nodePoint;
		if(deltaX >= 0)
		{
			if(nodePoint.x >= maxX)
			{
				deltaX = -deltaX;
				nodePoint.y += deltaY;
			}else{
				nodePoint.x += deltaX;
			}
		}else{
			if(nodePoint.x <= minX)
			{
				deltaX = -deltaX;
				nodePoint.y += deltaY;
			}else{
				nodePoint.x += deltaX;
			}
		}
	}
	//Ини больше ненужен
	ini->Release();
	ini = null;
	//Если была ошибка, всё удаляем и выходим
	if(errors.Size() == 0)
	{		
		for(GraphNodeBase * root = graph.current; root && root->GetType() != anxnt_hnode; root = root->parent);
		if(!root) root = graph.root;
		Assert(root);
		root->child.Reserve(root->child.Size() + nodes.Size());
		//Переложим ноды в текущий нод и строим цепочку
		GraphNodeAnimation * prevNode = null;
		for(dword i = 0; i < nodes.Size(); i++)
		{
			GraphNodeAnimation * node = nodes[i];
			nodes[i] = null;
			root->child.Add(node);
			node->parent = root;
			if(prevNode)
			{
				long linkIndex = root->links.CreateLink(prevNode, node);
				GraphLink * link = root->links.links[linkIndex];
				link->data.blendTime = 0.0f;
				link->data.defaultLink = true;
			}
			prevNode = node;
		}
	}
	for(dword i = 0; i < nodes.Size(); i++)
	{
		if(nodes[i])
		{
			delete nodes[i];
		}
	}
}

//Перезагрузить анимационные клипы
void AnxProject::ReloadAnts(const char * path, bool isRecursive, array<string> & errors)
{
	//Собрать все ноды в линейный список
	array<GraphNodeBase *> nodes(_FL_, 1024);
	if(isRecursive)
	{
		graph.CollectionNodes(nodes, graph.current);
	}else{
		GraphNodeBase * current = graph.current;
		if(!current) current = graph.root;
		nodes.AddElements(current->child.Size());
		for(dword i = 0; i < current->child.Size(); i++)
		{
			nodes[i] = current->child[i];
		}
	}
	//Перезагружаем анты
	string applyError;
	for(dword i = 0; i < nodes.Size(); i++)
	{
		if(nodes[i]->GetType() != anxnt_anode)
		{
			continue;
		}
		//Перебираем клипы нода
		GraphNodeAnimation * node = (GraphNodeAnimation *)nodes[i];
		for(dword j = 0; j < node->clips.Size(); j++)
		{
			GraphNodeAnimation::Clip * clip = NEW GraphNodeAnimation::Clip(options);
			string file = path;
			file += "\\";
			file += node->clips[j]->data.name;
			file.Lower().AddExtention(".ant");
			if(clip->data.Load(file))
			{		
				//Загрузили - заменяем				
				applyError.Empty();
				clip->data.ApplyMasterSkeleton(&applyError);
				if(applyError.NotEmpty())
				{
					errors.Add(applyError);
				}
				GraphNodeAnimation::Clip * oldClip = node->clips[j];
				node->clips[j] = clip;
				clip->ExtractInfo(*oldClip, node->isMovement);
				delete oldClip;
			}else{
				string error = "Ant file not loaded:\n    ";
				error += file;
				error += " \nError:\n    ";
				error += clip->data.loadError ? clip->data.loadError : "<unknow>";
				errors.Add(error);
				delete clip;
			}			
		}
	}
}


//Сбросить позицию камеры
void AnxProject::ResetCameraPosition()
{
	camAng = Vector(0.326f, PI, 0.0f);
	camPos = Vector(0.0f, 3.0f, 7.0f);
}

//Имя пиртиклов было изменено
void AnxProject::UpdateParticles()
{
//	try
//	{
		//if(options.particles) options.particles->OpenProject(preveiwParticles);
//	}catch(...){
//		options.gui_manager->MessageBox("Open particles project cause error", "Boom!!! :)", GUIMB_OK);
//	}
}




