//============================================================================================
// Spirenkov Maxim, 2008
//============================================================================================
// FormPolygon
//============================================================================================

#include "..\SndOptions.h"


class FormTestViewer;
class FormPolygonList;


class FormPolygon : public GUIControl
{
public:
	FormPolygon(GUIWindow * parent, GUIRectangle & rect);
	virtual ~FormPolygon();


	FormTestViewer * viewer;
	FormPolygonList * list;
};









