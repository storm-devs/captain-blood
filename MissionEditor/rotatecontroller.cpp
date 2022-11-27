#include "missioneditor.h"
#include "rotatecontroller.h"
#include "forms\mainwindow.h"
#include "forms\panel.h"
#include "attributes\AttributeList.h"


#define OFFSET_FROM_ZERO 0.0f
#define AXIS_SIZE 1.00f
#define OFFSET_TO_PRINT 1.25f

#define OFFSET_ADD 0.35f



extern BaseAttribute* pEditableNode;
extern TMainWindow* MainWindow;

extern Matrix ViewPortProjectionMatrix;


RotateController::RotateController ()
{
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
}

RotateController::~RotateController ()
{
	if (p3DFont) p3DFont->Release();
}

void RotateController::Activate (bool Active)
{
	this->Active = Active;
}

MissionEditor::tCreatedMO* RotateController::GetSelectedObject ()
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


RotationAttribute* RotateController::GetFirstRO_FromSelectedObject ()
{
	GUITreeNode* sNode = MainWindow->TreeView1->GetSelectedNode ();
	if (sNode == NULL) return NULL;
	if (sNode->bReadOnly) return NULL;

	if (sNode->Tag == TAG_ATTRIBUTE)
	{
		BaseAttribute* pBaseNode = (BaseAttribute*)sNode->Data;
		if (pBaseNode->GetType() != IMOParams::t_angles) return NULL;
		RotationAttribute* rot = (RotationAttribute*)pBaseNode;
		return rot;
	}


	if (sNode->Tag != TAG_OBJECT) return NULL;
	MissionEditor::tCreatedMO* pMo =  (MissionEditor::tCreatedMO*)sNode->Data;
	if (!pMo->pObject.Validate()) return NULL;


	RotationAttribute* finded_attr = NULL;
	for (int n =0; n < pMo->AttrList->GetCount(); n++)
	{
		BaseAttribute* bAttr = pMo->AttrList->Get(n);
		if (bAttr->GetType() == IMOParams::t_angles)
		{
			finded_attr = (RotationAttribute*)bAttr;
			return finded_attr;
		}
	}

	return NULL;
}

void RotateController::Draw ()
{
	if (!Active) return;

	RotationAttribute* f_attr = GetFirstRO_FromSelectedObject ();
	if (f_attr == NULL) return;
	Vector cPos, cAngles;
	cAngles = f_attr->GetValue();
	PositionAttribute* f_posattr = GetFirstPO_FromSelectedObject ();
	if (f_posattr) 
		cPos = f_posattr->GetValue();
	else	
		cPos = Vector(0.0f);

	nMatrix = Matrix().Build (cAngles);
	nMatrix.pos = cPos;

	//pRS->Print(0, 0, 0xFFFFFFFF, "cPos.x")
	

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
	pRS->DrawVector (Vector (OFFSET_FROM_ZERO, 0.0f, 0.0f)*nMatrix, Vector (AXIS_SIZE*AxisScale, 0.0f, 0.0f)*nMatrix, color, "EditorLineNoZ");
	p3DFont->SetColor (color);
	p3DFont->Print (Vector (OFFSET_TO_PRINT*AxisScale, 0.0f, 0.0f)*nMatrix, 1000.0f, 0.0f, "X");
	//Y
	color = 0xFF00FF00;
	if (Mode == Y_AXIS) color = 0xFFFFFF00;
	pRS->DrawVector (Vector (0.0f, OFFSET_FROM_ZERO, 0.0f)*nMatrix, Vector (0.0f, AXIS_SIZE*AxisScale, 0.0f)*nMatrix, color, "EditorLineNoZ");
	p3DFont->SetColor (color);
	p3DFont->Print (Vector (0.0f, OFFSET_TO_PRINT*AxisScale, 0.0f)*nMatrix, 1000.0f, 0.0f, "Y");

	//Z
	color = 0xFF0000FF;
	if (Mode == Z_AXIS) color = 0xFFFFFF00;
	pRS->DrawVector (Vector (0.0f, 0.0f, OFFSET_FROM_ZERO)*nMatrix, Vector (0.0f, 0.0f, AXIS_SIZE*AxisScale)*nMatrix, color, "EditorLineNoZ");
	p3DFont->SetColor (color);
	p3DFont->Print (Vector (0.0f, 0.0f, OFFSET_TO_PRINT*AxisScale)*nMatrix, 1000.0f, 0.0f, "Z");
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
	AxisX = Line(Vector (0.0f, 0.0f, 0.0f)*nMatrix,  Vector (AXIS_SIZE*AxisScale, 0.0f, 0.0f)*nMatrix);
	AxisY = Line(Vector (0.0f, 0.0f, 0.0f)*nMatrix,  Vector (0.0f, AXIS_SIZE*AxisScale, 0.0f)*nMatrix);
	AxisZ = Line(Vector (0.0f, 0.0f, 0.0f)*nMatrix,  Vector (0.0f, 0.0f, AXIS_SIZE*AxisScale)*nMatrix);


	//pRS->Print(v1.x, v1.y, 0xFFFFFFFF, "X");
	//pRS->Print(v2.x, v2.y, 0xFFFFFFFF, "X");
	//pRS->Print(newCursorPos.x, newCursorPos.y, 0xFFFFFFFF, "O");

	

}

void RotateController::ExtractRay (const RENDERVIEWPORT& viewport, const GUIPoint &ptCursor, Vector& raystart, Vector& rayend)
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

void RotateController::MouseMove (const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport)
{
	if (!Active) return;
//--------------------------
	if (!bPressed)
	{
		Vector r1, r2;
		ExtractRay (viewport, ptCursor, r1, r2);
		Line mouse_line(r1, r2);
		Mode = DISABLE;

		Matrix nMatrixInv = nMatrix;
		nMatrixInv.Inverse();
		bool result = false;

		float CurDistance = 999999999.0f;
		Vector CamPos = pRS->GetView().GetCamPos();

		
		result = AxisX.IntersectionLines(mouse_line, pivot, 0.05f*AxisScale);
		if (result)
		{	
			Vector nPivot = pivot * nMatrixInv;
			Vector nAxis1 = AxisX.p1 * nMatrixInv;
			Vector nAxis2 = AxisX.p2 * nMatrixInv;
			if ((nPivot.x >= nAxis1.x) && (nPivot.x <= nAxis2.x))
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
			Vector nPivot = pivot * nMatrixInv;
			Vector nAxis1 = AxisY.p1 * nMatrixInv;
			Vector nAxis2 = AxisY.p2 * nMatrixInv;
			if ((nPivot.y >= nAxis1.y) && (nPivot.y <= nAxis2.y))
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
			Vector nPivot = pivot * nMatrixInv;
			Vector nAxis1 = AxisZ.p1 * nMatrixInv;
			Vector nAxis2 = AxisZ.p2 * nMatrixInv;
			if ((nPivot.z >= nAxis1.z) && (nPivot.z <= nAxis2.z))
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

	//Vector relCursor = (mCursor - v1.v);

	//Vector flipedNormal = sNormal;
	//flipedNormal.x = sNormal.y;
	//flipedNormal.y = -sNormal.x;


  //newCursorPos = v1.v + ((relCursor | flipedNormal) * flipedNormal);

  //float dx = (float)HotSpot2D.x - newCursorPos.x;
  //float dy = (float)HotSpot2D.x - newCursorPos.y;

	float dist = (float)ptCursor.x - (float)HotSpot2D.x;
	//float dist = -dx;
	dist *= -0.01f;

	RotationAttribute* r_attr = GetFirstRO_FromSelectedObject();


	//Max
	if(!r_attr) return;


//*********
	Vector cAngles;
	cAngles = r_attr->GetValue();
	Matrix OriginalMatrix = Matrix().Build (cAngles);
	Matrix DeltaMatrix;



	if (Mode == X_AXIS)
	{
		Vector angles = Vector (0.0f);
		angles.x = dist;
		DeltaMatrix.Build(angles);
	}

	if (Mode == Y_AXIS)
	{
		Vector angles = Vector (0.0f);
		angles.y = dist;
		DeltaMatrix.Build(angles);
	}

	if (Mode == Z_AXIS)
	{
		Vector angles = Vector (0.0f);
		angles.z = dist;
		DeltaMatrix.Build(angles);
	}

	Vector realangles;
	Matrix RealRotate = DeltaMatrix * OriginalMatrix;
	RealRotate.GetAngles (realangles.x, realangles.y, realangles.z);

	ClampAndLimit (realangles.x, r_attr->GetMin().x, r_attr->GetMax().x, r_attr->GetIsLimit());
	ClampAndLimit (realangles.y, r_attr->GetMin().y, r_attr->GetMax().y, r_attr->GetIsLimit());
	ClampAndLimit (realangles.z, r_attr->GetMin().z, r_attr->GetMax().z, r_attr->GetIsLimit());
	r_attr->SetValue(realangles);


	MissionEditor::tCreatedMO* curNode = GetSelectedObject ();
	MOPWriter wrt(curNode->Level, curNode->pObject.SPtr()->GetObjectID().c_str());
	curNode->AttrList->AddToWriter (wrt);
#ifndef NO_TOOLS
	miss->EditorUpdateObject(curNode->pObject.SPtr(), wrt);
#endif
//	curNode->pObject->EditMode_Update (wrt.Reader ());

	
	HotSpot2D = ptCursor;
	//HotSpot2D.y = (int)newCursorPos.y;



//---------------------------------------------
	
	
}

bool RotateController::ButtonIsPressed (bool bPressed, const GUIPoint &ptCursor, const RENDERVIEWPORT& viewport)
{
	if (!Active) return true;

	if (bPressed == false)
	{
		RotationAttribute* r_attr = GetFirstRO_FromSelectedObject();
		if (r_attr)
		{
			pEditableNode = r_attr;
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
	

	HotSpot2D = ptCursor;
	sNormal = !(v2.v - v1.v);
	sNormal.z = 0.0f;
	this->bPressed = bPressed;
	return true;
}

bool RotateController::ValidValue (const Vector &vec)
{
	float fDistance = sqrtf(~(vec - pRS->GetView().GetCamPos()));
	if (fDistance < 0.01f) return false;
	if (fDistance > 100.0f) return false;

	return true;

	
}

void RotateController::ClampAndLimit (float& angle, float minLimit, float maxLimit, bool IsLimit)
{
	float DegAng = Rad2Deg(angle);

	// clamp angles
	int cX = (int)DegAng / 360;
	float ostX = (DegAng  - (cX * 360));
	if (ostX < 0) ostX = 360.0f + ostX;
// clamp angles

	DegAng = ostX;
	angle = Deg2Rad (DegAng);

	
	if (IsLimit)
	{
		if (angle < minLimit) angle = minLimit;
		if (angle > maxLimit) angle = maxLimit;
	}
	
}


PositionAttribute* RotateController::GetFirstPO_FromSelectedObject ()
{
	GUITreeNode* sNode = MainWindow->TreeView1->GetSelectedNode ();
	if (sNode == NULL) return NULL;

	if (sNode->Tag == TAG_ATTRIBUTE)
	{
		int my_index = -1;
		for (int n = 0; n < sNode->Parent->Childs.GetCount(); n++)
		{
			if (sNode->Parent->Childs[n] == sNode)
			{
				my_index = n;
				break;
			}
		}
		if (my_index == -1) return NULL;

		for (int i = my_index; i >= 0; i--)
		{
			if (sNode->Parent->Childs[i]->Tag == TAG_ATTRIBUTE)
			{
				// Узел который мы проверяем являеться аттрибутом...
				BaseAttribute* pBaseNode = (BaseAttribute*)sNode->Parent->Childs[i]->Data;
				if (pBaseNode->GetType() == IMOParams::t_position)
				{
					PositionAttribute* pos = (PositionAttribute*)pBaseNode;
					return pos;
				}
			}
		}

		return NULL;
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
