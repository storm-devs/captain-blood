#include "selector.h"
#include "boxcol.h"
#include "forms\mainwindow.h"
#include "attributes\AttributeList.h"

extern MissionEditor* sMission;
extern MOSafePointer pCurrentSelected;			// Текущий выбранный объект
extern TMainWindow* MainWindow;
extern Matrix ViewPortProjectionMatrix;

ObjectSelector::ObjectSelector () : ObjInTrace (_FL_)
{
	LastIntersectionPoint = Vector(0.0f);
	Active = false;
  pRS = (IRender*)api->GetService("DX9Render");
	r1 = Vector (0.0f);
	r2 = Vector (0.0f);
}

ObjectSelector::~ObjectSelector ()
{
}
 

bool ObjectSelector::GetIsActive ()
{
	return Active;
}

void ObjectSelector::Draw ()
{
	if (!Active) return;

	//pRS->DrawVector(r1, r2, 0xFFFFFF00);
	//for (int n =0; n < ObjInTrace; n++)
	//{
		//Vector point = ObjInTrace[n].point;
		//float distance = ObjInTrace[n].IntersectionDistance;
		//Color clr(distance, distance, distance);

		//pRS->DrawSphere(point, 0.5f, clr.GetDword());

		//pRS->DrawBox(ObjInTrace[n].bMin, ObjInTrace[n].bMax, ObjInTrace[n].bWorld, 0xFFFFFF00);
	//}
}

void ObjectSelector::Activate (bool Active)
{
	this->Active = Active;
}

void ObjectSelector::MouseMove (const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport)
{
	if (!Active) return;
}


void ObjectSelector::ExtractRay (const RENDERVIEWPORT& viewport, const GUIPoint &ptCursor, Vector& raystart, Vector& rayend)
{
	Matrix matProj = ViewPortProjectionMatrix;
	Vector v;
	v.x =  ( ( ( 2.0f * ptCursor.x ) / viewport.Width  ) - 1 ) / matProj.m[0][0];
	v.y = -( ( ( 2.0f * ptCursor.y ) / viewport.Height ) - 1 ) / matProj.m[1][1];
	v.z =  1.0f;

	Matrix mView = pRS->GetView();
	mView.Inverse ();

	Vector raydir;
	Vector rayorig;
	raydir = mView.MulNormal(v);
	rayorig = mView.pos;

	raystart = rayorig;
	rayend = (rayorig + (raydir * 100.f));
}



bool ObjectSelector::CompareFunc (const ObjectSelector::TracedObjects &item1, const ObjectSelector::TracedObjects &item2)
{
	if (item1.IntersectionDistance < item2.IntersectionDistance) return true;
	return false;
}

void ObjectSelector::ButtonIsPressed (bool bPressed, const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport)
{
	static SlowBoxCollider BoxColl;
	if (!bPressed) return;
	if (!Active) return;
	ObjInTrace.DelAll();
	
	ExtractRay (viewport, ptCursor, r1, r2);

	// Надо протестить луч на пересечение со всеми коробками
	// и выбрать ближайшую

	
	for (DWORD i =0; i < sMission->GetCreatedMissionObjectsCount(); i++)
	{
		MOSafePointer missObj = sMission->GetCreatedMissionObjectByIndex(i);
		
		if (!missObj.SPtr()->EditMode_IsVisible()) continue;

		Vector min(0.0f), max(0.0f);
		missObj.Ptr()->EditMode_GetSelectBox(min, max);
		Matrix matWorld;
		missObj.Ptr()->GetMatrix (matWorld);

		BoxColl.Init(min, max, matWorld);
		float val = BoxColl.RayTrace(r1, r2);
		if (val > 0)
		{
			TracedObjects newEnt;
			newEnt.IntersectionDistance = val;
			newEnt.NearMissionObject = missObj;
			
			newEnt.mObj = &sMission->GetCreatedMissionObjectStructByIndex(i);
			newEnt.point = BoxColl.IntersectionPoint;

			newEnt.bMin = min;
			newEnt.bMax = max;
			newEnt.bWorld = matWorld;


			ObjInTrace.Add(newEnt);
		}
	}


	ObjInTrace.QSort(CompareFunc);

	//api->Trace ("TRACE START ------------------------");
	for (DWORD j = 0; j < ObjInTrace.Size(); j++)
	{
		const char* oName = ObjInTrace[j].NearMissionObject.Ptr()->GetObjectID().c_str();
		float d = ObjInTrace[j].IntersectionDistance;
		//api->Trace("[%d] Obj - %s, Distance - %3.2f", j, oName, d);
	}

	bool ControlState = false;
	if (GetAsyncKeyState(VK_CONTROL) < 0) ControlState = true;


	// simple like Maya selector
	if ((!ControlState) || !LastSelectedObject.Validate())
	{
		if (ObjInTrace.Size() >= 1)
		{
			SelectObject (0);
		}
	} else
		{
			bool NeedLoop = true;
			for (int i = 0; i < ObjInTrace; i++)
			{
				Vector cam_pos = pRS->GetView().GetCamPos();
				float new_dist = Vector(ObjInTrace[i].point - cam_pos).GetLength();
				float old_dist = Vector(LastIntersectionPoint - cam_pos).GetLength();
				if ((new_dist > old_dist) && (LastSelectedObject != ObjInTrace[i].NearMissionObject))
				{
					SelectObject (i);
					NeedLoop = false;
					break;
				}
			}

			if (NeedLoop) SelectObject (0);
		}




}


void ObjectSelector::SelectObject (int index)
{
	if ((int)ObjInTrace.Size() <= index) return;
	ObjInTrace[index].NearMissionObject.SPtr()->EditMode_Select (true);
	if (pCurrentSelected.Validate())	pCurrentSelected.Ptr()->EditMode_Select (false);
	pCurrentSelected = ObjInTrace[index].NearMissionObject;
	LastSelectedObject = ObjInTrace[index].NearMissionObject;
	LastIntersectionPoint = ObjInTrace[index].point;


	MainWindow->TreeView1->ResetSelection(*MainWindow->TreeView1->Items);

	if (ObjInTrace[index].mObj)
	{
		string FullPath = ObjInTrace[index].mObj->PathInTree;
		FullPath += ObjInTrace[index].NearMissionObject.SPtr()->GetObjectID().c_str();

		GUITreeNode* fItem = MainWindow->TreeView1->FindItem (FullPath.GetBuffer ());
		if (fItem)
		{
			if (fItem->Parent) Expand (fItem->Parent);
			MainWindow->TreeView1->SetSelectedNode (fItem, false);
		}
	}
}

void ObjectSelector::Expand (GUITreeNode* fItem)
{
	fItem->Expanded = true;
	if (fItem->Parent) Expand (fItem->Parent);
}