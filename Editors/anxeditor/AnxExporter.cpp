//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Anx editor
//============================================================================================
// AnxExporter
//============================================================================================

#include "AnxExporter.h"
#include "AnxProject.h"

//============================================================================================

AnxExporter::AnxExporter(AnxOptions & options) : opt(options),
													error(_FL_, 256),
													bones(_FL_, 256),
													nodes(_FL_, 1024),
													clips(_FL_, 1024),
													links(_FL_, 4096),
													consts(_FL_, 4096),
													events(_FL_, 8192),
													evtParams(_FL_, 65536),
													strTable(_FL_, 65536),
													strTableStr(_FL_, 8192)
{
	buffer = null;
	data = null;
	header = null;
	stringsBase = null;
	tracksData = null;
	err.Reserve(4096);
}

AnxExporter::~AnxExporter()
{
}

//============================================================================================

//Выгрузить анимацию
bool AnxExporter::Export()
{
	Reset();
	//Рекурсивная выгрузка
	ExportTreeProcess(opt.project->graph.root);
	//Экспортируем линки
	PostProcessLinksList();
	//Вытягиваем косточки
	AddBones();
	//Проверяем на допустимость параметров
	if(nodes.Size() == 0)
	{
		err = "Error: Animation not contain nodes.";
		error.Add(err);
		isCritError = true;
	}
	if(clips.Size() == 0)
	{
		err = "Error: Animation not contain clips.";
		error.Add(err);
		isCritError = true;
	}
	if(startNode < 0)
	{
		err = "Error: Start node is not defined.";
		error.Add(err);
		isCritError = true;
	}
	if(error.Size() > 0)
	{		
		dword errors = 0;
		dword warnings = 0;
		for(dword i = 0; i < error.Size(); i++)
		{
			const char * str = error[i].c_str();
			if(str[0] == 'E' || str[0] == 'e')
			{
				errors++;
			}else{
				warnings++;
			}
		}
		err = "";
		error.Add(err);
		err = "Total errors: ";
		err += errors;
		err += ", warnings: ";
		err += warnings;
		error.Add(err);
		err = "";
		error.Add(err);
	}
	if(isCritError)
	{
		err = "Stop export. Please fix errors for continue...";
		error.Add(err);
		return false;
	}
	err = "";
	error.Add(err);
	//Формируем файл данных
	AllocBuffer();
	//Заполняем данные
	//Заполнить заголовки и разметить память
	FillHeaders();
	//Заполнить масив костей
	FillBones();
	//Заполнить ноды
	FillNodes();
	//Заполнить линки
	FillLinks();
	//Заполнить константы
	FillConsts();
	//Заполнить события с параметрами
	FillEvents();
	//Заполнить таблицу поиска
	FillFndTable();
	//Копировать таблицу строк
	CopyStrings();
	//Заполнить таблицу клипов, треков и выгрузить клипы
	ExportClips();
	//Преобразуем в переносимый формат
	header->PrepareAnimation(data);
	return true;
}

//Выгрузить анимацию для предпросмотра клипа
bool AnxExporter::PreviewExport(GraphNodeAnimation::Clip & clip, bool applyPosition, bool globalPos, bool isLoop)
{
	Reset();
	//Анимация с 1м клипом
	NodeInfo & ni = nodes[nodes.Add()];
	ni.node = null;
	ni.nameIndex = AddString("", AnxFndName::f_node);
	ni.clipsIndex = 0;
	ni.clipsCount = 1;
	ni.linksIndex = 0;
	ni.linksCount = 0;
	ni.defLink = 0;
	ni.constsIndex = 0;
	ni.constsCount = 0;
	startNode = 0;
	//Эвенты
	for(long i = 0; i < clip.events; i++)
	{
		if(clip.events[i])
		{
			dword frame = clip.events[i]->frame;
			if(frame < clip.data.GetNumFrames(applyPosition))
			{
				AddEvent(*clip.events[i], frame);
			}			
		}
	}	
	//Клип
	ClipInfo & ci = clips[clips.Add()];
	ci.clip = &clip.data;
	ci.frames = clip.data.GetNumFrames(applyPosition);
	ci.p = 1.0f;
	ci.eventsIndex = 0;
	ci.eventsCount = events;
	ci.isMovement = applyPosition;
	ci.isGlobalPos = globalPos;	
	AddBones();
	//Формируем файл данных
	AllocBuffer();
	//Заполняем данные
	//Заполнить заголовки и разметить память
	FillHeaders();
	//Заполнить масив костей
	FillBones();
	//Заполнить нод
	AnxNode & node = header->nodes.ptr[0];
	node.flags = isLoop ? AnxNode::isLoop : 0;
	node.clips.ptr = header->clips.ptr;
	node.numClips = 1;
	node.links.ptr = header->links.ptr;
	node.numLinks = 0;
	node.defLink = 0;
	node.consts.ptr = header->consts.ptr;
	node.numConsts = 0;
	node.name.ptr = stringsBase + ni.nameIndex;
	header->startNode.ptr = &node;
	//Заполнить события с параметрами
	FillEvents();
	//Заполнить таблицу поиска
	FillFndTable();
	//Копировать таблицу строк
	CopyStrings();
	//Заполнить таблицу клипов, треков и выгрузить клипы
	ExportClips();
	return true;
}

//Получить указатель на буфер данных
const byte * AnxExporter::GetData()
{
	return data;
}

//Получить размер данных
dword AnxExporter::GetSize()
{
	return totalSize;
}


//============================================================================================

//Очистить все буфера
void AnxExporter::Reset()
{
	Assert(opt.project);
	Assert(opt.project->skeleton);
	if(buffer)
	{
		delete buffer;
	}
	buffer = null;
	data = null;
	//Очищаем результаты экспорта
	bones.Empty();
	nodes.Empty();
	clips.Empty();
	links.Empty();
	consts.Empty();
	events.Empty();
	evtParams.Empty();
	bones.Empty();
	strTable.Empty();
	strTable.Add(0);
	strTableStr.Empty();
	for(dword i = 0; i < ARRSIZE(entryStrings); i++)
	{
		entryStrings[i] = -1;
	}
	totalSize = 0;
	header = null;
	stringsBase = null;
	tracksData = null;
	isCritError = false;
	startNode = -1;	
}

//Рекурсивная выгрузка
void AnxExporter::ExportTreeProcess(GraphNodeBase * node)
{
	for(long i = 0; i < node->child; i++)
	{
		if(node->child[i]->GetType() == anxnt_anode)
		{
			GraphNodeAnimation * n = (GraphNodeAnimation *)node->child[i];
			if(n->clips == 0)
			{
				err = "Error: Node \"";
				err += n->name;
				err += "\" not include animation clips!";
				error.Add(err);
				isCritError = true;
				continue;
			}
			//Добавляем новый нод
			dword nodeIndex = nodes.Add();
			NodeInfo & ni = nodes[nodeIndex];
			ni.node = n;
			ni.nameIndex = AddString(n->name, AnxFndName::f_node);
			ni.clipsIndex = clips.Size();
			ni.clipsCount = 0;
			ni.linksIndex = links.Size();
			ni.linksCount = 0;
			ni.defLink = -1;
			ni.constsIndex = consts.Size();
			ni.constsCount = 0;			
			//Добавляем клипы нода
			float normProb = 0.0f;
			for(dword j = 0; j < n->clips.Size(); j++)
			{
				normProb += AddClip(n, j);
			}
			ni.clipsCount = clips.Size() - ni.clipsIndex;
			Assert(ni.clipsCount > 0);
			//Нормализуем вероятности
			if(normProb >= 1e-10f && ni.clipsCount > 1)
			{
				normProb = 1.0f/normProb;
				for(j = 0; j < ni.clipsCount; j++)
				{
					clips[ni.clipsIndex + j].p *= normProb;
				}
			}else{
				float p = 1.0f/ni.clipsCount;
				for(j = 0; j < ni.clipsCount; j++)
				{
					clips[ni.clipsIndex + j].p = p;
				}
			}
			//Добавляем константы нода
			for(j = 0; j < n->consts.Size(); j++)
			{
				AddConst(*n->consts[j]);
			}
			ni.constsCount = consts.Size() - ni.constsIndex;
			//Добавялем линки нода
			for(j = 0; j < n->conectedLinks.Size(); j++)
			{
				if(n->conectedLinks[j]->from == n)
				{
					AddLinkToNode(nodeIndex, n->conectedLinks[j], &n->conectedLinks[j]->data);
				}
			}
			ni.linksCount = links.Size() - ni.linksIndex;
			//Определяем дефолтовый линк
			for(j = 0; j < ni.linksCount; j++)
			{
				LinkInfo & li = links[ni.linksIndex + j];
				if(li.data->defaultLink)
				{
					if(ni.defLink < 0)
					{
						ni.defLink = j;
					}else{
						err = "Warning: Repeat default link \"";
						err += li.data->name;
						err += "\" for node \"";
						err += &strTable[nodes[li.fromNode].nameIndex];
						err += "\"...";
						error.Add(err);
					}					
				}
			}
			if(ni.linksCount >= 2 && ni.defLink < 0 && ni.node->isLoop)
			{
				LinkInfo & li = links[ni.linksIndex];
				err = "Warning: Don't set default link for node \"";
				err += &strTable[nodes[li.fromNode].nameIndex];
				err += "\", select link \"";
				err += links[ni.linksIndex].data->name;
				err += "\" as default...";
				error.Add(err);
				ni.defLink = 0;
			}
			if(ni.defLink < 0)
			{
				ni.defLink = 0;
			}
			//Учитываем стартовый нод
			if(n == opt.project->startNode)
			{
				Assert(startNode == -1);
				startNode = nodeIndex;
			}
		}
		//Продолжаем рекурсивный спуск
		ExportTreeProcess(node->child[i]);
	}
}

//Добавить строку в таблицу строк
dword AnxExporter::AddString(const char * str, AnxFndName::Flags flag)
{
	//Ищем среди добавленых
	long i = FindString(str);
	if(i >= 0)
	{
		strTableStr[i].flags |= flag;
		return strTableStr[i].index;
	}
	//Добавляем строку
	dword index = strTable.Size();
	bool errorString = false;
	for(i = 0; str[i]; i++)
	{
		char c = str[i];
		if(c > 127)
		{
			errorString = true;
		}
		if(c >= 'A' && c <= 'Z') c += 'a' - 'A';
		strTable.Add(c);
	}
	strTable.Add(0);
	if(errorString)
	{
		err = "Warning: String \"";
		err += str;
		err += "\" include invalidate characters!";
		error.Add(err);
	}
	//Добавляем новую запись
	AddStringRecord(index, flag);
	return index;
}

//Добавить запись в таблицу строк
void AnxExporter::AddStringRecord(long index, AnxFndName::Flags flag)
{
	//Параметры строки
	dword len = 0;
	dword hash = string::HashNoCase(&strTable[index], len);
	dword hi = hash & (ARRSIZE(entryStrings) - 1);
	//Запись
	long i = strTableStr.Add();
	StringInfo & si = strTableStr[i];
	si.hash = hash;
	si.len = len;
	si.index = index;
	si.next = entryStrings[hi];
	si.flags = flag;
	entryStrings[hi] = i;
}

//Найти строку среди добавленных
long AnxExporter::FindString(const char * str)
{
	if(!str) str = "";
	//Хэшь строки
	dword len = 0;
	dword hash = string::HashNoCase(str, len);
	dword index = hash & (ARRSIZE(entryStrings) - 1);
	long i = entryStrings[index];
	while(i >= 0)
	{
		StringInfo & si = strTableStr[i];
		if(si.hash == hash && si.len == len)
		{
			const char * s = &strTable[si.index];
			if(string::IsEqual(s, str))
			{
				return i;
			}
		}
		i = si.next;
	}
	return -1;
}

//Добавить к ноду клип
float AnxExporter::AddClip(GraphNodeAnimation * gnode, long index)
{
	Assert(gnode->clips[index]);
	GraphNodeAnimation::Clip & clip = *gnode->clips[index];
	//Описание клипа
	ClipInfo & ci = clips[clips.Add()];
	ci.clip = &clip.data;
	ci.p = clip.data.probability;
	ci.eventsIndex = events.Size();
	ci.eventsCount = 0;
	ci.isMovement = gnode->isMovement;
	ci.isGlobalPos = gnode->isGlobalPos;
	ci.frames = ci.clip->GetNumFrames(ci.isMovement);
	//Добавляем события клипа
	for(long i = 0; i < clip.events; i++)
	{
		dword frame = clip.events[i]->frame;
		if(frame >= ci.frames)
		{
			err = "Error: Event \"";
			err += clip.events[i]->info;
			err += "\" (\"";
			err += clip.events[i]->name;
			err += "\") use frame out of range by clip \"";
			err += clip.data.name;
			err += "\". Node: \"";
			if(gnode)
			{
				err += gnode->name;
			}else{
				err += "unknown name";
			}
			err += "\". Not export it!";
			error.Add(err);
			isCritError = true;
			continue;
		}
		AddEvent(*clip.events[i], frame);
	}
	ci.eventsCount = events.Size() - ci.eventsIndex;
	return ci.p;
}

//Добавить к клипу событие
void AnxExporter::AddEvent(GraphNodeAnimation::Event & evt, dword frame)
{
	EventInfo & e =events[events.Add()];
	e.nameIndex = AddString(evt.name, AnxFndName::f_event);
	e.frame = frame;
	e.paramsIndex = evtParams.Size();
	e.paramsCount = 0;
	e.isNoBuffered = evt.noBuffered;
	//Строковые параметры события
	for(dword i = 0; i < evt.params.Size(); i++)
	{
		dword index = AddString(evt.params[i]->c_str(), AnxFndName::f_empty);
		evtParams.Add(index);
	}
	e.paramsCount = evtParams.Size() - e.paramsIndex;
}

//Добавить к ноду константу
void AnxExporter::AddConst(GraphNodeAnimation::ConstBlock & cst)
{
	if(!cst.consts) return;
	for(long i = 0; i < cst.consts; i++)
	{
		if(cst.consts[i]->type == GraphNodeAnimation::Const::t_empty)
		{
			continue;
		}
		ConstInfo & ci = consts[consts.Add()];
		ci.nameIndex = AddString(cst.consts[i]->name, AnxFndName::f_const);
		switch(cst.consts[i]->type)
		{
		case GraphNodeAnimation::Const::t_string:
			ci.type = AnxConst::ct_string;
			ci.strIndex = AddString(cst.consts[i]->str, AnxFndName::f_empty);
			ci.fvalue = 0.0f;
			break;
		case GraphNodeAnimation::Const::t_float:
			ci.type = AnxConst::ct_float;
			ci.strIndex = AddString("", AnxFndName::f_empty);
			ci.fvalue = cst.consts[i]->flt;			
			break;
		case GraphNodeAnimation::Const::t_blend:
			ci.type = AnxConst::ct_blend;
			ci.strIndex = AddString("", AnxFndName::f_empty);
			ci.fvalue = cst.consts[i]->bld;
			break;
		default:
			Assert(false);
		}
	}	
}

//Добавить к ноду линк
void AnxExporter::AddLinkToNode(long from, GraphLink * link, LinkData * data)
{
	Assert(link);
	switch(link->to->GetType())
	{
	case anxnt_anode:
		//Приплыли, сохраняем параметры данного линка
		AddLinkDataToBuffer(from, *data, link->to->name, false);
		break;
	case anxnt_hnode:
		//Углубляемся в нод
		Assert(link->fake);
		AddLinkToNodeHierarchyDown(from, link);
		break;
	case anxnt_gnode:
		//Надо добавить группу линков на различные ноды
		Assert(link->fake);
		AddLinkToGroupNode(from, (GraphNodeGroup *)link->to);
		break;
	case anxnt_onode:
		//Выходной нод, надо подняться по иерархии вверх
		AddLinkToNodeHierarchyUp(from, link, data);
		break;
	default:
		api->Trace("AnxExporter::AddLinkToNode -> Graph structure is damaged!");
		Assert(false);
	}
}

//Надо добавить группу линков на различные ноды
void AnxExporter::AddLinkToGroupNode(long from, GraphNodeGroup * group)
{	
	for(long i = 0; i < group->vlink; i++)
	{
		//Получаем нод
		GraphNodeBase * baseNode = opt.project->graph.Find(group->vlink[i]->toNodeName);
		if(!baseNode)
		{
			err = "Error: Node \"";
			err += group->vlink[i]->toNodeName;
			err += "\" not found. Group node: \"";
			err += group->name;
			err += "\".";
			error.Add(err);
			isCritError = true;
			continue;
		}
		if(baseNode == group) continue;
		//Если анимационный нод, добавляем его
		switch(baseNode->GetType())
		{
		case anxnt_anode:
			//Приплыли, сохраняем параметры данного линка
			AddLinkDataToBuffer(from, group->vlink[i]->linkData, group->vlink[i]->toNodeName, group->isLoopLinks);
			break;
		case anxnt_gnode:
			//Надо добавить группу линков на различные ноды
			AddLinkToGroupNode(from, (GraphNodeGroup *)baseNode);
			break;
		default:
			err = "Error: Group node \"";
			err += group->name;
			err += "\" contain link to inadmissible node \"";
			err += baseNode->name;
			err += "\"";
			error.Add(err);
			isCritError = true;
		}		
	}
}

//Спуститься по иерархии
void AnxExporter::AddLinkToNodeHierarchyDown(long from, GraphLink * link)
{
	//Убедимся в правильных исходных данных
	Assert(link->fake);
	Assert(link->to->GetType() == anxnt_hnode);
	//Ищим среди детей нода входные линки
	for(long i = 0; i < link->to->child; i++)
	{
		if(link->to->child[i]->GetType() != anxnt_inode) continue;
		GraphNodeInOut * io = (GraphNodeInOut *)link->to->child[i];
		if(io->link == link->toINode) break;
	}
	if(i >= link->to->child)
	{
		err = "Error: Hierarchy node \"";
		err += link->to->name;
		err += "\" not contain input node to link \"";
		err += link->toINode;
		err += "\".";
		error.Add(err);
		isCritError = true;
		return;
	}
	//Прослеживаем линки исходящии из найденного нода
	GraphNodeBase * baseNode = link->to->child[i];
	for(i = 0; i < baseNode->conectedLinks; i++)
	{
		Assert(baseNode->conectedLinks[i]->from == baseNode);
		AddLinkToNode(from, baseNode->conectedLinks[i], &baseNode->conectedLinks[i]->data);
	}
}

//Подняться по иерархии
void AnxExporter::AddLinkToNodeHierarchyUp(long from, GraphLink * link, LinkData * data)
{
	//Убедимся в правильных исходных данных
	Assert(!link->fake);
	Assert(link->to->GetType() == anxnt_onode);
	Assert(link->to->parent);
	Assert(link->to->parent->GetType() == anxnt_hnode);
	//Имя подключения интерисующего нас линка
	string & linkOName = ((GraphNodeInOut *)link->to)->link;
	long count = 0;
	//Ищим интерисующие нас линки среди подключённых
	GraphNodeBase * baseNode = link->to->parent;
	for(long i = 0; i < baseNode->conectedLinks; i++)
	{
		Assert(baseNode->conectedLinks[i]->fake);
		if(baseNode->conectedLinks[i]->fromONode == linkOName)
		{
			AddLinkToNode(from, baseNode->conectedLinks[i], data);
			count++;
		}
	}
	if(!count)
	{
		err = "Warning: Output node \"";
		err += link->to->name;
		err += "\" not connected to link...";
		error.Add(err);
	}
}


//Сохранить данные найденного клипа в буфере, для последующего добавления
void AnxExporter::AddLinkDataToBuffer(long from, const LinkData & link, const char * toNode, bool isEnableLoopLink)
{
	//Пропустим неизвестные линки
	if(!toNode) return;
	//Пропускаем линки сами на себя
	if(!isEnableLoopLink && string::IsEqual(toNode, &strTable[nodes[from].nameIndex]))
	{
		err = "Warning: Skip loop link \"";		
		err += link.name;
		err += "\", node: \"";
		err += toNode;
		err += "\"...";
		error.Add(err);
		return;
	}
	//Добавляем линк в буффер
	LinkInfo & li = links[links.Add()];
	li.fromNode = from;
	li.toNode = -1;
	li.toNodeName = toNode;
	li.data = &link;
}

//Генерим реальный список линков
void AnxExporter::PostProcessLinksList()
{
	for(long i = 0; i < links; i++)
	{
		//Получаем адресат линка
		LinkInfo & li = links[i];
		li.nameIndex = AddString(li.data->name.c_str(), AnxFndName::f_link);
		long nameIndex = FindString(li.toNodeName);
		if(nameIndex >= 0)
		{
			nameIndex = strTableStr[nameIndex].index;
			for(long j = 0; j < nodes; j++)
			{
				if(nodes[j].nameIndex == nameIndex)
				{
					li.toNode = j;
					break;
				}
			}
		}
		if(li.toNode < 0)
		{
			err = "Error: destination node \"";
			err += li.toNodeName;
			err += "\" for link [";
			err += nodes[li.fromNode].node->name;
			err += "] -> [";
			err += li.toNodeName;
			err += "] not found.";
			error.Add(err);
			isCritError = true;
			continue;
		}
		li.arange[0] = li.data->aranges[0];
		li.arange[1] = li.data->aranges[1];
		li.mrange[0] = li.data->mranges[0];
		li.mrange[1] = li.data->mranges[1];
		CorrectLinkRange(nodes[li.fromNode], li, li.arange[0], li.arange[1]);
		CorrectLinkRange(nodes[li.fromNode], li, li.mrange[0], li.mrange[1]);
	}
}

//Проверить диапазон линка
void AnxExporter::CorrectLinkRange(NodeInfo & ni, LinkInfo & li, long & from, long & to)
{
	dword maxFramesMin = clips[ni.clipsIndex].frames;
	dword maxFramesMax = maxFramesMin;
	for(dword i = 1; i < ni.clipsCount; i++)
	{
		dword nf = clips[ni.clipsIndex].frames;
		if(maxFramesMin > nf)
		{
			maxFramesMin = nf;
		}
		if(maxFramesMax < nf)
		{
			maxFramesMax = nf;
		}
	}
	if(from > to)
	{
		long tmp = from;
		from = to;
		to = tmp;
	}
	if(to < 0 || from >= (long)maxFramesMax)
	{
		err = "Error: link [";
		err += nodes[li.fromNode].node->name;
		err += "] -> [";
		err += li.toNodeName;
		err += "] have range out of all clips.";
		error.Add(err);
		isCritError = true;
	}
	if(to < 0 || from >= (long)maxFramesMin)
	{
		err = "Warning: link [";
		err += nodes[li.fromNode].node->name;
		err += "] -> [";
		err += li.toNodeName;
		err += "] have range out of some clips.";
		error.Add(err);
	}
	if(from <= 0 && to >= (long)maxFramesMax)
	{
		from = to = AnxLink::invalidate_range_value;
	}
}

//Добавить имена костей в таблицу имён
void AnxExporter::AddBones()
{
	MasterSkeleton * ms = opt.project->skeleton;
	bones.AddElements(ms->bones.Size());
	for(long i = 0; i < ms->bones; i++)
	{
		BoneInfo & bi = bones[i];
		bi.parent = ms->bones[i]->parent;
		//Добавляем полное имя в массив
		bi.nameIndex = AddString(ms->bones[i]->name.c_str(), AnxFndName::f_empty);		
		//Выделяем коротнкое имя
		const char * str = &strTable[bi.nameIndex];
		for(long j = 0, s = 0; str[j]; j++)
		{
			if(str[j] == '|')
			{
				s = j;
			}
		}
		if(str[s] == '|') s++;
		//Ищем среди добавленых
		long sn = FindString(str + s);
		if(sn < 0)
		{
			//Добавляем новую запись
			AddStringRecord(bi.nameIndex + s, AnxFndName::f_empty);
			bi.nameShortIndex = bi.nameIndex + s;
		}else{
			bi.nameShortIndex = strTableStr[sn].index;
		}
		//Хэшь строк
		bi.hash = string::HashNoCase(&strTable[bi.nameIndex]);
		bi.hashShort = string::HashNoCase(&strTable[bi.nameShortIndex]);
	}
}

//Выделить буфер на максимальный возможный объём файла
void AnxExporter::AllocBuffer()
{
	dword size = sizeof(AnxHeaderId) + sizeof(AnxHeader);
	dword numBones = opt.project->skeleton->bones.Size();
	size += numBones*sizeof(AnxBone);
	size += nodes.Size()*sizeof(AnxNode);
	size += links.Size()*sizeof(AnxLink);
	size += consts.Size()*sizeof(AnxConst);
	size += events.Size()*sizeof(AnxEvent);
	size += strTableStr.Size()*sizeof(AnxFndName);
	size += clips.Size()*sizeof(AnxClip);
	size += clips.Size()*sizeof(AnxMvTrackHeader);
	size += clips.Size()*numBones*sizeof(AnxTrackHeader);
	size += ec_max_bones_hashtable*sizeof(AnxBone *);
	size += ec_max_bones_hashtable*sizeof(AnxBone *);
	size += ec_max_names_hashtable*sizeof(AnxFndName *);
	size += evtParams.Size()*sizeof(const char *);
	dword frameSize = numBones*7*sizeof(float);
	dword movementSize = 2*sizeof(float);
	dword totalFrameSize = frameSize + movementSize;
	for(long i = 0; i < clips; i++)
	{
		dword numFrames = clips[i].frames;
		size += numFrames*totalFrameSize;
	}
	totalSize = size + 1024;	
	buffer = NEW byte[totalSize + 16];
	data = ((buffer - (byte *)0 + 0xf) & ~0xf) + (byte *)0;
	memset(data, 0, totalSize);
}

//Заполнить заголовки и разметить память
void AnxExporter::FillHeaders()
{
	//Убедимся что исполняемся на правильной машине
	AnxHeaderId::CheckMachine();
	//Идентификатор файла
	AnxHeaderId & hId = *(AnxHeaderId *)data;
	hId.Init();
	//Заголовок
	byte * ptr = data + sizeof(AnxHeaderId);
	header = (AnxHeader *)ptr;
	ptr += sizeof(AnxHeader);
	//Кости
	header->bones.ptr = (AnxBone *)ptr;
	header->numBones = bones.Size();
	ptr += header->numBones*sizeof(AnxBone);
	//Ноды
	header->nodes.ptr = (AnxNode *)ptr;
	header->numNodes = nodes.Size();
	ptr += header->numNodes*sizeof(AnxNode);
	//Линки
	header->links.ptr = (AnxLink *)ptr;
	header->numLinks = links.Size();
	ptr += header->numLinks*sizeof(AnxLink);
	//Константы
	header->consts.ptr = (AnxConst *)ptr;
	header->numConsts = consts.Size();
	ptr += header->numConsts*sizeof(AnxConst);
	//События
	header->events.ptr = (AnxEvent *)ptr;
	header->numEvents = events.Size();
	ptr += header->numEvents*sizeof(AnxEvent);
	//Элементы поиска
	header->fndNames.ptr = (AnxFndName *)ptr;
	header->numNames = 0;
	for(long i = 0; i < strTableStr; i++)
	{
		if(strTableStr[i].flags != AnxFndName::f_empty)
		{
			header->numNames++;
		}
	}	
	ptr += header->numNames*sizeof(AnxFndName);
	//Клипы
	header->clips.ptr = (AnxClip *)ptr;
	header->numClips = clips.Size();
	ptr += header->numClips*sizeof(AnxClip);
	//Заголовки треков перемещения
	header->movmentTracks.ptr = (AnxMvTrackHeader *)ptr;
	header->numMovmentTracks = 0;
	for(long i = 0; i < clips; i++)
	{
		if(clips[i].isMovement)
		{
			header->numMovmentTracks++;
		}
	}
	ptr += header->numMovmentTracks*sizeof(AnxMvTrackHeader);
	//Заголовки треков костей
	header->bonesTracks.ptr = (AnxTrackHeader *)ptr;
	header->numBonesTracks = clips.Size()*bones.Size();
	ptr += header->numBonesTracks*sizeof(AnxTrackHeader);
	//Входная таблица для поиска кости по полному имени
	header->htBones.ptr = (AnxPointer<AnxBone> *)ptr;
	HashCalculator * hc = NEW HashCalculator(ec_max_bones_hashtable);
	for(long i = 0; i < bones; i++)
	{
		BoneInfo & bi = bones[i];
		hc->AddHash(bi.hash);
	}
	dword tSize = totalSize - sizeof(dword)*(ec_max_bones_hashtable*2 + ec_max_names_hashtable);
	header->htBonesMask = hc->GetOptimalMask(tSize);
	delete hc; hc = null;
	ptr += (header->htBonesMask + 1)*sizeof(AnxBone *);
	//Входная таблица для поиска кости по короткому имени
	header->htShortBones.ptr = (AnxPointer<AnxBone> *)ptr;
	hc = NEW HashCalculator(ec_max_bones_hashtable);
	for(long i = 0; i < bones; i++)
	{
		BoneInfo & bi = bones[i];
		hc->AddHash(bi.hashShort);
	}
	header->htShortBonesMask = hc->GetOptimalMask(tSize);
	delete hc; hc = null;
	ptr += (header->htShortBonesMask + 1)*sizeof(AnxBone *);
	//Входная таблица для поиска именён
	header->htNames.ptr = (AnxPointer<AnxFndName> *)ptr;
	hc = NEW HashCalculator(ec_max_names_hashtable);
	for(long i = 0; i < strTableStr; i++)
	{
		StringInfo & si = strTableStr[i];
		if(si.flags != AnxFndName::f_empty)
		{
			hc->AddHash(si.hash);
		}		
	}
	header->htNamesMask = hc->GetOptimalMask(tSize);
	delete hc; hc = null;
	ptr += (header->htNamesMask + 1)*sizeof(AnxFndName *);
	//Массив указателей на параметры событий
	header->evtPrms.ptr = (AnxPointer<const char> *)ptr;
	header->numEvtPrms = evtParams.Size();
	ptr += header->numEvtPrms*sizeof(const char *);
	//База строк
	stringsBase = (const char *)ptr;
	ptr += strTable.Size();
	//Стартовый нод
	Assert(startNode >= 0);
	Assert(startNode < (long)header->numNodes);
	header->startNode.ptr = header->nodes.ptr + startNode;
	//Флаги
	header->flags = 0;
	//Буфер для данных трека
	tracksData = ptr;
}

//Заполнить масив костей
void AnxExporter::FillBones()
{
	for(dword i = 0; i <= header->htBonesMask; i++)
	{
		header->htBones.ptr[i].ptr = null;
	}
	for(dword i = 0; i <= header->htShortBonesMask; i++)
	{
		header->htShortBones.ptr[i].ptr = null;
	}
	for(dword i = 0; i < header->numBones; i++)
	{
		BoneInfo & bi = bones[i];
		AnxBone & bone = header->bones.ptr[i];
		bone.parent = bi.parent;
		bone.flags = 0;
		//Полное имя
		bone.name.ptr = stringsBase + bi.nameIndex;
		bone.hash = bi.hash;
		bone.nextName.ptr = null;
		dword eindex = bone.hash & header->htBonesMask;
		if(header->htBones.ptr[eindex].ptr)
		{
			for(AnxBone * b = header->htBones.ptr[eindex].ptr; b->nextName.ptr != null; b = b->nextName.ptr);
			b->nextName.ptr = &bone;
		}else{
			header->htBones.ptr[eindex].ptr = &bone;
		}
		//Короткое имя
		bone.shortName.ptr = stringsBase + bi.nameShortIndex;
		bone.shortHash = bi.hashShort;
		bone.nextShortName.ptr = null;
		eindex = bone.shortHash & header->htShortBonesMask;
		if(header->htShortBones.ptr[eindex].ptr)
		{
			for(AnxBone * b = header->htShortBones.ptr[eindex].ptr; b->nextShortName.ptr != null; b = b->nextShortName.ptr);
			b->nextShortName.ptr = &bone;
		}else{
			header->htShortBones.ptr[eindex].ptr = &bone;
		}
	}
}

//Заполнить ноды
void AnxExporter::FillNodes()
{
	for(dword i = 0; i < header->numNodes; i++)
	{
		NodeInfo & ni = nodes[i];
		AnxNode & node = header->nodes.ptr[i];
		node.flags = 0;
		if(ni.node->isLoop) node.flags |= AnxNode::isLoop;
		if(ni.node->isChange) node.flags |= AnxNode::isChange;
		if(opt.project->stopNode == ni.node) node.flags |= AnxNode::isStop;
		node.clips.ptr = header->clips.ptr + ni.clipsIndex;
		node.numClips = ni.clipsCount;
		node.links.ptr = header->links.ptr + ni.linksIndex;
		node.numLinks = word(ni.linksCount);
		node.defLink = word(ni.defLink);
		node.consts.ptr = header->consts.ptr + ni.constsIndex;
		node.numConsts = ni.constsCount;
		node.name.ptr = stringsBase + ni.nameIndex;
	}
}

//Заполнить линки
void AnxExporter::FillLinks()
{
	for(dword i = 0; i < header->numLinks; i++)
	{
		LinkInfo & li = links[i];
		AnxLink & link = header->links.ptr[i];
		//Коэфициент время блендинга между нодами 1/t, если <= 0 то мгновенно переключаться
		if(li.data->blendTime > 1e-8)
		{
			link.kBlendTime = 1.0f/li.data->blendTime;
		}else{
			link.kBlendTime = 0.0f;
		}
		//Относительная синхронизация нодов
		if(links[i].data->isSync)
		{
			link.syncPos = li.data->syncPos*0.01f;
			if(link.syncPos < 0.0f) link.syncPos = 0.0f;
			if(link.syncPos > 1.0f) link.syncPos = 1.0f;
		}else{
			link.syncPos = -1.0f;
		}
		//Нод, на который переходит линк
		link.toNode.ptr = header->nodes.ptr + li.toNode;
		//Имя линка
		link.name.ptr = stringsBase + li.nameIndex;
		//Диапазон активации по кадрам
		link.arange[0] = li.arange[0];
		link.arange[1] = li.arange[1];
		//Диапазон перехода по кадрам
		link.mrange[0] = li.mrange[0];
		link.mrange[1] = li.mrange[1];
	}
}

//Заполнить константы
void AnxExporter::FillConsts()
{
	for(dword i = 0; i < header->numConsts; i++)
	{
		ConstInfo & ci = consts[i];
		AnxConst & cnst = header->consts.ptr[i];
		cnst.name.ptr = stringsBase + ci.nameIndex;
		cnst.type = ci.type;
		//Строковое значение константы
		if(ci.strIndex >= 0)
		{
			cnst.svalue.ptr = stringsBase + ci.strIndex;
		}else{
			cnst.svalue.ptr = null;
		}
		//Числовое значение константы
		cnst.fvalue = ci.fvalue;
	}
}

//Заполнить события с параметрами
void AnxExporter::FillEvents()
{
	for(dword i = 0; i < header->numEvents; i++)
	{
		EventInfo & ei = events[i];
		AnxEvent & evt = header->events.ptr[i];
		evt.name.ptr = stringsBase + ei.nameIndex;
		evt.frame = ei.frame;
		evt.params.ptr = &header->evtPrms.ptr[ei.paramsIndex].ptr;
		evt.numParams = ei.paramsCount;
		evt.flags = 0;
		if(ei.isNoBuffered) evt.flags |= AnxEvent::flg_isNoBuffered;
	}
	for(dword i = 0; i < header->numEvtPrms; i++)
	{
		dword strIndex = evtParams[i];
		header->evtPrms.ptr[i].ptr = stringsBase + strIndex;
	}
}

//Заполнить таблицу поиска
void AnxExporter::FillFndTable()
{
	for(long i = 0, n = 0; i < strTableStr; i++)
	{
		StringInfo & si = strTableStr[i];
		if(si.flags != AnxFndName::f_empty)
		{
			AnxFndName & fnd = header->fndNames.ptr[n++];			
			fnd.name.ptr = stringsBase + si.index;
			fnd.hash = si.hash;
			fnd.next.ptr = null;
			fnd.type = si.flags;
			dword eindex = fnd.hash & header->htNamesMask;
			if(header->htNames.ptr[eindex].ptr)
			{
				for(AnxFndName * f = header->htNames.ptr[eindex].ptr; f->next.ptr != null; f = f->next.ptr);
				f->next.ptr = &fnd;
			}else{
				header->htNames.ptr[eindex].ptr = &fnd;
			}
		}
	}
	Assert(n == header->numNames);
}

//Заполнить таблицу клипов, треков и выгрузить клипы
void AnxExporter::ExportClips()
{
	byte * tData = tracksData;
	array<byte> buffer(_FL_, 4096);
	for(dword i = 0, mt = 0, bt = 0; i < header->numClips; i++)
	{
		//Описание клипа
		ClipInfo & ci = clips[i];
		AnxClip & clip = header->clips.ptr[i];
		clip.btracks.ptr = header->bonesTracks.ptr + bt;
		bt += header->numBones;		
		clip.frames = ci.frames;
		clip.fps = ci.clip->fps;
		if(ci.isMovement)
		{
			clip.mtrack.ptr = header->movmentTracks.ptr + mt;
			mt += 1;
		}else{
			clip.mtrack.ptr = null;
		}		
		clip.probability = ci.p;
		clip.events.ptr = header->events.ptr + ci.eventsIndex;
		clip.numEvents = ci.eventsCount;
		//Выгрудаем клип
		dword dataSize = ci.clip->ExportData(clip.btracks.ptr, clip.mtrack.ptr, ci.isGlobalPos, tData, buffer);
		tData += dataSize;
	}
	//Дополняем размер буферной зоной поскольку из треков читаеться за раз по 4 байта
	for(dword i = 0; i < 4; i++)
	{
		*tData++ = 0;
	}
	//Выравнимаем размер по 4 байтам
	dword size = -1;
	for(dword i = 0; i < 4; i++)
	{
		size = tData - data;
		if((size & 3) == 0)
		{
			break;
		}
		*tData++ = 0;
	}
	Assert(size < totalSize);
	Assert(mt == header->numMovmentTracks);
	Assert(bt == header->numBonesTracks);
	totalSize = size;
}

//Копировать таблицу строк
void AnxExporter::CopyStrings()
{
	const char * src = strTable.GetBuffer();
	const char * srcEnd = src + strTable.Size();
	char * dst = (char *)stringsBase;	
	while(src < srcEnd)
	{
		*dst++ = *src++;
	}
}


//============================================================================================

AnxExporter::HashCalculator::HashCalculator(dword maxSize) : descs(_FL_),
															 buffer(_FL_)
{
	Assert(maxSize > 0);
	Assert((maxSize & (maxSize - 1)) == 0);
	buffer.AddElements(maxSize*2);
	memset(buffer.GetBuffer(), 0, buffer.GetDataSize());
	for(dword size = 1, curTable = 0; size <= maxSize; size <<= 1)
	{
		Desc & d = descs[descs.Add()];
		d.table = &buffer[curTable];
		d.mask = size - 1;
		d.maxChain = 0;
		curTable += size;
	}
	count = 0;
}

void AnxExporter::HashCalculator::AddHash(dword hash)
{
	for(long i = 0; i < descs; i++)
	{
		Desc & d = descs[i];
		d.table[hash & d.mask]++;
	}
	count++;
}

dword AnxExporter::HashCalculator::GetOptimalMask(dword totalSize)
{
	Assert(count);
	//Получаем статистику по хэшь-таблицам
	for(long i = 0; i < descs; i++)
	{
		Desc & d = descs[i];
		d.maxChain = 0;
		float middle = 0.0f;
		float filling = 0;
		for(dword j = 0; j <= d.mask; j++)
		{
			dword v = d.table[j];
			if(d.maxChain < v)
			{
				d.maxChain = v;
			}
			if(v > 0)
			{
				filling += 1.0f;
				middle += v;
			}			
		}
		if(filling > 0.0f) middle /= filling;
		filling /= (d.mask + 1.0f);
		//Расчитываем вес таблицы исходя из кретериев выбора
		//Вес размера в общем файле
		float sizeInPercents = 100.0f*((d.mask + 1.0f)/totalSize);
		float wSize = 0.001f/sizeInPercents;
		if(wSize > 2.0f) wSize = 2.0f;
		//Вес заполнения
		float wFilling = powf(filling, 1.2f);
		if(wFilling > 1.0f) wFilling = 1.0f;
		if(wFilling < 0.1f) wFilling = 0.1f;
		//Вес дилнны цепочки
		float wChain = powf(2.0f/middle, 4.0f);
		//Вес равномерных длинн цепочек
		float wFlat = 1.0f - (d.maxChain - middle)/d.maxChain;
		//Сохраняем сумарный вес
		d.weight = wSize*wFilling*wChain*wFlat;
	}
	//Выкидываем таблицы с одинаковыми цепочками но большим размером
	float bestWeight = descs[0].weight;
	dword bestSize = descs[0].mask;
	for(long i = 1; i < descs; )
	{
		Desc & d = descs[i];
		if(d.maxChain != descs[i - 1].maxChain)
		{			
			if(d.weight > bestWeight)
			{
				bestWeight = d.weight;
				bestSize = d.mask;
			}
			i++;
		}else{
			descs.DelIndex(i);
		}
	}
	return bestSize;
}

