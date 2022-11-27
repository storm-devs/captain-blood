#ifndef _MOVE_CONTROLLER_H_
#define _MOVE_CONTROLLER_H_


#include "missioneditor.h"

class IFont;
class IRender;
class PositionAttribute;



class MoveController
{
	bool bStickyUpOffset;
	bool bStickyMove;
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


	Line AxisX;
	Line AxisY;
	Line AxisZ;

	Vector pivot;

	void ScreenToD3D (int sX, int sY, float &d3dX, float &d3dY);
	void DrawOccluder (int pX, int pY, int width, int height, float depth, unsigned long color);
	IOcclusionQuery * query;


	float GetDepth_BinarySearch(const GUIPoint & pt, float fMin, float fMax, int depth);


public:

 MoveController ();
 ~MoveController ();

 void Draw ();

 void Activate (bool Active);

 void MouseMove (const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport);

 bool ButtonIsPressed (bool bPressed, const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport);


 void ExtractRay (const RENDERVIEWPORT& viewport, const GUIPoint &ptCursor, Vector& raystart, Vector& rayend);

 PositionAttribute* GetFirstPO_FromSelectedObject ();
 MissionEditor::tCreatedMO* GetSelectedObject ();

 
 bool ValidValue (const Vector &vec);


 Vector GetWorldPositionUnderMouse(const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport, double &distToCamera);
};



#endif