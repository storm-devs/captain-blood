#ifndef _ROTATE_CONTROLLER_H_
#define _ROTATE_CONTROLLER_H_


#include "missioneditor.h"

class IFont;
class IRender;
class PositionAttribute;
class RotationAttribute;




class RotateController
{
	Matrix nMatrix;
	float AxisScale;

	Vector newCursorPos;
	Vector  sNormal;
	Vector4 v1;
	Vector4 v2;

	bool bPressed;
	bool Active;
	IFont* p3DFont;
	int Mode;
	IRender * pRS;


	GUIPoint HotSpot2D;
	Line AxisX;
	Line AxisY;
	Line AxisZ;

	Vector pivot;


public:

 RotateController ();
 ~RotateController ();

 void Draw ();

 void Activate (bool Active);

 void MouseMove (const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport);

 bool ButtonIsPressed (bool bPressed, const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport);


 void ExtractRay (const RENDERVIEWPORT& viewport, const GUIPoint &ptCursor, Vector& raystart, Vector& rayend);

 PositionAttribute* GetFirstPO_FromSelectedObject ();
 RotationAttribute* GetFirstRO_FromSelectedObject ();
 MissionEditor::tCreatedMO* GetSelectedObject ();

 
 bool ValidValue (const Vector &vec);

 void ClampAndLimit (float& angle, float minLimit, float maxLimit, bool IsLimit);
};



#endif