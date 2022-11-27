#include "gizmo.h"
#include "..\..\..\common_h\gui.h"

DWORD RGDTransformGizmo::OriginalAxisColors[3] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF};
DWORD RGDTransformGizmo::AxisColors[3] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF};

#define X_AXIS_INDEX 0
#define Y_AXIS_INDEX 1
#define Z_AXIS_INDEX 2

RGDTransformGizmo::RGDTransformGizmo(GizmoType _type, GizmoMode _mode, const Matrix& _matTransform) : XAxis(_FL_, 256),
																																																YAxis(_FL_, 256),
																																																ZAxis(_FL_, 256)
{
	bEnabled = true;
	GizmoScale = 1.0f;
	fDeltaMove = 0.0f;
	SelectedAxis = GA_AXIS_NONE;
	pRS = (IRender*)api->GetService("DX9Render");

	pConsole = (IConsole*)api->GetService("Console");

	HighlightedAxis = GA_AXIS_NONE;

	mType = _type;
	mMode = _mode;
	mTransform = _matTransform;
	
	bMousePressed = false;

	GizmoviewPort.MinZ = 0.0f;
	GizmoviewPort.MaxZ = 1.0f;
	GizmoviewPort.X = 0;
	GizmoviewPort.Y = 0;
	GizmoviewPort.Width = pRS->GetScreenInfo3D().dwWidth;
	GizmoviewPort.Height = pRS->GetScreenInfo3D().dwHeight;

	bHighlighted=false;
}

RGDTransformGizmo::~RGDTransformGizmo()
{
}

void RGDTransformGizmo::Draw ()
{
	if (!bEnabled) return;
	switch (mType)
	{
	case GT_MOVE:
		DrawMoveGizmo();
		break;
	case GT_ROTATE:
		DrawRotateGizmo();
		break;
	}
}

void RGDTransformGizmo::PushLine (GizmoAxis mode, const Vector& v1, const Vector& v2)
{
	switch(mode)
	{
		case GA_AXIS_X:
			XAxis.Add(Segment(v1, v2));
			break;
		case GA_AXIS_Y:
			YAxis.Add(Segment(v1, v2));
			break;
		case GA_AXIS_Z:
			ZAxis.Add(Segment(v1, v2));
			break;
	}
}



void RGDTransformGizmo::CalcGizmoScale ()
{
	Vector vPositionInCamera = mTransform.pos * pRS->GetView();
	float fDistance = vPositionInCamera.z;
	GizmoScale = fDistance / 20.0f;
	
	//if (GizmoScale < 1.0f) GizmoScale = 1.0f;
}

void RGDTransformGizmo::DrawMoveGizmo ()
{
	if (!bMousePressed) CalcGizmoScale();


	XAxis.DelAll();
	YAxis.DelAll();
	ZAxis.DelAll();


	Vector mFrom = mTransform.pos;

	float fAxisScale = 2.0f * GizmoScale;

	Vector vXAxis = Vector (fAxisScale, 0.0f, 0.0f);
	Vector vYAxis = Vector (0.0f, fAxisScale, 0.0f);
	Vector vZAxis = Vector (0.0f, 0.0f, fAxisScale);

	if (mMode == RGDTransformGizmo::GM_LOCAL)
	{
		Matrix wrld = mTransform;
		vXAxis = vXAxis * wrld;
		vYAxis = vYAxis * wrld;
		vZAxis = vZAxis * wrld;
	} else
	{
		vXAxis += mTransform.pos;
		vYAxis += mTransform.pos;
		vZAxis += mTransform.pos;
	}

	pRS->DrawVector(mFrom, vXAxis, AxisColors[X_AXIS_INDEX], "GizmoLineNoZ");
	pRS->DrawVector(mFrom, vYAxis, AxisColors[Y_AXIS_INDEX], "GizmoLineNoZ");
	pRS->DrawVector(mFrom, vZAxis, AxisColors[Z_AXIS_INDEX], "GizmoLineNoZ");

	//pRS->DrawLine(mFrom, AxisColors[X_AXIS_INDEX], vXAxis, AxisColors[X_AXIS_INDEX], false, "EditorLineNoZ");
	//pRS->DrawLine(mFrom, AxisColors[Y_AXIS_INDEX], vYAxis, AxisColors[Y_AXIS_INDEX], false, "EditorLineNoZ");
	//pRS->DrawLine(mFrom, AxisColors[Z_AXIS_INDEX], vZAxis, AxisColors[Z_AXIS_INDEX], false, "EditorLineNoZ");

	PushLine(GA_AXIS_X, mFrom, vXAxis);
	PushLine(GA_AXIS_Y, mFrom, vYAxis);
	PushLine(GA_AXIS_Z, mFrom, vZAxis);
}

bool RGDTransformGizmo::NeedDrawRotateGizmoLine (const Vector &v1, const Vector &v2, const Vector& vCamDir)
{
	Vector vCenter = mTransform.pos;

	Vector dir1 = v1 - vCenter;
	Vector dir2 = v1 - vCenter;

	if ((vCamDir | dir1) > 0) return false;
	if ((vCamDir | dir2) > 0) return false;

	return true;
}

void RGDTransformGizmo::DrawRotateGizmo ()
{
	if (!bMousePressed) CalcGizmoScale();

	Matrix mCurView = pRS->GetView();
	mCurView.Inverse();
	Vector vCamDir = mCurView.vz;


	XAxis.DelAll();
	YAxis.DelAll();
	ZAxis.DelAll();


	Matrix wrld = Matrix();
	if (mMode == RGDTransformGizmo::GM_LOCAL)
	{
		wrld = mTransform;
		wrld.pos = 0.0f;
	} 

	float fRadius = 2.0f * GizmoScale;
	float fDelta = 0.04f;


	Vector vStart;
	Vector vEnd;

	Vector mFrom = mTransform.pos;


	Vector vStartPoint = Vector(sinf(0)*fRadius, cosf(0)*fRadius, 0.0f);


	pRS->FlushBufferedLines();

	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(sinf(Angle)*fRadius, cosf(Angle)*fRadius, 0.0f);

		vStart = vStartPoint*wrld;
		vEnd = vPoint*wrld;

		vStart += mFrom;
		vEnd += mFrom;

		if (NeedDrawRotateGizmoLine(vStart, vEnd, vCamDir))
		{
			//pRS->DrawBufferedLine(vStart, AxisColors[Z_AXIS_INDEX], vEnd, AxisColors[Z_AXIS_INDEX]);
			pRS->DrawLine(vStart, AxisColors[Z_AXIS_INDEX], vEnd, AxisColors[Z_AXIS_INDEX], false, "EditorLineNoZ");
			PushLine(GA_AXIS_Z, vStart, vEnd);
		}

		
		vStartPoint = vPoint;
	}	

	vStart = vStartPoint*wrld;
	vEnd = Vector(sinf(0)*fRadius, cosf(0)*fRadius, 0.0f)*wrld;
	vStart += mFrom;
	vEnd += mFrom;
	if (NeedDrawRotateGizmoLine(vStart, vEnd, vCamDir))
	{
		//pRS->DrawBufferedLine(vStart, AxisColors[Z_AXIS_INDEX], vEnd, AxisColors[Z_AXIS_INDEX]);
		pRS->DrawLine(vStart, AxisColors[Z_AXIS_INDEX], vEnd, AxisColors[Z_AXIS_INDEX], false, "EditorLineNoZ");

		PushLine(GA_AXIS_Z, vStart, vEnd);
	}
	



	vStartPoint = Vector(0.0f, cosf(0)*fRadius, sinf(0)*fRadius);
	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(0.0f, cosf(Angle)*fRadius, sinf(Angle)*fRadius);
		vStart = vStartPoint*wrld;
		vEnd = vPoint*wrld;
		vStart += mFrom;
		vEnd += mFrom;
		
		if (NeedDrawRotateGizmoLine(vStart, vEnd, vCamDir))
		{
			//pRS->DrawBufferedLine(vStart, AxisColors[X_AXIS_INDEX], vEnd, AxisColors[X_AXIS_INDEX]);
			pRS->DrawLine(vStart, AxisColors[X_AXIS_INDEX], vEnd, AxisColors[X_AXIS_INDEX], false, "EditorLineNoZ");

			PushLine(GA_AXIS_X, vEnd, vStart);
		}
		
		vStartPoint = vPoint;
	}

	vStart = vStartPoint*wrld;
	vEnd = Vector(0.0f, cosf(0)*fRadius, sinf(0)*fRadius)*wrld;
	vStart += mFrom;
	vEnd += mFrom;
	if (NeedDrawRotateGizmoLine(vStart, vEnd, vCamDir))
	{
		//pRS->DrawBufferedLine(vStart, AxisColors[X_AXIS_INDEX], vEnd, AxisColors[X_AXIS_INDEX]);
		pRS->DrawLine(vStart, AxisColors[X_AXIS_INDEX], vEnd, AxisColors[X_AXIS_INDEX], false, "EditorLineNoZ");

		PushLine(GA_AXIS_X, vStart, vEnd);
	}
	



	vStartPoint = Vector(cosf(0)*fRadius, 0.0f, sinf(0)*fRadius);
	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(cosf(Angle)*fRadius, 0.0f, sinf(Angle)*fRadius);

		vStart = vStartPoint*wrld;
		vEnd = vPoint*wrld;
		vStart += mFrom;
		vEnd += mFrom;
		if (NeedDrawRotateGizmoLine(vStart, vEnd, vCamDir))
		{
			//pRS->DrawBufferedLine(vStart, AxisColors[Y_AXIS_INDEX], vEnd, AxisColors[Y_AXIS_INDEX]);
			pRS->DrawLine(vStart, AxisColors[Y_AXIS_INDEX], vEnd, AxisColors[Y_AXIS_INDEX], false, "EditorLineNoZ");

			PushLine(GA_AXIS_Y, vStart, vEnd);
		}
		
		vStartPoint = vPoint;
	}

	vStart = vStartPoint*wrld;
	vEnd = Vector(cosf(0)*fRadius, 0.0f, sinf(0)*fRadius)*wrld;
	vStart += mFrom;
	vEnd += mFrom;

	if (NeedDrawRotateGizmoLine(vStart, vEnd, vCamDir))
	{
		//pRS->DrawBufferedLine(vStart, AxisColors[Y_AXIS_INDEX], vEnd, AxisColors[Y_AXIS_INDEX]);
		pRS->DrawLine(vStart, AxisColors[Y_AXIS_INDEX], vEnd, AxisColors[Y_AXIS_INDEX], false, "EditorLineNoZ");

		PushLine(GA_AXIS_Y, vStart, vEnd);
	}
	




	Matrix mView = pRS->GetView();
	mView.pos = 0.0f;
	mView.Inverse();
	DWORD dwColor = 0xFF424242;
	vStartPoint = Vector(sinf(0)*fRadius, cosf(0)*fRadius, 0.0f);
	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(sinf(Angle)*fRadius, cosf(Angle)*fRadius, 0.0f);

		vStart = vStartPoint*mView;
		vEnd = vPoint*mView;
		vStart += mFrom;
		vEnd += mFrom;

		//pRS->DrawBufferedLine(vStart, dwColor, vEnd, dwColor);
		pRS->DrawLine(vStart, dwColor, vEnd, dwColor, false, "EditorLineNoZ");
		
		vStartPoint = vPoint;
	}

	vStart = vStartPoint*mView;
	vEnd = Vector(sinf(0)*fRadius, cosf(0)*fRadius, 0.0f)*mView;
	vStart += mFrom;
	vEnd += mFrom;
	pRS->DrawBufferedLine(vStart, dwColor, vEnd, dwColor);

	pRS->DrawLine(vStart, dwColor, vEnd, dwColor, false, "EditorLineNoZ");


	//pRS->FlushBufferedLines());

}

float RGDTransformGizmo::GetDistanceBetweenPointAndLine (const Vector& line_start, const Vector& line_end, const Vector& point, bool bSegmentCollision)
{
	Vector pNormal = line_end - line_start; 
	if(pNormal.Normalize() <= 1e-30f) return -1.0f;
	float pD = (pNormal | point);

	//Проверяем на пересечение с плоскостью...
	float dsrc = (pNormal | line_start) - pD;
	float ddst = (pNormal | line_end) - pD;
	ddst = dsrc - ddst; 
	if(fabsf(ddst) <= 1e-30f) return -1.0f;
	float k = dsrc/ddst;

	Vector p = line_start + (line_end - line_start)*k - point;



	if (bSegmentCollision)
	{
		//Надо проверить не рубиться ли точка p
		//плоскостями одна из которых в начале линии, а другая в конце...
		Vector pN1 = pNormal;
		float pD1 = (pN1 | line_start);
		float Dist1 = (point | pN1) - pD1;
		if (Dist1 < 0) return 9999999999.0f;

		Vector pN2 = -pNormal;
		float pD2 = (pN2 | line_end);
		float Dist2 = (point | pN2) - pD2;
		if (Dist2 < 0) return 9999999999.0f;
	}


	return p.GetLength();

}

bool RGDTransformGizmo::LineLineIntersection (const Vector& line1_start, const Vector& line1_end, const Vector& line2_start, const Vector& line2_end, float Epsilon, Vector& intersection_point, bool bSegmentCollision)
{
	intersection_point = Vector(0.0f, 0.0f, 0.0f);
	//Направляющие прямых
	Vector dir1 = line1_end - line1_start;
	Vector dir2 = line2_end - line2_start;

	//Нормаль плоскости образованной line2 и перпендикулярным отрезком между прямыми
	Vector pNormal = ((dir1 ^ dir2) ^ dir2);
	if(pNormal.Normalize() <= 1e-30f) return false;

	//Дистанция до плоскости...
	float pD = (pNormal | line2_start);

	//Проверяем на пересечение с плоскостью...
	float dsrc = (pNormal | line1_start) - pD;
	float ddst = (pNormal | line1_end) - pD;
	ddst = dsrc - ddst; 
	if(fabsf(ddst) <= 1e-30f) return false;
	float k = dsrc/ddst;

	//точка пересечения линии line1 и плоскости которая из line2 сделана
	Vector p = line1_start + (line1_end - line1_start)*k;

	//дистанцию от точки до линии смотрим
	float fDist = GetDistanceBetweenPointAndLine(line2_start, line2_end, p, bSegmentCollision);

	// Если больше чем надо выход
	if(fDist < 0.0f || fDist > Epsilon) return false;

	intersection_point = p;

	//Пересечение !!!!
	return true;
}

void RGDTransformGizmo::GetPickupRay (const Matrix& matProjection, const Matrix& matView, const RENDERVIEWPORT& ViewPort, float sX, float sY, Vector& rayStart, Vector& rayEnd)
{
	Vector v;
	v.x =  ( ( ( 2.0f * sX ) / ViewPort.Width  ) - 1 ) / matProjection.m[0][0];
	v.y = -( ( ( 2.0f * sY ) / ViewPort.Height ) - 1 ) / matProjection.m[1][1];
	v.z =  1.0f;

	Vector tmp = matView.GetCamPos();

	Matrix mView = matView;
	mView.Inverse ();

	Vector rayDir;
	Vector rayOrig;
	rayOrig = mView.pos;

	//mView.pos = 0.0f;
	//rayDir = mView * v;
	rayDir = mView.MulNormal(v);

	rayStart = rayOrig;
	rayEnd = (rayOrig + (rayDir * 100.0f));
}


void RGDTransformGizmo::SetView (const Matrix& mView)
{
	matGizmoView = mView;
	matGizmoViewInv = matGizmoView;
	matGizmoViewInv.Inverse();
}

void RGDTransformGizmo::SetProjection  (const Matrix& mProjection)
{
	matGizmoProjection = mProjection;
}


void RGDTransformGizmo::SetViewPort (const RENDERVIEWPORT& viewport)
{
	GizmoviewPort = viewport;
}


bool RGDTransformGizmo::HandleMessages(unsigned int uMsg, int LocalMouseX, int LocalMouseY)
{
	if (!bEnabled) return false;
	if (GetAsyncKeyState (VK_MENU) < 0 && uMsg == WM_LBUTTONDOWN) return false;

	// Current screen mouse position
	float fMouseX = (float)LocalMouseX;
	float fMouseY = (float)LocalMouseY;

	if (uMsg == WM_LBUTTONDOWN)
	{
		bMousePressed = true;

		MousePressedPoint = Vector (fMouseX, fMouseY, 0.0f);
	}

	if (uMsg == WM_LBUTTONUP)
	{
		bMousePressed = false;
	}


	if (uMsg == WM_MOUSEMOVE)
	{
		if (bMousePressed)
		{
			return GizmoChangeTransform (fMouseX, fMouseY);
		} else
		{
			return HighlightAxis (fMouseX, fMouseY);
		}
	}


	return false;
}

bool RGDTransformGizmo::HighlightAxis(float fMouseX, float fMouseY)
{
	Vector vPickupStart;
	Vector vPickupEnd;
	GetPickupRay(matGizmoProjection, matGizmoView, GizmoviewPort, fMouseX, fMouseY, vPickupStart, vPickupEnd);

	float fEpsilon = 0.1f * GizmoScale;
	Vector vIntersection;
	
	Vector vCamPos = matGizmoView.GetCamPos();
	float fSelDistance = 99999999.0f;
	SelectedAxis = GA_AXIS_NONE;

	bHighlighted=false;

	//Test X axis...
	for (DWORD i = 0; i < XAxis.Size(); i++)
	{
		//нужна точка пересечения...
		bool bResult = LineLineIntersection(vPickupStart, vPickupEnd, XAxis[i].vStart, XAxis[i].vEnd, fEpsilon, vIntersection, true);
		if (bResult)
		{
			float fIntersectionDistance = Vector(vCamPos-vIntersection).GetLength();
			if (fIntersectionDistance < fSelDistance)
			{
				PointOfAxisSelect = vIntersection;
				fSelDistance = fIntersectionDistance;
				SelectedAxis = GA_AXIS_X;
				
				CreateMouseLine (XAxis[i].vStart, XAxis[i].vEnd);

				bHighlighted=true;
			}
		}
	}

	//Test Y axis...
	for ( i = 0; i < YAxis.Size(); i++)
	{
		//нужна точка пересечения...
		bool bResult = LineLineIntersection(vPickupStart, vPickupEnd, YAxis[i].vStart, YAxis[i].vEnd, fEpsilon, vIntersection, true);
		if (bResult)
		{
			float fIntersectionDistance = Vector(vCamPos-vIntersection).GetLength();
			if (fIntersectionDistance < fSelDistance)
			{
				PointOfAxisSelect = vIntersection;

				fSelDistance = fIntersectionDistance;
				SelectedAxis = GA_AXIS_Y;

				CreateMouseLine (YAxis[i].vStart, YAxis[i].vEnd);
				
				bHighlighted=true;
			}
		}
	}

	//Test Z axis...
	for ( i = 0; i < ZAxis.Size(); i++)
	{
		//нужна точка пересечения...
		bool bResult = LineLineIntersection(vPickupStart, vPickupEnd, ZAxis[i].vStart, ZAxis[i].vEnd, fEpsilon, vIntersection, true);
		if (bResult)
		{
			float fIntersectionDistance = Vector(vCamPos-vIntersection).GetLength();
			if (fIntersectionDistance < fSelDistance)
			{
				PointOfAxisSelect = vIntersection;

				fSelDistance = fIntersectionDistance;
				SelectedAxis = GA_AXIS_Z;

				CreateMouseLine (ZAxis[i].vStart, ZAxis[i].vEnd);

				bHighlighted=true;
			}
		}
	}

	switch (SelectedAxis)
	{
		case GA_AXIS_NONE:
			{
				AxisColors[X_AXIS_INDEX] = OriginalAxisColors[X_AXIS_INDEX];
				AxisColors[Y_AXIS_INDEX] = OriginalAxisColors[Y_AXIS_INDEX];
				AxisColors[Z_AXIS_INDEX] = OriginalAxisColors[Z_AXIS_INDEX];
				break;
			}
		case GA_AXIS_X:
			{
				AxisColors[X_AXIS_INDEX] = 0xFFFFFF00;
				AxisColors[Y_AXIS_INDEX] = OriginalAxisColors[Y_AXIS_INDEX];
				AxisColors[Z_AXIS_INDEX] = OriginalAxisColors[Z_AXIS_INDEX];
				break;
			}
		case GA_AXIS_Y:
			{
				AxisColors[Y_AXIS_INDEX] = 0xFFFFFF00;
				AxisColors[X_AXIS_INDEX] = OriginalAxisColors[X_AXIS_INDEX];
				AxisColors[Z_AXIS_INDEX] = OriginalAxisColors[Z_AXIS_INDEX];
				break;
			}
		case GA_AXIS_Z:
			{
				AxisColors[Z_AXIS_INDEX] = 0xFFFFFF00;
				AxisColors[X_AXIS_INDEX] = OriginalAxisColors[X_AXIS_INDEX];
				AxisColors[Y_AXIS_INDEX] = OriginalAxisColors[Y_AXIS_INDEX];
				break;
			}
	}

	return true;
}

bool RGDTransformGizmo::GizmoChangeTransform (float fMouseX, float fMouseY)
{
	bool bRes = false;

	//Рассчитываем смещение по 2-х мерной линии 
	vMouseLine_Dir = vMouseLine_End - vMouseLine_Start;
	fMouseLine_Len = vMouseLine_Dir.Normalize();
	Vector vMousePosition(fMouseX, fMouseY, 0.0f);
	Vector vPointDir = vMousePosition - vMouseLine_Start;
	Vector vPoint = vMouseLine_Start + (vMouseLine_Dir | vPointDir) * vMouseLine_Dir;
	fDeltaMove = Vector(vPoint-MousePressedPoint).GetLength();
	Vector t_dir = (vPoint - MousePressedPoint);
	if ((t_dir | vMouseLine_Dir) < 0) fDeltaMove = -fDeltaMove;
	MousePressedPoint = vPoint;

	switch (mType)
	{
	case GT_MOVE:
		bRes = GizmoChangeTransformMove(fMouseX, fMouseY);
		break;
	case GT_ROTATE:
		bRes = GizmoChangeTransformRotate(fMouseX, fMouseY);
		break;
	}
	return bRes;
}

bool RGDTransformGizmo::GizmoChangeTransformMove (float fMouseX, float fMouseY)
{
	float fDeltaPosition = fDeltaMove * 0.04f;

	Vector vPickupStart;
	Vector vPickupEnd;
	GetPickupRay(matGizmoProjection, matGizmoView, GizmoviewPort, MousePressedPoint.x, MousePressedPoint.y, vPickupStart, vPickupEnd);

	float fEpsilon = 0.1f;
	Vector vIntersection = 0.0f;
	Vector vAxisDirection;
	
	switch (SelectedAxis)
	{
	case GA_AXIS_X:
		LineLineIntersection(vPickupStart, vPickupEnd, XAxis[0].vStart, XAxis[0].vEnd, fEpsilon, vIntersection, false);
		vAxisDirection = XAxis[0].vEnd-XAxis[0].vStart;
		break;
	case GA_AXIS_Y:
		LineLineIntersection(vPickupStart, vPickupEnd, YAxis[0].vStart, YAxis[0].vEnd, fEpsilon, vIntersection, false);
		vAxisDirection = YAxis[0].vEnd-YAxis[0].vStart;
		break;
	case GA_AXIS_Z:
		LineLineIntersection(vPickupStart, vPickupEnd, ZAxis[0].vStart, ZAxis[0].vEnd, fEpsilon, vIntersection, false);
		vAxisDirection = ZAxis[0].vEnd-ZAxis[0].vStart;
		break;
	}


	//vIntersection - точка на линии куда надо сдвинуть...
	//PointOfAxisSelect - точка куда ткунули когда тянуть начали...

	float fOffset = Vector(vIntersection-PointOfAxisSelect).GetLength();
	Vector t_dir = (vIntersection - PointOfAxisSelect);
	if ((t_dir | vAxisDirection) < 0) fOffset = -fOffset;

	Vector vOffset = 0.0f;
	switch (SelectedAxis)
	{
	case GA_AXIS_X:
		vOffset.x += fOffset;
		break;
	case GA_AXIS_Y:
		vOffset.y += fOffset;
		break;
	case GA_AXIS_Z:
		vOffset.z += fOffset;
		break;
	}


	if (mMode == GM_LOCAL)
	{
		Matrix mToLocal = mTransform;
		mToLocal.pos = 0.0f;
		vOffset = vOffset * mToLocal;
	}

	mTransform.pos += vOffset;


	PointOfAxisSelect = vIntersection;

	return false;
}

bool RGDTransformGizmo::GizmoChangeTransformRotate (float fMouseX, float fMouseY)
{
	float fDeltaAngle = fDeltaMove * api->GetDeltaTime();

	Matrix mRotate;
	switch (SelectedAxis)
	{
		case GA_AXIS_X:
			mRotate.BuildRotateX(fDeltaAngle);
			break;
		case GA_AXIS_Y:
			mRotate.BuildRotateY(fDeltaAngle);
			break;
		case GA_AXIS_Z:
			mRotate.BuildRotateZ(fDeltaAngle);
			break;
	}

	if (mMode == GM_WORLD)
	{
		Vector vOldPos = mTransform.pos;
		mTransform.pos = 0.0f;
		mTransform = mTransform * mRotate;
		mTransform.pos = vOldPos;
	} else
	{
		mTransform = mRotate * mTransform;
	}

	return true;
}


void RGDTransformGizmo::CreateMouseLine(const Vector& v1, const Vector& v2)
{
	Vector line_start = v1 * matGizmoView;
	Vector line_end = v2 * matGizmoView;
	Vector4 v4Start = matGizmoProjection.Projection(line_start, (GizmoviewPort.Width*0.5f), (GizmoviewPort.Height*0.5f));
	Vector4 v4End = matGizmoProjection.Projection(line_end, (GizmoviewPort.Width*0.5f), (GizmoviewPort.Height*0.5f));

	vMouseLine_Start = Vector(v4Start.x, v4Start.y, 0.0f);
	vMouseLine_End = Vector(v4End.x, v4End.y, 0.0f);

	Vector vDir = vMouseLine_Start-vMouseLine_End;
	vDir.Normalize();

	vMouseLine_Start -= vDir * 40.0f;
	vMouseLine_End += vDir * 40.0f;


}

const Matrix& RGDTransformGizmo::GetTransform ()
{
	return mTransform;
}






void RGDTransformGizmo::SetType (GizmoType _type)
{
	bMousePressed = false;
	mType = _type;
}
void RGDTransformGizmo::SetMode (GizmoMode _mode)
{
	bMousePressed = false;
	mMode = _mode;
}

RGDTransformGizmo::GizmoType RGDTransformGizmo::GetType ()
{
	return mType;
}

RGDTransformGizmo::GizmoMode RGDTransformGizmo::GetMode ()
{
	return mMode;
}

void RGDTransformGizmo::SetTransform (const Matrix& m)
{
	bMousePressed = false;
	mTransform = m;
}

bool RGDTransformGizmo::IsActiveMode()
{
	return bMousePressed;
}

void RGDTransformGizmo::Enable (bool _bEnabled)
{
	bMousePressed = false;
	bEnabled = _bEnabled;
}

bool RGDTransformGizmo::IsEnabled ()
{
	return bEnabled;
}
