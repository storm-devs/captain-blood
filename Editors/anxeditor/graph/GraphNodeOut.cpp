//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphNodeOut	
//============================================================================================
			

#include "GraphNodeOut.h"
#include "..\Forms\NodeInOutForm.h"

//============================================================================================

GraphNodeOut::GraphNodeOut(AnxOptions & options) : GraphNodeInOut(options)
{

}

GraphNodeOut::~GraphNodeOut()
{

}


//============================================================================================

//Нарисовать нод
void GraphNodeOut::Draw(const GUIPoint & pos, IRender * render, GUICliper & clipper)
{
	if(!IsView(pos)) return;
	GUIPoint poly[6];
	poly[0].x = rect.x; poly[0].y = rect.y;
	poly[1].x = rect.x + rect.w - rect.h/2; poly[1].y = rect.y;
	poly[2].x = rect.x + rect.w; poly[2].y = rect.y + rect.h/2;
	poly[3].x = rect.x + rect.w - rect.h/2; poly[3].y = rect.y + rect.h;
	poly[4].x = rect.x; poly[4].y = rect.y + rect.h;
	poly[5].x = rect.x; poly[5].y = rect.y;
	for(long i = 0; i < 6; i++) poly[i] += pos;
	GUIHelper::DrawPolygon(poly, 5, select ? opt.node.outSelBkg : opt.node.outBkg);
	GUIHelper::DrawLines(poly, 6, opt.node.defFrm);
	if(opt.node.fontLabel)
	{
		long cx = pos.x + rect.pos.x;
		long cy = pos.y + rect.pos.y;
		clipper.SetRectangle(GUIRectangle(cx + rect.h/2, cy, rect.w - rect.h/2, rect.h));
		float x = (float)opt.node.fontLabel->GetWidth(name);
		float h = (float)opt.node.fontLabel->GetHeight();
		x = rect.x + pos.x + (rect.w - rect.h/2 - x)*0.5f;
		opt.node.fontLabel->Print((int)x, (int)(rect.y + pos.y + rect.h/2 - h - 1.0f), 0xffffffff, name);
		opt.stmp = "["; opt.stmp += link; opt.stmp += "]";
		x = (float)opt.node.fontLabel->GetWidth(opt.stmp);
		x = rect.x + pos.x + (rect.w - rect.h/2 - x)*0.5f;
		opt.node.fontLabel->Print((int)x, (int)(rect.y + pos.y + rect.h/2 + 1.0f), 0xffffffff, opt.stmp);
	}
}

//Проверить поподание в нод
bool GraphNodeOut::Inside(const GUIPoint & pnt)
{
	if(!rect.Inside(pnt)) return false;
	if(pnt.x <= rect.x + rect.w - rect.h/2) return true;
	if(abs(pnt.y - rect.y - rect.h/2) <= (rect.x + rect.w - pnt.x)) return true;
	return false;
}

//Активировать
void GraphNodeOut::Activate()
{
	Assert(opt.gui_manager);
 	NodeInOutForm * form = NEW NodeInOutForm(this);
	opt.gui_manager->ShowModal(form);
}

//Получить тип нода
AnxNodeTypes GraphNodeOut::GetType()
{
	return anxnt_onode;
}


