//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormPolygon
//============================================================================================


#include "FormPolygon.h"
#include "external\FormTestViewer.h"
#include "lists\FormPolygonList.h"


FormPolygon::FormPolygon(GUIWindow * parent, GUIRectangle & rect) : GUIControl(parent)
{
#ifndef NO_TOOLS
	options->sa->EditEnablePreview(true);
#endif
	SetClientRect(rect);
	SetDrawRect(rect);	
	GUIRectangle r;
	r.x = 5;
	r.y = 5;
	r.w = rect.w - 250;
	r.h = rect.h - 10;
	GUIRectangle rl = r;
	rl.x = rl.x + rl.w;
	rl.w = rect.w - rl.x - 5;
	list = NEW FormPolygonList(this, rl);
	viewer = NEW FormTestViewer(this, r, *list);
	
}

FormPolygon::~FormPolygon()
{

}