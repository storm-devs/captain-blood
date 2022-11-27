//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphLinks	
//============================================================================================
			

#include "GraphLinks.h"
#include "GraphNodeBase.h"
#include "..\AnxProject.h"

//============================================================================================

GraphLinks::GraphLinks(AnxOptions & options) : opt(options), links(_FL_)
{

}

GraphLinks::~GraphLinks()
{
	for(long i = 0; i < links; i++) delete links[i];
	links.Empty();
}


//============================================================================================

void GraphLinks::Draw(const GUIPoint & pos, IRender * render)
{
	GUIPoint points[4];

	bool isShowInfo = opt.IsShowInfo();
	//Рисуем линии
	for(long i = 0; i < links; i++)
	{
		//Позиции линка
		GUIPoint ps = pos; ps += links[i]->from->rect.pos + links[i]->from->rect.size/2;
		GUIPoint pd = pos; pd += links[i]->to->rect.pos + links[i]->to->rect.size/2;
		//Рисуем линию
		if(ps.x < 0 && pd.x < 0) continue;
		if(ps.y < 0 && pd.y < 0) continue;
		if(ps.x >= long(opt.width) && pd.x >= long(opt.width)) continue;
		if(ps.y >= long(opt.height) && pd.y >= long(opt.height)) continue;
		GUIHelper::Draw2DLine(ps.x, ps.y, pd.x, pd.y, !isShowInfo ? 0xffffffff : 0xff000000);
	}
	//Рисуем треугольники
	for(i = 0; i < links; i++)
	{
		//Позиции линка
		GUIPoint ps = pos; ps += links[i]->from->rect.pos + links[i]->from->rect.size/2;
		GUIPoint pd = pos; pd += links[i]->to->rect.pos + links[i]->to->rect.size/2;
		//Рисуем треугольник
		Vector vdir = !Vector(float(pd.x - ps.x), 0.0f, float(pd.y - ps.y))*10.0f;
		GUIPoint dir = GUIPoint(long(vdir.x), long(vdir.z));
		GUIPoint pc = (ps + pd)/2;
		points[0] = pc + dir*2;
		points[1] = pc - GUIPoint(dir.y, -dir.x);
		points[2] = pc - GUIPoint(-dir.y, +dir.x);
		points[3] = points[0];
		GUIPoint min = points[0];
		GUIPoint max = points[0];
		for(long j = 1; j < 3; j++)
		{
			if(min.x > points[i].x) min.x = points[i].x;
			if(min.y > points[i].y) min.y = points[i].y;
			if(max.x < points[i].x) max.x = points[i].x;
			if(max.y < points[i].y) max.y = points[i].y;
		}
		if(max.x < 0 || max.y < 0) continue;
		if(min.x > long(opt.width) || min.y > long(opt.height)) continue;
		dword fonColor;
		if(!links[i]->fake)
		{
			fonColor = links[i]->select ? opt.link.defSelBkg : opt.link.defBkg;
		}else{
			fonColor = links[i]->select ? opt.link.fakeSelBkg : opt.link.fakeBkg;
		}
		GUIHelper::DrawPolygon(points, 3, fonColor);
		GUIHelper::DrawLines(points, 4, links[i]->from->select ? opt.link.defActFrm : opt.link.defFrm);
		if(isShowInfo && !links[i]->fake)
		{
			char buffer[256];
			crt_snprintf(buffer, sizeof(buffer), links[i]->data.name.c_str());
			int width = opt.node.fontLabel->GetWidth(buffer);
			int height = opt.node.fontLabel->GetHeight();
			long x = pc.x - width/2;
			long y = pc.y - height/2;
			GUIHelper::Draw2DRect(x - 1, y - 1, width + 2, height + 2, 0xc0000000);
			opt.node.fontLabel->Print(x, y, 0xffffff60, buffer);
		}
	}
}


//Создать линк
long GraphLinks::CreateLink(GraphNodeBase * from, GraphNodeBase * to)
{
	long index = links.Add(NEW GraphLink(opt, from, to));
	return index;
}

//Удалить линк
void GraphLinks::DeleteLink(GraphNodeBase * from, GraphNodeBase * to)
{
	long index = FindLink(from, to);
	if(index < 0) return;
	delete links[index];
	links.DelIndex(index);
}

//Найти линк
long GraphLinks::FindLink(GraphNodeBase * from, GraphNodeBase * to)
{
	for(long i = 0; i < links; i++)
	{
		if(links[i]->from == from && links[i]->to == to) return i;
	}
	return -1;
}

//Найти линк по позиции
long GraphLinks::FindLink(const GUIPoint & pos)
{
	GUIPoint points[4];
	for(long i = links - 1; i >= 0; i--)
	{
		//Позиции линка
		GUIPoint ps = links[i]->from->rect.pos + links[i]->from->rect.size/2;
		GUIPoint pd = links[i]->to->rect.pos + links[i]->to->rect.size/2;
		//Координаты треугольника
		Vector vdir = !Vector(float(pd.x - ps.x), 0.0f, float(pd.y - ps.y))*10.0f;
		GUIPoint dir = GUIPoint(long(vdir.x), long(vdir.z));
		GUIPoint pc = (ps + pd)/2;
		points[0] = pc + dir*2;
		points[1] = pc - GUIPoint(dir.y, -dir.x);
		points[2] = pc - GUIPoint(-dir.y, +dir.x);
		points[3] = points[0];
		//Проверяем попадание точки в треугольник
		for(long j = 0; j < 3; j++)
		{
			//Сторона
			float nx = float(points[j + 1].y - points[j].y);
			float ny = float(-(points[j + 1].x - points[j].x));
			if(nx*pos.x + ny*pos.y > points[j].x*nx + points[j].y*ny) break;
		}
		if(j == 3) return i;
	}
	return -1;
}

//Выделить линк
void GraphLinks::SelectLink(long index, bool deselectCurrent, bool deselectAll)
{
	if(deselectAll)
	{
		for(long i = 0; i < links; i++)
		{
			links[i]->select = false;
		}
	}
	if(index >= 0)
	{	
		GraphLink * l = links[index];
		l->select = !deselectCurrent;
		links.Del(l);
		links.Add(l);		
	}
}

//Записать данные в поток
void GraphLinks::Write(AnxStream & stream)
{
	stream.WriteVersion(1);
	stream << links;
	for(long i = 0; i < links; i++)
	{
		WriteLink(stream, i);
	}
}

//Прочитать данные из потока
void GraphLinks::Read(AnxStream & stream, bool importMode)
{
	if(stream.ReadVersion() != 1) throw "Invalidate links data version";
	long nlinks = 0;
	stream >> nlinks;	
	for(long i = 0; i < nlinks; i++)
	{
		ReadLink(stream, importMode);
	}
}

//Записать данные в поток линки идущие к селекченым нодам
void GraphLinks::WriteSelection(AnxStream & stream)
{
	stream.WriteVersion(1);
	long count = 0;
	for(long i = 0; i < links; i++)
	{
		Assert(links[i]->from);
		Assert(links[i]->to);
		if(links[i]->from->select && links[i]->to->select)
		{
			count++;
		}
	}
	stream << count;
	for(long i = 0; i < links; i++)
	{
		if(links[i]->from->select && links[i]->to->select)
		{
			WriteLink(stream, i);
		}		
	}
}

//Записать данные в поток
void GraphLinks::WriteLink(AnxStream & stream, long index)
{
	stream.EnterWriteSection("Link -> ");
	//Адресаты линка
	stream << links[index]->from->name;
	stream << links[index]->to->name;
	//Данные линка
	links[index]->Write(stream);
	stream.ExitWriteSection();
}

//Прочитать данные из потока
void GraphLinks::ReadLink(AnxStream & stream, bool importMode)
{
	string sid, fromName, toName;
	stream.EnterReadSection(sid);
	GraphLink * link = null;
#ifdef ANX_STREAM_CATCH
	try
	{
#endif
		if(sid != "Link -> ") throw "Invalidate link section id";
		//Адресаты
		stream >> fromName;
		stream >> toName;
		if(importMode)
		{
			ImportTranslate(fromName);
			ImportTranslate(toName);
		}
		GraphNodeBase * from = opt.project->graph.Find(fromName);
		if(!from) throw "Not found source node for link";
		GraphNodeBase * to = opt.project->graph.Find(toName);
		if(!to) throw "Not found destination node for link";
		//Линк
		link = NEW GraphLink(opt, from, to);
		//Данные линка
		link->Read(stream);
		links.Add(link);
		link = null;
#ifdef ANX_STREAM_CATCH
	}catch(const char * err){
		api->Trace("AnxEditor links section: IORead link error: %s", err ? err : "<no info>");
	}catch(...){
		api->Trace("AnxEditor links section: IORead link unknow error.");
	}
#endif
	if(link) delete link;
	stream.ExitReadSection();
}

//Транслирование имени нода при импо
void GraphLinks::ImportTranslate(string & nodeName)
{
	long count = opt.project->graph.importTable;
	for(long i = 0; i < count; i++)
	{
		if(opt.project->graph.importTable[i].oldNodeName == nodeName)
		{
			nodeName = opt.project->graph.importTable[i].newNodeName;
			break;
		}
	}
}


