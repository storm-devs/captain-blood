//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// 
//===========================================================================================================================
// GraphNodeIn	
//============================================================================================
			

#include "GraphNodeIn.h"
#include "..\Forms\NodeInOutForm.h"

//============================================================================================

GraphNodeIn::GraphNodeIn(AnxOptions & options) : GraphNodeInOut(options)
{

}

GraphNodeIn::~GraphNodeIn()
{

}


//============================================================================================

//Нарисовать нод
void GraphNodeIn::Draw(const GUIPoint & pos, IRender * render, GUICliper & clipper)
{
	if(!IsView(pos)) return;
	GUIPoint poly[6];
	poly[0].x = rect.x + rect.w; poly[0].y = rect.y;
	poly[1].x = rect.x + rect.w; poly[1].y = rect.y + rect.h;
	poly[2].x = rect.x; poly[2].y = rect.y + rect.h;
	poly[3].x = rect.x + rect.h/2; poly[3].y = rect.y + rect.h/2;
	poly[4].x = rect.x; poly[4].y = rect.y;
	poly[5].x = rect.x + rect.w; poly[5].y = rect.y;
	for(long i = 0; i < 6; i++) poly[i] += pos;
	GUIHelper::DrawPolygon(poly, 5, select ? opt.node.inSelBkg : opt.node.inBkg);
	GUIHelper::DrawLines(poly, 6, opt.node.defFrm);
	if(opt.node.fontLabel)
	{
		long cx = pos.x + rect.pos.x;
		long cy = pos.y + rect.pos.y;
		clipper.SetRectangle(GUIRectangle(cx + rect.h/2, cy, rect.w - rect.h/2, rect.h));
		float x = (float)opt.node.fontLabel->GetWidth(name);
		float h = (float)opt.node.fontLabel->GetHeight();
		x = rect.x + pos.x + rect.h/2 + (rect.w - rect.h/2 - x)*0.5f;
		opt.node.fontLabel->Print((int)x, (int)(rect.y + pos.y + rect.h/2 - h - 1.0f), 0xffffffff, name);
		opt.stmp = "["; opt.stmp += link; opt.stmp += "]";
		x = (float)opt.node.fontLabel->GetWidth(opt.stmp);
		x = rect.x + pos.x + rect.h/2 + (rect.w - rect.h/2 - x)*0.5f;
		opt.node.fontLabel->Print((int)x, (int)(rect.y + pos.y + rect.h/2 + 1.0f), 0xffffffff, opt.stmp);
	}
}

//Проверить поподание в нод
bool GraphNodeIn::Inside(const GUIPoint & pnt)
{
	if(!rect.Inside(pnt)) return false;
	if(pnt.x >= rect.x + rect.h/2) return true;
	if(abs(pnt.y - rect.y - rect.h/2) >= (rect.x + rect.h/2 - pnt.x)) return true;
	return false;
}

//Активировать
void GraphNodeIn::Activate()
{
	Assert(opt.gui_manager);
 	NodeInOutForm * form = NEW NodeInOutForm(this);
	opt.gui_manager->ShowModal(form);
}

//Получить тип нода
AnxNodeTypes GraphNodeIn::GetType()
{
	return anxnt_inode;
}


