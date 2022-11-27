//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphNodeBase	
//============================================================================================
			

#include "GraphNodeBase.h"
#include "..\Commands\AnxCmd_CreateNode.h"

//============================================================================================

GraphNodeBase::GraphNodeBase(AnxOptions & options) : opt(options), rect(0, 0, options.node.w, options.node.h), links(options),
													child(_FL_),
													conectedLinks(_FL_)
{
	select = false;
	name = "No named node";
	parent = null;
}

GraphNodeBase::~GraphNodeBase()
{
	while(child) delete child[0];
	if(parent) parent->child.Del(this);
	while(conectedLinks)
	{
		Assert(parent);
		parent->links.DeleteLink(conectedLinks[0]->from, conectedLinks[0]->to);
	}
}


//============================================================================================

//Нарисовать детей
void GraphNodeBase::DrawChilds(GUIPoint pos, IRender * render, GUICliper & clipper)
{
	links.Draw(pos + base, render);
	for(long i = 0; i < child; i++)
	{
		clipper.Push();
		child[i]->Draw(pos + base, render, clipper);
		clipper.Pop();
	}	
}

//Нарисовать нод
void GraphNodeBase::Draw(const GUIPoint & pos, IRender * render, GUICliper & clipper)
{
	if(!IsView(pos)) return;
	GUIHelper::Draw2DRect(rect.x + pos.x, rect.y + pos.y, rect.w, rect.h, select ? opt.node.defSelBkg : opt.node.defBkg);
	GUIHelper::DrawLinesBox(rect.x + pos.x, rect.y + pos.y, rect.w, rect.h, opt.node.defFrm);
	if(opt.node.fontLabel)
	{
		long cx = pos.x + rect.pos.x;
		long cy = pos.y + rect.pos.y;
		clipper.SetRectangle(GUIRectangle(cx, cy, rect.w, rect.h));
		float x = (float)opt.node.fontLabel->GetWidth(name);
		float y = (float)opt.node.fontLabel->GetHeight();
		x = rect.x + pos.x + (rect.w - x)*0.5f;
		y = rect.y + pos.y + (rect.h - y)*0.5f;
		opt.node.fontLabel->Print((int)x, (int)y, 0xffffffff, name);
	}
}

//Проверить поподание в нод
bool GraphNodeBase::Inside(const GUIPoint & pnt)
{
	return rect.Inside(pnt);
}

//Найти по нод имени
GraphNodeBase * GraphNodeBase::Find(const char * nodeName)
{
	if(!nodeName) return false;
	if(string::IsEqual(name, nodeName)) return this;
	for(long i = 0; i < child; i++)
	{
		GraphNodeBase * n = child[i]->Find(nodeName);
		if(n) return n;
	}
	return null;
}

//Найти нод
GraphNodeBase * GraphNodeBase::ChildByPosition(const GUIPoint & pnt)
{
	for(long i = child - 1; i >= 0; i--)
	{
		if(child[i]->Inside(pnt)) return child[i];
	}
	return null;
}

//Снять выделение со всех детей
void GraphNodeBase::DeselectAllChilds()
{
	for(long i = 0; i < child; i++)
	{
		child[i]->select = false;
	}
}

//Виден ли данный нод (по rect)
bool GraphNodeBase::IsView(const GUIPoint & pos)
{
	if(rect.x + pos.x > long(opt.width)) return false;
	if(rect.y + pos.y > long(opt.height)) return false;
	if(rect.x + pos.x + rect.w < 0) return false;
	if(rect.y + pos.y + rect.h < 0) return false;
	return true;
}

//Можно отводить линк
bool GraphNodeBase::CanFromLink()
{
	return true;
}

//Можно присоединять линк
bool GraphNodeBase::CanToLink()
{
	return true;
}

//Нормальный линк подходит к ноду или фейковый
bool GraphNodeBase::IsFakeLink()
{
	return false;
}

//Записать данные в поток
void GraphNodeBase::Write(AnxStream & stream)
{
	stream.WriteVersion(1);
	//Основные параметры нода
	stream << name;
	stream << rect.x << rect.y << rect.w << rect.h;
	stream << base.x << base.y;
	stream << select;
	//Наследственные ноды
	stream << child;
	for(long i = 0; i < child; i++)
	{
		WriteChild(stream, i);
	}
	//Линки вложенного графа
	stream.EnterWriteSection("Links -> ");
	links.Write(stream);
	stream.ExitWriteSection();
}

//Прочитать данные из потока
void GraphNodeBase::Read(AnxStream & stream, GraphNodeBase * importTo)
{
	bool importMode = (importTo != null);
	//Удалим старое
	if(!importMode)
	{
		while(child) delete child[0];
	}	
	if(stream.ReadVersion() != 1) throw "Invalidate version of base node data";
	//Основные параметры нода
	if(importTo != this)
	{
		stream >> name;
		stream >> rect.x >> rect.y >> rect.w >> rect.h;
		stream >> base.x >> base.y;
		stream >> select;
	}else{
		string stmp; int ltmp; bool btmp;
		stream >> stmp;
		stream >> ltmp >> ltmp >> ltmp >> ltmp;
		stream >> ltmp >> ltmp;
		stream >> btmp;
	}
	//В режиме импорта проверим имя нода на повторения
	if(importMode)
	{
		string tmp = name;
		name = "";
		if(opt.project->graph.root->Find(tmp))
		{
			//Подбираем имя
			for(long i = 0; i < 1000000; i++)
			{
				string newName = tmp;
				newName += "_";
				newName += i;
				if(!opt.project->graph.root->Find(newName))
				{
					name = newName;
					rect.x += 30;
					rect.y += 30;
					break;
				}
			}
			Assert(i < 1000000);
			//Регестрируем в таблице
			dword index = opt.project->graph.importTable.Add();
			opt.project->graph.importTable[index].oldNodeName = tmp;
			opt.project->graph.importTable[index].newNodeName = name;
		}else{
			name = tmp;
		}
	}
	//Наследственные ноды	
	long childs = 0;
	stream >> childs;	
	for(long i = 0; i < childs; i++)
	{
		ReadChild(stream, importMode);
	}
	/*
	//Ретранслируем груповые ноды
	for(long i = 0; i < child; i++)
	{
		child[i]->ImportTranslate();
	}*/
	//Линки вложенного графа
	string sid;
	stream.EnterReadSection(sid);
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(sid != "Links -> ") throw "Invalidate links section id";
		links.Read(stream, importMode);
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		api->Trace("AnxEditor node base section: IORead node error: %s", err ? err : "<no info>");
	}catch(...){
		api->Trace("AnxEditor node base section: IORead node unknow error.");
	}
#endif
	stream.ExitReadSection();
	if(importMode && opt.project->graph.root != this)
	{
		select = true;
	}
}

//Ретранслировать данные при импорте
void GraphNodeBase::ImportTranslate()
{
}

//Собрать список антов с выделеных нодов, которые надо сохранить
void GraphNodeBase::CollectAntsFromNode(array<long> & tbl)
{
}

//Собрать список антов с выделеных нодов, которые надо сохранить
void GraphNodeBase::CollectSelectionAnts(array<long> & tbl)
{
	for(long i = 0; i < child; i++)
	{
		if(child[i]->select)
		{
			child[i]->CollectAntsFromNode(tbl);
			child[i]->CollectSelectionAnts(tbl);
		}
	}
}

//Сохранить выделенные ноды и линки в потоке под фиктивным нодом
long GraphNodeBase::WriteSelection(AnxStream & stream)
{
	stream.WriteVersion(1);
	//Основные параметры нода
	stream << name;
	stream << rect.x << rect.y << rect.w << rect.h;
	stream << base.x << base.y;
	stream << select;
	//Наследственные ноды
	long count = 0;
	for(long i = 0; i < child; i++)
	{
		if(child[i]->select)
		{
			count++;
		}
	}
	stream << count;
	for(long i = 0; i < child; i++)
	{
		if(child[i]->select)
		{
			WriteChild(stream, i);
		}		
	}
	//Линки вложенного графа
	stream.EnterWriteSection("Links -> ");
	links.WriteSelection(stream);
	stream.ExitWriteSection();
	return count;
}

//Записать данные дочернего нода в поток
void GraphNodeBase::WriteChild(AnxStream & stream, long index)
{
	//Секция
	stream.EnterWriteSection("Node -> ");
	//Тип нода
	stream << dword(child[index]->GetType());
	//Данные нода
	child[index]->Write(stream);
	//Выйти из секции нода
	stream.ExitWriteSection();
}

//Прочитать данные дочернего нода из потока
void GraphNodeBase::ReadChild(AnxStream & stream, bool importMode)
{
	//Секция
	string sid;
	stream.EnterReadSection(sid);
	GraphNodeBase * createNode = null;
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(sid != "Node -> ") throw "Invalidate node section id";
		//Тип нода
		dword type = anxnt_error;
		stream >> type;
		createNode = AnxCmd_CreateNode::CreateNodeByType(AnxNodeTypes(type), opt);
		if(createNode)
		{
			//Данные нода
			child.Add(createNode);
			createNode->parent = this;
			createNode->Read(stream, importMode ? this : null);
			createNode = null;
		}
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		api->Trace("AnxEditor node base childs section: IORead node error: %s", err ? err : "<no info>");
	}catch(...){
		api->Trace("AnxEditor node base childs section: IORead node unknow error.");
	}
#endif
	if(createNode)
	{
		createNode->parent = null;
		child.Del(createNode);
		delete createNode;
	}
	//Выйти из секции нода
	stream.ExitReadSection();
}

//Получить тип нода
AnxNodeTypes GraphNodeBase::GetType()
{
	return anxnt_bnode;
}
