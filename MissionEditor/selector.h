#ifndef OBJECT_SELECTOR_H_
#define OBJECT_SELECTOR_H_

#include "missioneditor.h"

class ObjectSelector
{

	struct TracedObjects
	{
		MissionEditor::tCreatedMO* mObj;
		MOSafePointer NearMissionObject;
		float IntersectionDistance;
		Vector point;

		Vector bMin;
		Vector bMax;
		Matrix bWorld;

		TracedObjects ()
		{
			IntersectionDistance = 99999999.9f;
			point = Vector (0.0f);
		}
	};


	MOSafePointer LastSelectedObject;
	Vector LastIntersectionPoint;

	Vector r1;
	Vector r2;
	array<TracedObjects> ObjInTrace;

	bool Active;
	IRender * pRS;

	static bool CompareFunc (const ObjectSelector::TracedObjects &item1, const ObjectSelector::TracedObjects &item2);


	void SelectObject (int index);

public:

 ObjectSelector ();
 ~ObjectSelector ();
 
 void Draw ();

 bool GetIsActive ();
 void Activate (bool Active);

 void MouseMove (const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport);

 void ButtonIsPressed (bool bPressed, const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport);

 void ExtractRay (const RENDERVIEWPORT& viewport, const GUIPoint &ptCursor, Vector& raystart, Vector& rayend); 

 void Expand (GUITreeNode* fItem);

};



#endif