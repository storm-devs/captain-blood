//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphNodeHierarchy	
//============================================================================================
			

#include "GraphNodeHierarchy.h"
#include "..\AnxProject.h"
#include "..\Commands\AnxCmd_ChangeHrcNode.h"

//============================================================================================

GraphNodeHierarchy::GraphNodeHierarchy(AnxOptions & options) : GraphNodeBase(options)
{

}

GraphNodeHierarchy::~GraphNodeHierarchy()
{

}


//============================================================================================


//Нарисовать нод
void GraphNodeHierarchy::Draw(const GUIPoint & pos, IRender * render, GUICliper & clipper)
{
	if(!IsView(pos)) return;
	GUIHelper::Draw2DRect(rect.x + pos.x, rect.y + pos.y, rect.w, rect.h, select ? opt.node.hrcSelBkg : opt.node.hrcBkg);
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

//Активировать
void GraphNodeHierarchy::Activate()
{
	Assert(opt.project);
	opt.project->Execute(NEW AnxCmd_ChangeHrcNode(name, opt));
}

//Нормальный линк подходит к ноду или фейковый
bool GraphNodeHierarchy::IsFakeLink()
{
	return true;
}

//Получить тип нода
AnxNodeTypes GraphNodeHierarchy::GetType()
{
	return anxnt_hnode;
}

