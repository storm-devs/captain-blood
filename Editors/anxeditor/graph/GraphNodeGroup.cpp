//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphNodeGroup	
//============================================================================================
			

#include "GraphNodeGroup.h"
#include "..\Forms\NodeGroupForm.h"

//============================================================================================

GraphNodeGroup::GraphNodeGroup(AnxOptions & options) : GraphNodeBase(options), vlink(_FL_)
{
	isLoopLinks = false;
}

GraphNodeGroup::~GraphNodeGroup()
{
	for(long i = 0; i < vlink; i++) delete vlink[i];
}


//============================================================================================

//Нарисовать нод
void GraphNodeGroup::Draw(const GUIPoint & pos, IRender * render, GUICliper & clipper)
{
	if(!IsView(pos)) return;
	GUIPoint poly[5];
	poly[0].x = rect.x; poly[0].y = rect.y + rect.h/2;
	poly[1].x = rect.x + rect.w/2; poly[1].y = rect.y;
	poly[2].x = rect.x + rect.w; poly[2].y = rect.y + rect.h/2;
	poly[3].x = rect.x + rect.w/2; poly[3].y = rect.y + rect.h;
	poly[4].x = rect.x; poly[4].y = rect.y + rect.h/2;
	for(long i = 0; i < 5; i++) poly[i] += pos;
	GUIHelper::DrawPolygon(poly, 4, select ? opt.node.grpSelBkg : opt.node.grpBkg);
	GUIHelper::DrawLines(poly, 5, opt.node.defFrm);
	if(opt.node.fontLabel)
	{
		long cx = pos.x + rect.pos.x;
		long cy = pos.y + rect.pos.y;
		clipper.SetRectangle(GUIRectangle(cx + rect.h/2, cy, rect.w - rect.h, rect.h));
		float x = (float)opt.node.fontLabel->GetWidth(name);
		float y = (float)opt.node.fontLabel->GetHeight();
		x = rect.x + pos.x + rect.h/2 + (rect.w - rect.h - x)*0.5f;
		y = rect.y + pos.y + (rect.h - y)*0.5f;
		opt.node.fontLabel->Print((int)x, (int)y, 0xffffffff, name);
	}
}

//Проверить поподание в нод
bool GraphNodeGroup::Inside(const GUIPoint & pnt)
{
	if(!rect.Inside(pnt)) return false;
	long x = abs(pnt.x - (rect.x + rect.w/2));
	long y = abs(pnt.y - (rect.y + rect.h/2));
	if((rect.h*0.5f - x*float(rect.h)/float(rect.w)) <= y) return false;
	return true;
}

//Активировать
void GraphNodeGroup::Activate()
{
	Assert(opt.gui_manager);
 	NodeGroupForm * form = NEW NodeGroupForm(this);
	opt.gui_manager->ShowModal(form);
}

//Можно отводить линк
bool GraphNodeGroup::CanFromLink()
{
	return false;
}

//Нормальный линк подходит к ноду или фейковый
bool GraphNodeGroup::IsFakeLink()
{
	return true;
}

//Записать данные в поток
void GraphNodeGroup::Write(AnxStream & stream)
{
	GraphNodeBase::Write(stream);
	stream.WriteVersion(2);
	//Количество виртуальных линков
	stream << vlink;
	//Линки
	for(long i = 0; i < vlink; i++)
	{
		stream.EnterWriteSection("vlink -> ");
		vlink[i]->linkData.Write(stream);
		stream << vlink[i]->toNodeName;
		stream.ExitWriteSection();
	}
	stream << isLoopLinks;
}

//Прочитать данные из потока
void GraphNodeGroup::Read(AnxStream & stream, GraphNodeBase * importTo)
{
	GraphNodeBase::Read(stream, importTo);
	if(importTo != this)
	{
		dword ver = stream.ReadVersion();
		if(ver < 1 || ver > 2) throw "Invalidate version of group node data";
		//Количество виртуальных линков
		long vlinks = 0;
		stream >> vlinks;
		for(long i = 0; i < vlink; i++) delete vlink[i];
		vlink.Empty();
		string sid;
		//Линки
		for(i = 0; i < vlinks; i++)
		{
			stream.EnterReadSection(sid);		
			VirtualLink * vl = null;
	#ifdef ANX_STREAM_CATCH
			try
			{
	#endif
				if(sid != "vlink -> ") throw "Invalidate section ID";
				vl = NEW VirtualLink();
				vl->linkData.Read(stream);
				stream >> vl->toNodeName;
				vlink.Add(vl);
				vl = null;
	#ifdef ANX_STREAM_CATCH
			}catch(const char * err){
				api->Trace("AnxEditor node group section: IORead node error: %s", err ? err : "<no info>");
			}catch(...){
				api->Trace("AnxEditor node group section: IORead node unknow error.");
			}
	#endif
			if(vl) delete vl;
			stream.ExitReadSection();
		}
		if(ver > 1)
		{
			stream >> isLoopLinks;
		}
	}
	if(importTo)
	{
		opt.project->graph.translateGroupNodes.Add(name);
	}
}

//Ретранслировать данные при импорте
void GraphNodeGroup::ImportTranslate()
{
	for(long i = 0; i < vlink; i++)
	{
		long count = opt.project->graph.importTable;
		for(long j = 0; j < count; j++)
		{
			if(opt.project->graph.importTable[j].oldNodeName == vlink[i]->toNodeName)
			{
				vlink[i]->toNodeName = opt.project->graph.importTable[j].newNodeName;
			}
		}
	}
}

//Получить тип нода
AnxNodeTypes GraphNodeGroup::GetType()
{
	return anxnt_gnode;
}
