//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphManager	
//============================================================================================
			

#include "GraphManager.h"
#include "..\Commands\AnxCmd_MoveNodes.h"
#include "..\Commands\AnxCmd_ChangeHrcNode.h"
#include "..\Commands\AnxCmd_CreateLink.h"
#include "GraphNodeAnimation.h"

//============================================================================================

GraphManager::GraphManager(AnxOptions & opt) : options(opt),
                                               importTable(_FL_, 256),
											   translateGroupNodes(_FL_, 256)
{
	//Корневой нод
	current = root = NEW GraphNodeBase(options);
	root->name = "Main graph";
	modeCreateLink = false;
	modeMoveNodes = false;
	modeMoveGraph = false;
	from = null;
}

GraphManager::~GraphManager()
{
	Release();
}

//Удалить граф
void GraphManager::Release()
{
	delete root; root = null;
}

//============================================================================================
//Найти по нод имени
GraphNodeBase * GraphManager::Find(const char * nodeName)
{
	Assert(root);
	return root->Find(nodeName);
}

//Определить, есть ли выделенные ноды
bool GraphManager::IsHaveSelNodes()
{
	Assert(current);
	for(long i = 0; i < current->child; i++)
	{
		if(current->child[i]->select) return true;
	}
	return false;
}

//Записать данные в поток
void GraphManager::Write(AnxStream & stream)
{
	stream.EnterWriteSection("Graph section -> ");
	root->Write(stream);
	stream.ExitWriteSection();
}

//Прочитать данные из потока
void GraphManager::Read(AnxStream & stream)
{	
	Assert(!root->child);
	//Секция
	string sid;
	stream.EnterReadSection(sid);
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(sid != "Graph section -> ") throw "Invalidate graph section id";
		root->Read(stream, false);
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		api->Trace("AnxEditor graph section: IORead graph error: %s", err ? err : "<no info>");
	}catch(...){
		api->Trace("AnxEditor graph section: IORead graph unknow error.");
	}
#endif
	//Выйти из секции нода
	stream.ExitReadSection();
}

//Прочитать данные из потока
void GraphManager::Import(AnxStream & stream, GraphNodeBase * importTo)
{
	//Очищаем все таблицы трансляции
	root->DeselectAllChilds();
	importTable.DelAll();
	translateGroupNodes.DelAll();
	bool isInsideTranslates = false;
	//Импортируем анты	
	string sid;
	stream.EnterReadSection(sid);
	if(sid == "Export ants -> ")
	{
#ifdef ANX_STREAM_CATCH
		try
		{
#endif
			isInsideTranslates = true;
			options.project->clipsRepository.BeginTranslates();
			long count = 0;
			stream >> count;
			for(long i = 0; i < count; i++)
			{
				options.project->clipsRepository.ImportAnt(stream);
			}
#ifdef ANX_STREAM_CATCH
		}catch(const char * err){
			api->Trace("AnxEditor graph section: IORead graph error: %s", err ? err : "<no info>");
		}catch(...){
			api->Trace("AnxEditor graph section: IORead graph unknow error.");
		}
#endif
		stream.ExitReadSection();
		sid.Empty();
		stream.EnterReadSection(sid);
	}
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(sid != "Graph section -> ") throw "Invalidate graph section id";
		if(!importTo)
		{
			importTo = root;
		}
		importTo->Read(stream, importTo);
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		api->Trace("AnxEditor graph section: IORead graph error: %s", err ? err : "<no info>");
	}catch(...){
		api->Trace("AnxEditor graph section: IORead graph unknow error.");
	}
#endif
	//Транслируем линки в импорчёных груповых нодах
	for(long i = 0; i < translateGroupNodes; i++)
	{
		GraphNodeBase * gnode = Find(translateGroupNodes[i].c_str());
		if(gnode)
		{
			gnode->ImportTranslate();
		}
	}
	//Выйти из секции нода
	stream.ExitReadSection();
	importTable.DelAll();
	translateGroupNodes.DelAll();
	if(isInsideTranslates)
	{
		options.project->clipsRepository.EndTranslates();
	}	
}

//Экспортировать выделенные ноды в поток
void GraphManager::Export(AnxStream & stream)
{
	//Экспортируем используемые анты
	stream.EnterWriteSection("Export ants -> ");
	array<long> exportList(_FL_, 1024);	
	current->CollectSelectionAnts(exportList);
	long count = exportList.Size();
	stream << count;
	for(long i = 0; i < count; i++)
	{
		options.project->clipsRepository.ExportAnt(exportList[i], stream);
	}
	stream.ExitWriteSection();
	//Экспортим сам граф
	stream.EnterWriteSection("Graph section -> ");
	current->WriteSelection(stream);
	stream.ExitWriteSection();
}

//Удалить эвенты с заданным именем
void GraphManager::DeleteEventsByName(const char * eventName[], dword count, GraphNodeBase * rootNode)
{
	if(!rootNode) rootNode = root;
	if(rootNode->GetType() == anxnt_anode)
	{
		for(dword i = 0; i < count; i++)
		{
			((GraphNodeAnimation *)rootNode)->DeleteEventsByName(eventName[i]);
		}
	}
	for(dword i = 0; i < rootNode->child.Size(); i++)
	{
		if(rootNode->child[i])
		{
			DeleteEventsByName(eventName, count, rootNode->child[i]);
		}
	}
}

//Удалить всё из графа
void GraphManager::Clear()
{
	delete root;
	current = root = NEW GraphNodeBase(options);
	root->name = "Main graph";
	modeCreateLink = false;
	modeMoveNodes = false;
	modeMoveGraph = false;
	from = null;
}

//Собрать все ноды в линейный список
void GraphManager::CollectionNodes(array<GraphNodeBase *> & nodes, GraphNodeBase * rootNode)
{
	if(!rootNode)
	{
		CollectionNodes(nodes, root);
		return;
	}
	nodes.Add(rootNode);
	for(dword i = 0; i < rootNode->child.Size(); i++)
	{
		if(rootNode->child[i])
		{
			CollectionNodes(nodes, rootNode->child[i]);
		}
	}	
}

//============================================================================================

//Отрисовка содержимого проекта
void GraphManager::Draw(IRender * render, const GUIRectangle & rect, GUICliper & clipper)
{
	//Рисуем ноды
	current->DrawChilds(rect.pos, render, clipper);
	//Рисуем создаваемый линк
	if(modeCreateLink)
	{
		GUIHelper::Draw2DLine(startLinkPoint.x, startLinkPoint.y, endLinkPoint.x, endLinkPoint.y, 0xffffffff);
	}
}

//Обрабатываем нажание левой кнопки мыши
void GraphManager::MouseLeftDown(GUIPoint & pnt, dword flags)
{
	pnt -= current->base;
	moveDelta = GUIPoint(0, 0);
	GraphNodeBase * node = current->ChildByPosition(pnt);
	if(node)
	{
		current->links.SelectLink(-1);
		current->child.Del(node);
		current->child.Add(node);
		if(flags & agks_ctrl)
		{
			node->select = false;
		}else{			
			if(!(flags & agks_shift))
			{
				if(!node->select) current->DeselectAllChilds();
			}
			node->select = true;
			modeMoveNodes = true;
			modeMoveGraph = false;
		}
	}else{
		current->DeselectAllChilds();
		modeMoveNodes = false;
		long i = current->links.FindLink(pnt);
		if(i >= 0)
		{
			if(flags & agks_ctrl)
			{
				current->links.SelectLink(i, true, false);
			}else{
				if(!(flags & agks_shift))
				{
					current->links.SelectLink(i, false, true);
				}else{
					current->links.SelectLink(i, false, false);
				}
			}
		}else{
			modeMoveGraph = true;
		}
	}
}

//Обрабатываем отпускание левой кнопки мыши
void GraphManager::MouseLeftUp(GUIPoint & pnt, dword flags)
{
	if(modeMoveNodes)
	{
		Assert(options.project);
		if(moveDelta.x | moveDelta.y)
		{
			options.project->Execute(NEW AnxCmd_MoveNodes(options, moveDelta));
		}
		modeMoveNodes = false;
	}
	if(modeMoveGraph)
	{
		modeMoveGraph = false;
	}
	moveDelta = GUIPoint(0, 0);
}

//Обрабатываем нажание правой кнопки мыши
void GraphManager::MouseRightDown(GUIPoint & pnt, dword flags)
{
	pnt -= current->base;
	moveDelta = GUIPoint(0, 0);
	GraphNodeBase * node = current->ChildByPosition(pnt);
	if(node)
	{
		current->links.SelectLink(-1);
		current->child.Del(node);
		current->child.Add(node);
		if(node->CanFromLink())
		{
			modeCreateLink = true;
			modeMoveNodes = false;
			modeMoveGraph = false;
			startLinkPoint = current->base + node->rect.pos + node->rect.size/2;
			endLinkPoint = current->base + pnt;
			from = node;
		}
	}
}

//Обрабатываем отпускание правой кнопки мыши
void GraphManager::MouseRightUp(GUIPoint & pnt, dword flags)
{
	if(modeCreateLink)
	{
		modeCreateLink = false;
		modeMoveNodes = false;
		modeMoveGraph = false;
		GraphNodeBase * to = current->ChildByPosition(pnt - current->base);
		if(to && to != from && to->CanToLink() && current->links.FindLink(from, to) < 0)
		{
			options.project->Execute(NEW AnxCmd_CreateLink(options, from, to));
		}
	}
}

//Обрабатываем двойной щелчёк мыши
void GraphManager::MouseDblClick(GUIPoint & pnt, dword flags)
{
	pnt -= current->base;
	moveDelta = GUIPoint(0, 0);
	GraphNodeBase * node = current->ChildByPosition(pnt);
	if(node)
	{
		current->DeselectAllChilds();
		current->links.SelectLink(-1);
		node->select = true;
		node->Activate();
	}else{
		long i = current->links.FindLink(pnt);
		if(i >= 0)
		{
			current->links.links[i]->Activate();
			current->links.SelectLink(i);			
		}else{
			Assert(current);
			if(current->parent)
			{
				options.project->Execute(NEW AnxCmd_ChangeHrcNode(current->parent->name, options));
			}
		}
	}
}

//Обрабатываем перемещение мышки
void GraphManager::MouseMove(GUIPoint & pnt, GUIPoint & dpnt, dword flags)
{
	moveDelta += dpnt;
	if(modeCreateLink)
	{
		endLinkPoint = pnt;		
	}else
	if(modeMoveNodes)
	{
		for(long i = 0; i < current->child; i++)
		{
			if(current->child[i]->select) current->child[i]->rect.pos += dpnt;
		}
	}else
	if(modeMoveGraph)
	{
		current->base += dpnt;
	}
}

//============================================================================================

