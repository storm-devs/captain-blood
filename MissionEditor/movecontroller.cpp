#include "missioneditor.h"
#include "movecontroller.h"
#include "forms\mainwindow.h"
#include "forms\panel.h"
#include "attributes\AttributeList.h"


#define OFFSET_FROM_ZERO 0.0f
#define AXIS_SIZE 1.00f
#define OFFSET_TO_PRINT 1.25f

#define OFFSET_ADD 0.35f


extern TMainWindow* MainWindow;
extern BaseAttribute* pEditableNode;
extern Matrix ViewPortProjectionMatrix;

MoveController::MoveController ()
{
	bStickyMove = false;
	AxisScale = 1.0f;
	newCursorPos = Vector (0.0f);
	sNormal = Vector (0.0f);
	v1 = Vector (0.0f);
	v2 = Vector (0.0f);
	bPressed = false;
	Mode = DISABLE;
	Active = false;
	Mode = 0;
	pRS = (IRender*)api->GetService("DX9Render");
	p3DFont = pRS->CreateFont("Arial", 18.0f, 0xFFFFFF, "EditorFont");


	query = pRS->CreateOcclusionQuery(_FL_);
}

MoveController::~MoveController ()
{
	if (query)
	{
		query->Release();
	}

	if (p3DFont) p3DFont->Release();
}

void MoveController::Activate (bool Active)
{
	this->Active = Active;
}

MissionEditor::tCreatedMO* MoveController::GetSelectedObject ()
{
	GUITreeNode* sNode = MainWindow->TreeView1->GetSelectedNode ();
	if (sNode == NULL) return NULL;
	if (sNode->bReadOnly) return NULL;

	if (sNode->Tag == TAG_ATTRIBUTE)
	{
		BaseAttribute* pBaseNode = (BaseAttribute*)sNode->Data;
		MissionEditor::tCreatedMO* curNode = (MissionEditor::tCreatedMO*)pBaseNode->GetMasterData ();
		return curNode;
	}

	if (sNode->Tag != TAG_OBJECT) return NULL;
	MissionEditor::tCreatedMO* pMo =  (MissionEditor::tCreatedMO*)sNode->Data;
	if (!pMo->pObject.Validate()) return NULL;
	return pMo;
}


PositionAttribute* MoveController::GetFirstPO_FromSelectedObject ()
{
	GUITreeNode* sNode = MainWindow->TreeView1->GetSelectedNode ();
	if (sNode == NULL) return NULL;
	if (sNode->bReadOnly) return NULL;
	
	if (sNode->Tag == TAG_ATTRIBUTE)
	{
		BaseAttribute* pBaseNode = (BaseAttribute*)sNode->Data;
		if (pBaseNode->GetType() != IMOParams::t_position) return NULL;
		PositionAttribute* pos = (PositionAttribute*)pBaseNode;
		return pos;
	}


	if (sNode->Tag != TAG_OBJECT) return NULL;
	MissionEditor::tCreatedMO* pMo =  (MissionEditor::tCreatedMO*)sNode->Data;
	if (!pMo->pObject.Validate()) return NULL;


	PositionAttribute* finded_attr = NULL;
	for (int n =0; n < pMo->AttrList->GetCount(); n++)
	{
		BaseAttribute* bAttr = pMo->AttrList->Get(n);
		if (bAttr->GetType() == IMOParams::t_position)
		{
			finded_attr = (PositionAttribute*)bAttr;
			return finded_attr;
		}
	}

	return NULL;
}

void MoveController::Draw ()
{
	if (!Active) return;


	PositionAttribute* f_attr = GetFirstPO_FromSelectedObject ();
	if (f_attr == NULL) return;
	Vector cPos;
	cPos = f_attr->GetValue();
	

	bStickyUpOffset = false;
	if (GetAsyncKeyState(VK_SHIFT) < 0)
	{
		if (GetAsyncKeyState(VK_CONTROL) < 0)
		{
			bStickyUpOffset = true;
		}

		if (bStickyUpOffset)
		{
			pRS->DrawXZCircle(cPos - Vector(0.0f, -0.25f, 0.0f), 1.0f, 0xFFFFFF00);
			pRS->DrawXZCircle(cPos - Vector(0.0f, -0.5f, 0.0f), 1.0f, 0xFFFFFF00);
		}

		pRS->DrawXZCircle(cPos, 1.0f, 0xFFFFFF00);
		bStickyMove = true;
		return;
	}

	bStickyMove = false;


	if (!bPressed)
	{
		Vector tobject_pos = cPos * pRS->GetView();
		float fDistance = tobject_pos.z;

//		float fDistance = sqrtf(~(cPos - .GetCamPos()));
		AxisScale = fDistance / 10.0f;
		if (AxisScale < 1.0f) AxisScale = 1.0f;
	}


/*
	Matrix matObj;
	pMo->pObject->GetMatrix(matObj);
	Vector cPos = matObj.pos;
*/
	pRS->SetWorld (Matrix());
	//X
	DWORD color = 0xFFFF0000;
	if (Mode == X_AXIS) color = 0xFFFFFF00;
	pRS->DrawVector (cPos + Vector (OFFSET_FROM_ZERO, 0.0f, 0.0f), cPos + Vector (AXIS_SIZE*AxisScale, 0.0f, 0.0f), color, "EditorLineNoZ");
	p3DFont->SetColor (color);
	p3DFont->Print (cPos + Vector (OFFSET_TO_PRINT*AxisScale, 0.0f, 0.0f), 1000.0f, 0.0f, "X");
	//Y
	color = 0xFF00FF00;
	if (Mode == Y_AXIS) color = 0xFFFFFF00;
	pRS->DrawVector (cPos + Vector (0.0f, OFFSET_FROM_ZERO, 0.0f), cPos + Vector (0.0f, AXIS_SIZE*AxisScale, 0.0f), color, "EditorLineNoZ");
	p3DFont->SetColor (color);
	p3DFont->Print (cPos + Vector (0.0f, OFFSET_TO_PRINT*AxisScale, 0.0f), 1000.0f, 0.0f, "Y");

	//Z
	color = 0xFF0000FF;
	if (Mode == Z_AXIS) color = 0xFFFFFF00;
	pRS->DrawVector (cPos + Vector (0.0f, 0.0f, OFFSET_FROM_ZERO), cPos + Vector (0.0f, 0.0f, AXIS_SIZE*AxisScale), color, "EditorLineNoZ");
	p3DFont->SetColor (color);
	p3DFont->Print (cPos + Vector (0.0f, 0.0f, OFFSET_TO_PRINT*AxisScale), 1000.0f, 0.0f, "Z");
/*
	// additional draw...
	// BLUE
	color = 0xFF0000FF;
	pRS->DrawLine (cPos + Vector (0.0f, 0.0f, OFFSET_ADD), color, cPos + Vector (OFFSET_ADD, 0.0f, OFFSET_ADD), color, false, "EditorLineNoZ");
	pRS->DrawLine (cPos + Vector (0.0f, 0.0f, OFFSET_ADD), color, cPos + Vector (0.0f, OFFSET_ADD, OFFSET_ADD), color, false, "EditorLineNoZ");
	
	// GREEN
	color = 0xFF00FF00;
	pRS->DrawLine (cPos + Vector (0.0f, OFFSET_ADD, 0.0f), color, cPos + Vector (0.0f, OFFSET_ADD, OFFSET_ADD), color, false, "EditorLineNoZ");
	pRS->DrawLine (cPos + Vector (0.0f, OFFSET_ADD, 0.0f), color, cPos + Vector (OFFSET_ADD, OFFSET_ADD, 0.0f), color, false, "EditorLineNoZ");
	
	// RED
	color = 0xFFFF0000;
	pRS->DrawLine (cPos + Vector (OFFSET_ADD, 0.0f, 0.0f), color, cPos + Vector (OFFSET_ADD, 0.0f, OFFSET_ADD), color, false, "EditorLineNoZ");
	pRS->DrawLine (cPos + Vector (OFFSET_ADD, 0.0f, 0.0f), color, cPos + Vector (OFFSET_ADD, OFFSET_ADD, 0.0f), color, false, "EditorLineNoZ");

*/
	AxisX = Line(cPos + Vector (0.0f, 0.0f, 0.0f), cPos + Vector (AXIS_SIZE*AxisScale, 0.0f, 0.0f));
	AxisY = Line(cPos + Vector (0.0f, 0.0f, 0.0f), cPos + Vector (0.0f, AXIS_SIZE*AxisScale, 0.0f));
	AxisZ = Line(cPos + Vector (0.0f, 0.0f, 0.0f), cPos + Vector (0.0f, 0.0f, AXIS_SIZE*AxisScale));


	//pRS->Print(v1.x, v1.y, 0xFFFFFFFF, "X");
	//pRS->Print(v2.x, v2.y, 0xFFFFFFFF, "X");
	//pRS->Print(newCursorPos.x, newCursorPos.y, 0xFFFFFFFF, "O");

	

}

void MoveController::ExtractRay (const RENDERVIEWPORT& viewport, const GUIPoint &ptCursor, Vector& raystart, Vector& rayend)
{
	Matrix matProj = ViewPortProjectionMatrix;;
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


struct RecVertex
{
	Vector p;
	unsigned long color;
};


void MoveController::ScreenToD3D (int sX, int sY, float &d3dX, float &d3dY)
{
	float fScrX = float(pRS->GetScreenInfo3D().dwWidth) / 2.0f;
	float fScrY = float(pRS->GetScreenInfo3D().dwHeight) / 2.0f;

	d3dX = (float)sX / fScrX - 1.0f;
	d3dY = -((float)sY / fScrY - 1.0f);
}

void MoveController::DrawOccluder (int pX, int pY, int width, int height, float depth, unsigned long color)
{
	RecVertex vrx[6];

	Vector From = Vector (0.0f);
	Vector To = Vector (0.0f);

	ScreenToD3D (pX, pY, From.x, From.y);
	ScreenToD3D ((pX+width), (pY+height), To.x, To.y);


	vrx[0].p = From;
	vrx[1].p = Vector (To.x, From.y, 1.0f);
	vrx[2].p = To;

	vrx[3].p = To;
	vrx[4].p = Vector (From.x, To.y, 1.0f);
	vrx[5].p = From;

	for (int r = 0; r < 6; r++)
	{
		vrx[r].color = color;
		vrx[r].p.z = depth;
	}

	ShaderId occRectID;
	pRS->GetShaderId("OccluderRec", occRectID);		
	pRS->DrawPrimitiveUP (occRectID, PT_TRIANGLELIST, 2, vrx, sizeof (RecVertex));
}

float MoveController::GetDepth_BinarySearch(const GUIPoint & pt, float fMin, float fMax, int depth)
{
	float fDepthCenter = fMin + ((fMax - fMin) * 0.5f);

	if (depth > 24)
	{
		return fDepthCenter;
	}

	query->Begin();
	DrawOccluder (pt.x, pt.y, 1, 1, fDepthCenter, 0xFFFFFFFF);
	query->End();
	DWORD dwPixels = query->GetResult();

	if (dwPixels > 0)
	{
		//точка ближе чем нужно
		fMin = fDepthCenter;
		fMax = fMax;
	} else
	{
		//точка дальше чем нужно
		fMin = fMin;
		fMax = fDepthCenter;
	}


	float foundedDepth = GetDepth_BinarySearch(pt, fMin, fMax, depth+1);
	return foundedDepth;
}

Vector MoveController::GetWorldPositionUnderMouse(const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport, double &distToCamera)
{
	RENDERVIEWPORT oldVP = pRS->GetViewport();
	RENDERVIEWPORT newVP = pRS->GetFullScreenViewPort_2D();
	pRS->SetViewport(newVP);

	GUIPoint pt;
	pt.x = oldVP.X + ptCursor.x;
	pt.y = oldVP.Y + ptCursor.y;
	float depth = GetDepth_BinarySearch(pt, 0.0f, 1.0f, 1);




	pRS->SetViewport(oldVP);

	Matrix mtxProjectionInv = pRS->GetProjection();
	mtxProjectionInv.InverseComplette4X4();


	double z = mtxProjectionInv.m[2][2] * depth + mtxProjectionInv.m[3][2];
	double w = mtxProjectionInv.m[2][3] * depth + mtxProjectionInv.m[3][3];

	distToCamera = z / w;


	Vector start, end;
	ExtractRay (viewport, ptCursor, start, end);


	Vector dir = (end - start);
	dir.Normalize();

	Vector pos = start + (dir * distToCamera);


	//pRS->Print(0, 0, 0xFFFFFFFF, "depth %f, dist : %f", depth, distToCamera);

	return pos;
}

void MoveController::MouseMove (const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport)
{

	if (!Active) return;


//--------------------------
	if (!bPressed)
	{
		Vector r1, r2;
		ExtractRay (viewport, ptCursor, r1, r2);
		Line mouse_line(r1, r2);

		//pRS->DrawLine(r1, 0xFFFFFFFF, r2, 0xFFFFFFFF);

		Mode = DISABLE;
		bool result = false;
		result = AxisX.IntersectionLines(mouse_line, pivot, 0.05f*AxisScale);

		float CurDistance = 999999999.0f;
		Vector CamPos = pRS->GetView().GetCamPos();


		if (result)
		{
			if ((pivot.x >= AxisX.p1.x) && (pivot.x <= AxisX.p2.x))
			{
				float cDistnace = (CamPos-pivot).GetLength();
				if (cDistnace < CurDistance)
				{
					Mode = X_AXIS;
					CurDistance = cDistnace;
				}
			}
		}
		result = AxisY.IntersectionLines(mouse_line, pivot, 0.05f*AxisScale);
		if (result)
		{
			if ((pivot.y >= AxisY.p1.y) && (pivot.y <= AxisY.p2.y))	
			{
				float cDistnace = (CamPos-pivot).GetLength();
				if (cDistnace < CurDistance)
				{
					Mode = Y_AXIS;
					CurDistance = cDistnace;
				}
			}
		}
		result = AxisZ.IntersectionLines(mouse_line, pivot, 0.05f*AxisScale);
		if (result)
		{
			if ((pivot.z >= AxisZ.p1.z) && (pivot.z <= AxisZ.p2.z))	
			{
				float cDistnace = (CamPos-pivot).GetLength();
				if (cDistnace < CurDistance)
				{
					Mode = Z_AXIS;
					CurDistance = cDistnace;
				}
			}
		}


		return;
	}
//---------------------------------------------



	Vector mCursor;
	mCursor.x = (float)ptCursor.x;
	mCursor.y = (float)ptCursor.y;
	mCursor.z = 0.0f;

	Vector relCursor = (mCursor - v1.v);

    newCursorPos = v1.v + ((relCursor | sNormal) * sNormal);

	GUIPoint newPosition;
	newPosition.x = (int)newCursorPos.x;
	newPosition.y = (int)newCursorPos.y;

	Vector r1, r2;
	ExtractRay (viewport, newPosition, r1, r2);
	Line mouse_line(r1, r2);

	Vector p;
	if (Mode == X_AXIS)
	{
		AxisX.IntersectionLines(mouse_line, p, 0.05f*AxisScale);
		PositionAttribute* f_attr = GetFirstPO_FromSelectedObject ();
		if (f_attr)
		{
			Vector xvec = f_attr->GetValue();
			float delta = (p.x - pivot.x);
			//if (delta > 100.0f) delta = 100.0f;
			xvec.x += delta;
			if (ValidValue (xvec))
			{
				if (f_attr->GetIsLimit())
				{
					if (xvec.x < f_attr->GetMin().x) xvec.x = f_attr->GetMin().x;
					if (xvec.x > f_attr->GetMax().x) xvec.x = f_attr->GetMax().x;
				}
				f_attr->SetValue(xvec);
				pivot = p;
				MissionEditor::tCreatedMO* curNode = GetSelectedObject ();
				MOPWriter wrt(curNode->Level, curNode->pObject.SPtr()->GetObjectID().c_str());
				curNode->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
				miss->EditorUpdateObject(curNode->pObject.Ptr(), wrt);
#endif
				//curNode->pObject->EditMode_Update (wrt.Reader ());
			}
		}
	}

	if (Mode == Y_AXIS)
	{
		AxisY.IntersectionLines(mouse_line, p, 0.05f*AxisScale);
		PositionAttribute* f_attr = GetFirstPO_FromSelectedObject ();
		if (f_attr)
		{
			Vector xvec = f_attr->GetValue();
			float delta = (p.y - pivot.y);
			//if (delta > 100.0f) delta = 100.0f;
			xvec.y += delta;
			if (ValidValue (xvec))
			{
				if (f_attr->GetIsLimit())
				{
					if (xvec.y < f_attr->GetMin().y) xvec.y = f_attr->GetMin().y;
					if (xvec.y > f_attr->GetMax().y) xvec.y = f_attr->GetMax().y;
				}

				f_attr->SetValue(xvec);
				pivot = p;
				MissionEditor::tCreatedMO* curNode = GetSelectedObject ();
				MOPWriter wrt(curNode->Level, curNode->pObject.Ptr()->GetObjectID().c_str());
				curNode->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
				miss->EditorUpdateObject(curNode->pObject.Ptr(), wrt);
#endif
				//curNode->pObject->EditMode_Update (wrt.Reader ());
			}
		}
	}

	if (Mode == Z_AXIS)
	{
		AxisZ.IntersectionLines(mouse_line, p, 0.05f*AxisScale);
		PositionAttribute* f_attr = GetFirstPO_FromSelectedObject ();
		if (f_attr)
		{
			Vector xvec = f_attr->GetValue();
			float delta = (p.z - pivot.z);
			//if (delta > 100.0f) delta = 100.0f;
			xvec.z += delta;
			if (ValidValue (xvec))
			{
				if (f_attr->GetIsLimit())
				{
					if (xvec.z < f_attr->GetMin().z) xvec.z = f_attr->GetMin().z;
					if (xvec.z > f_attr->GetMax().z) xvec.z = f_attr->GetMax().z;
				}

				f_attr->SetValue(xvec);
				pivot = p;
				MissionEditor::tCreatedMO* curNode = GetSelectedObject ();
				MOPWriter wrt(curNode->Level, curNode->pObject.Ptr()->GetObjectID().c_str());
				curNode->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
				miss->EditorUpdateObject(curNode->pObject.Ptr(), wrt);
#endif
				//curNode->pObject->EditMode_Update (wrt.Reader ());

			}
			
		}
	}

//---------------------------------------------

	
	
}

bool MoveController::ButtonIsPressed (bool bPressed, const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport)
{
	if (!Active) return true;

	//"Липкая" таскалка с шифтом
	if (bStickyMove && bPressed == true)
	{
		double distToCam = 0;
		Vector wp = GetWorldPositionUnderMouse (ptCursor, viewport, distToCam);

		if (bStickyUpOffset)
		{
			wp.y += 0.5f;
		}

		//слишком далеко не ставим
		if (distToCam < 1500.0f)
		{
			PositionAttribute* f_attr = GetFirstPO_FromSelectedObject ();
			if (f_attr)
			{
				Vector xvec = wp;
				if (ValidValue (xvec))
				{
					if (f_attr->GetIsLimit())
					{
						if (xvec.x < f_attr->GetMin().x) xvec.x = f_attr->GetMin().x;
						if (xvec.x > f_attr->GetMax().x) xvec.x = f_attr->GetMax().x;

						if (xvec.y < f_attr->GetMin().y) xvec.y = f_attr->GetMin().y;
						if (xvec.y > f_attr->GetMax().y) xvec.y = f_attr->GetMax().y;

						if (xvec.z < f_attr->GetMin().z) xvec.z = f_attr->GetMin().z;
						if (xvec.z > f_attr->GetMax().z) xvec.z = f_attr->GetMax().z;

					}

					f_attr->SetValue(xvec);

					MissionEditor::tCreatedMO* curNode = GetSelectedObject ();
					MOPWriter wrt(curNode->Level, curNode->pObject.SPtr()->GetObjectID().c_str());
					curNode->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
					miss->EditorUpdateObject(curNode->pObject.Ptr(), wrt);
#endif
				}
			}
			//pRS->DrawSphere(wp, 0.1f, 0xFFFFFFFF);
		}

		return true;
	}



	if (bPressed == false)
	{
		PositionAttribute* f_attr = GetFirstPO_FromSelectedObject ();
		if (f_attr)
		{
			pEditableNode = f_attr;
			//MainWindow->UpdateTree(NULL);
			GUITreeNode* sNode = MainWindow->TreeView1->GetSelectedNode ();
			if (sNode->Tag == TAG_ATTRIBUTE)
			{
				BaseAttribute* pBaseNode = (BaseAttribute*)sNode->Data;
				pBaseNode->UpdateTree(sNode);
			}

		}
	}

	if (Mode == DISABLE) return false;



	if (Mode == X_AXIS)
	{
		Matrix mVP(pRS->GetView(), ViewPortProjectionMatrix);
		v1 = mVP.Projection(AxisX.p1, viewport.Width * 0.5f, viewport.Height * 0.5f);
		v2 = mVP.Projection(AxisX.p2, viewport.Width * 0.5f, viewport.Height * 0.5f);
	}

	if (Mode == Y_AXIS)
	{
		Matrix mVP(pRS->GetView(), ViewPortProjectionMatrix);
		v1 = mVP.Projection(AxisY.p1, viewport.Width * 0.5f, viewport.Height * 0.5f);
		v2 = mVP.Projection(AxisY.p2, viewport.Width * 0.5f, viewport.Height * 0.5f);
	}

	if (Mode == Z_AXIS)
	{
		Matrix mVP(pRS->GetView(), ViewPortProjectionMatrix);
		v1 = mVP.Projection(AxisZ.p1, viewport.Width * 0.5f, viewport.Height * 0.5f);
		v2 = mVP.Projection(AxisZ.p2, viewport.Width * 0.5f, viewport.Height * 0.5f);
	}
	

	sNormal = !(v2.v - v1.v);
	sNormal.z = 0.0f;
	this->bPressed = bPressed;
	return true;
}

bool MoveController::ValidValue (const Vector &vec)
{
	//float fDistance = sqrtf(~(vec - pRS->GetView().GetCamPos()));
	//if (fDistance < 0.01f) return false;
	//if (fDistance > 100.0f) return false;

	return true;

	
}