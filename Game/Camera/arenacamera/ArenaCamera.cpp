#include "ArenaCamera.h"




__forceinline float floatMoveByStep(float from, float to, float step)
{
	if(step <= 0.0f)
	{
		return from;
	}
	
	//Дистанция до точки
	float dist = to - from;
	if(dist < 1e-30)
	{
		return to;
	}

	//Движение к точке
	if(step >= dist)
	{
		return to;
	}
	double k = step/dist;
	return float(from+(dist*k));
}



ShipFightCamera::ShipFightCamera() :
	targets( _FL_ )
{
	fCamHeight = 10.0f;
//	fDistToFitFrustum = 30.0f;
	bShowDebug = true;

	activeShipIdx = -1;

	bRealNotInited = true;

	moveSpeed = 10.0f;

	fPanOffset = 0.0f;
	fPanOffsetReal = 0.0f;
	_panVector = 0.0f;

	fZoom = 0.0f;
	fZoomSpeed = 1.0f;

	fTimeWithoutZoom = 0.0f;

}

ShipFightCamera::~ShipFightCamera()
{
}

void ShipFightCamera::Reset()
{
	BaseCamera::Reset();
}

//Прочитать параметры идущие перед стандартными
bool ShipFightCamera::CameraPreCreate(MOPReader & reader)
{
	fCamHeight = reader.Float();
	fCamHeightNear = reader.Float();
//	fDistToFitFrustum = reader.Float();
	bShowDebug = reader.Bool();
	fDistToSwitch = reader.Float();

	fOffsetToPlayer = 1.0f - Clampf (reader.Float() / 100.0f);

	moveSpeed = reader.Float();
	fPanSpeed = reader.Float();
	fZoomSpeed = reader.Float();


	dword targetsCount = reader.Array();
	Assert(targetsCount == 3);
	
	targets.DelAll();
	
	targets.AddElements(targetsCount);
	// заполнить пустотой
	for(dword i = 0; i < targetsCount; i++)
	{
		targets[i].target.Reset();
		targets[i].targetID.Empty();
		targets[i].active = true;
	}
	// взять заполнение из параметров
	for(i = 0; i < targetsCount - 1; i++)
	{
		targets[i].targetID = reader.String();
		targets[i].targetPnt = reader.Position();
	}

	return true;
}

//Прочитать параметры идущие после стандартных
bool ShipFightCamera::CameraPostCreate(MOPReader & reader)
{
	if( targets.Size() > 0 )
	{
		targets.LastE().targetID = targetID;
		targets.LastE().targetPnt = targetPos;
	}

	SetUpdate(&ShipFightCamera::DebugDraw, ML_ALPHA5);

	return true;
}

// аптейт данных камеры на кадре
void ShipFightCamera::WorkUpdate(float dltTime)
{
	// обновить цели
	UpdateTargets();

}


// trg.target.Ptr() не const конструкция, пришлось поправить :(
void ShipFightCamera::AddObjectToSphere(bool & bSphereInited, Sphere & sph, /*const */Target &trg)
{
	Vector boxMin;
	Vector boxMax;

	Matrix mtx(true);

	if (!trg.target.Validate())
	{
		return;
	}

	trg.target.Ptr()->GetMatrix(mtx);
	trg.target.Ptr()->GetBox(boxMin, boxMax);


	if (bSphereInited)
	{
		sph.AddPoint( mtx *  Vector(boxMin.x, boxMin.y, boxMin.z));
	} else
	{
		sph.pos = mtx * Vector(boxMin.x, boxMin.y, boxMin.z);
		sph.r = 0.0f;

		bSphereInited = true;
	}

	sph.AddPoint( mtx *  Vector(boxMin.x, boxMin.y, boxMax.z));
	sph.AddPoint( mtx *  Vector(boxMax.x, boxMin.y, boxMin.z));
	sph.AddPoint( mtx *  Vector(boxMax.x, boxMin.y, boxMax.z));

	sph.AddPoint( mtx *  Vector(boxMin.x, boxMax.y, boxMin.z));
	sph.AddPoint( mtx *  Vector(boxMin.x, boxMax.y, boxMax.z));
	sph.AddPoint( mtx *  Vector(boxMax.x, boxMax.y, boxMin.z));
	sph.AddPoint( mtx *  Vector(boxMax.x, boxMax.y, boxMax.z));
}

void ShipFightCamera::CalcMinDistanceToPlane(bool & bSphereInited, Vector& minV, float & fMinDepth, float & fMin, const Plane & p, /*const */Target &trg, const Matrix & mtxView, LRFlag flg)
{
	Vector boxMin;
	Vector boxMax;

	Matrix mtxWorld(true);

	if (!trg.target.Validate())
	{
		return;
	}

	trg.target.Ptr()->GetMatrix(mtxWorld);
	trg.target.Ptr()->GetBox(boxMin, boxMax);




	Matrix mProj = Render().GetProjection();
	Matrix mtxWorldViewProj(true);
	mtxWorldViewProj.EqMultiplyFast(mtxView, mProj);

	




	Vector arr[8];

	arr[0] = mtxWorld * Vector(boxMin.x, boxMax.y, boxMin.z); 
	arr[1] = mtxWorld * Vector(boxMin.x, boxMax.y, boxMax.z); 
	arr[2] = mtxWorld * Vector(boxMax.x, boxMax.y, boxMin.z); 
	arr[3] = mtxWorld * Vector(boxMax.x, boxMax.y, boxMax.z); 

	arr[4] = mtxWorld * Vector(boxMin.x, boxMin.y, boxMin.z); 
	arr[5] = mtxWorld * Vector(boxMin.x, boxMin.y, boxMax.z); 
	arr[6] = mtxWorld * Vector(boxMax.x, boxMin.y, boxMin.z); 
	arr[7] = mtxWorld * Vector(boxMax.x, boxMin.y, boxMax.z); 

	Vector4 projV[8];
	for (int i = 0; i < 8; i++)
	{
		projV[i] = mtxWorldViewProj.Projection(arr[i]);
		//projV[i].x = (projV[i].x * 2.0f) - 1.0f;
	}

	fMinDepth = 0.0f;

	if (flg == ShipFightCamera::LR_LEFT)
	{
		int start = 0;
		if (bSphereInited == false)
		{
			fMin = projV[0].x;
			minV = arr[0];
			start = 1;
			bSphereInited = true;
		}

		for (int i = start; i < 4; i++)
		{
			if (projV[i].x < fMin)
			{
				fMin = projV[i].x;
				minV = arr[i];
			}
		}

		//fMin = fabsf(fMin);

		return;
	} 


	int start = 0;
	if (bSphereInited == false)
	{
		fMin = projV[0].x;
		minV = arr[0];
		start = 1;
		bSphereInited = true;
	}

	for (int i = start; i < 4; i++)
	{
		if (projV[i].x > fMin)
		{
			fMin = projV[i].x;
			minV = arr[i];
		}
	}

	//fMin = fabsf(fMin);

	return;
	
	


	Vector cam = mtxView.GetCamPos();

	if (bSphereInited)
	{
		float dist = GetDistanceToPlane(arr[0], p);
		if (dist < fMin)
		{
			fMin = dist;
			fMinDepth = (arr[0]-cam).GetLength();
			minV = arr[0];
		}
		

	} else
	{
		fMin = GetDistanceToPlane(arr[0], p);
		fMinDepth = (arr[0]-cam).GetLength();
		minV = arr[0];
		
		bSphereInited = true;
	}


	
	//тока по верхним точкам смотрим...
	for (long j = 1; j < 4; j++)
	{
		float dist = GetDistanceToPlane(arr[j], p);
		if (dist < fMin)
		{
			fMin = dist;
			fMinDepth = (arr[j]-cam).GetLength();
			minV = arr[j];
		}
	}


}


// обновить таргеты камеры
void ShipFightCamera::UpdateTargets()
{
	for(dword i = 0; i < targets.Size(); i++)
	{
		Target & trg = targets[i];
		trg.active = false;
		if(trg.targetID.NotEmpty())
		{
			// обновить(проверить) указатель на объект
			if(!trg.target.Validate())
			{
				FindObject(trg.targetID, trg.target);
			}

			// объект живой - значит используем его в списке активных целей
			if( trg.target.Ptr() && trg.target.Ptr()->IsActive() && !trg.target.Ptr()->IsDead() )
			{
				Matrix & mtx = trg.world;
				trg.target.Ptr()->GetMatrix(mtx);
				trg.active = true;
			}			
		}
	}


}



//Инициализация параметров
void ShipFightCamera::InitParams()
{
}


//Выбирает второй по значимости в композиции корабль (1-й это корабль игрока)
long ShipFightCamera::SelectMainShip (long currentMainShip)
{
	if (targets[1].active == false && targets[2].active == false)
	{
		return 0;
	}

	//Простые случаи, кто то один сдох....
	if (targets[1].active == false)
	{
		return 2;
	}

	if (targets[2].active == false)
	{
		return 1;
	}


	//Сложный случай, оба живы
	//надо выбрать кто поглавнее
	//тут непомешал бы доступ в мысли корабля, но нет так нет...


	//Выбираем тупо по дистанции, не было в прошлом кадре главного
	if (currentMainShip < 0)
	{
		float len1 = (targets[0].world.pos - targets[1].world.pos).GetLength();
		float len2 = (targets[0].world.pos - targets[2].world.pos).GetLength();

		if (len1 > len2)
		{
			return 1;
		}

		return 2;
	}


	//Оба живы и в прошлый раз был активен currentMainShip корабль...
	//Меняем активность, только если разница между более близким кораблем и текущим активным превысила 200 метров

	long currentPassiveShip = 1;
	if (currentMainShip == 1) currentPassiveShip = 2;

	float len_act = (targets[0].world.pos - targets[currentMainShip].world.pos).GetLength();
	float len_passive = (targets[0].world.pos - targets[currentPassiveShip].world.pos).GetLength();

	//Если ближе к игроку, чем прошлый активный на fDistToSwitch метров
	if ((len_act - len_passive) > fDistToSwitch)
	{
		return currentPassiveShip;
	}


	return currentMainShip;
}


Vector ShipFightCamera::CalcCamPosFromQuat (const Quaternion & q, const Vector & lookTo, float fDist)
{
	Matrix mCam;
	q.GetMatrix(mCam);
	Vector camPos = lookTo - mCam.vz * fDist;
	return camPos;
}


void _cdecl ShipFightCamera::DebugDraw(float dltTime, long level)
{

	UpdateTargets();


	activeShipIdx = SelectMainShip(activeShipIdx);

	long currentPassiveShip = 1;
	if (activeShipIdx == 1) currentPassiveShip = 2;
	//if (activeShipIdx == 0) currentPassiveShip = 0;


	


	if (bShowDebug)
	{
		Render().DrawSphere(targets[0].world.pos, 30.0f, 0xFFFF0000);
		Render().DrawSphere(targets[activeShipIdx].world.pos, 30.0f, 0xFFFF0000);

		Render().Print(0, 0, 0xFFFFFFFF, "%s vs %s", targets[0].targetID.c_str(), targets[activeShipIdx].targetID.c_str());
	}

	



	bool bInited = false;
	Sphere sph;



	AddObjectToSphere(bInited, sph, targets[activeShipIdx]);
	AddObjectToSphere(bInited, sph, targets[0]);

	if (bShowDebug)
	{
		Render().DrawSphere(sph.pos, sph.r, 0xFFFF00FF);
	}


	Vector nearestPoints[3];
	Vector farestPoints[3];
	for(dword i = 0; i < targets.Size(); i++)
	{
		Target & trg = targets[i];

		Vector dir = (trg.world.pos - sph.pos);
		dir.y = 0.0f;
		dir.Normalize();

		nearestPoints[i] = sph.pos + (dir * sph.r);
		nearestPoints[i].y = 0.0f;

		farestPoints[i] = sph.pos - (dir * sph.r);
		farestPoints[i].y = 0.0f;


		if (bShowDebug)
		{
			Render().DrawSphere(nearestPoints[i], 3.0f, 0xFF00FF00);
			Render().DrawSphere(farestPoints[i], 3.0f, 0xFF0000FF);

			Render().DrawVector(trg.world.pos, nearestPoints[i], 0xFF00FF00);
			Render().DrawVector(trg.world.pos, farestPoints[i], 0xFF0000FF);
		}

	}


	//Сферическая интерполяция тут не подходит, нужна линейная - пока просто это перпендикуляр...
	//а можно по идее прямую между кораблями посторить и на ней точку отложить и от этой точки найти ближайшую на сфере уже
	//что бы не ровно посередине была камера....
	Vector delta = !(nearestPoints[0] - nearestPoints[activeShipIdx]);
	Vector up = Vector (0.0f, 1.0f, 0.0f);

	Vector normal2D = delta.Rotate_PI2_CW();
	Vector potentialCamPointNrm = sph.pos + normal2D * sph.r;


	Vector dir = !(potentialCamPointNrm - nearestPoints[0]);
	Vector dist = (potentialCamPointNrm - nearestPoints[0]).GetLength();

	Vector potentialCamPointOnLine = nearestPoints[0] + dir * (dist * fOffsetToPlayer);


	dir = (potentialCamPointOnLine - sph.pos);
	dir.y = 0.0f;
	dir.Normalize();
	Vector potentialCamPoint = sph.pos + (dir * sph.r);
	potentialCamPoint.y = 0.0f;


	float blend_to_BackgroundShip = (!(farestPoints[currentPassiveShip]-sph.pos)) | (!(potentialCamPoint-sph.pos));
	blend_to_BackgroundShip = Clampf(blend_to_BackgroundShip);

	if (currentPassiveShip <= 0)
	{
		blend_to_BackgroundShip = 0.0f;
	}

	if (!targets[currentPassiveShip].active)
	{
		blend_to_BackgroundShip = 0.0f;
	}



	if (bShowDebug)
	{
		Render().DrawSphere(potentialCamPointNrm, 5.0f, 0xFFA0FF20);
		Render().DrawLine(potentialCamPointNrm, 0xFFA0FF00, potentialCamPoint, 0xFFA0FF00);

		Render().DrawSphere(potentialCamPoint, 6.0f, 0xFFA0FF00);
		Render().DrawVector(sph.pos, potentialCamPoint, 0xFFA0FF00);
		Render().Print (potentialCamPoint + Vector(0.0f, 5.5f, 0.0f), 10000.0f, 0.0f, 0xFF00FF00, "%f", blend_to_BackgroundShip);
	}







	Quaternion mainCam;
	Quaternion secondCam;

	Matrix mCamTemp;


	float fDistToMain = (targets[0].world.pos - targets[activeShipIdx].world.pos).GetLength();
	float fCalcHeight = Lerp (fCamHeightNear, fCamHeight, Clampf(fDistToMain / 320.0f));

	camTarget = sph.pos;
	camTarget.y = fCalcHeight;


	//Основная камера, смотрит между корабля игрока и корабля главного злодея...
	camPos = potentialCamPoint;
	camPos.y += fCalcHeight;
	mCamTemp.BuildView(camPos, camTarget, Vector (0.0, 1.0f, 0.0f));
	mCamTemp.Inverse();
	mainCam = Quaternion(mCamTemp);


	//Дополнительная камера смотрит на корабль третьего плана, через центр сферы..
	camPos = farestPoints[currentPassiveShip];
	camPos.y += fCalcHeight-1.1f;
	mCamTemp.BuildView(camPos, camTarget, Vector (0.0, 1.0f, 0.0f));
	mCamTemp.Inverse();
	secondCam = Quaternion(mCamTemp);


	//Блендим с коэфицентом, чем дальше второстепенная камера от главной, тем меньше блендинг...
	Quaternion mainCamera;
	mainCamera.SLerp(mainCam, secondCam, blend_to_BackgroundShip);


	float distR = sph.r * fZoom;
	camPos = CalcCamPosFromQuat(mainCamera, camTarget, sph.r + distR);
	camPos.y = fCalcHeight;





	if (bRealNotInited)
	{
		camPosReal = camPos;
		camTargetReal = camTarget;

		bRealNotInited = false;
	} else
	{
		camPosReal.MoveByStep(camPos, moveSpeed * dltTime);
		camTargetReal.MoveByStep(camTarget, moveSpeed * dltTime);
	}

	Matrix view_withoutPan;
	view_withoutPan.BuildView((camPosReal), (camTargetReal), Vector(0.0f, 1.0f, 0.0f));
	Matrix mInvV = view_withoutPan;
	mInvV.Inverse();

	_panVector = (mInvV.vx * fPanOffsetReal);


	Matrix view;
	view.BuildView((camPosReal+_panVector), (camTargetReal+_panVector), Vector(0.0f, 1.0f, 0.0f));

	//Vector camPos = view.GetCamPos();


	Plane Frustum[2];
	Vector v[2];

	Matrix mP = Render().GetProjection();
	v[0] = !Vector( mP.m[0][0], 0.0f, 1.0f);
	v[1] = !Vector(-mP.m[0][0], 0.0f, 1.0f);

	/*
	if(largeshot->Get(0))
	{
		v[0] = Vector(0.0f, 0.0f, 1.0f);
		v[1] = Vector(0.0f, 0.0f, 1.0f);
	}
	*/

	Matrix mIV = view;
	mIV.Inverse();

	for (dword i = 0; i < 2; i++) 
	{
		Frustum[i].n = mIV.MulNormal(v[i]);
		Frustum[i].d = Frustum[i].n | mIV.pos;
	}

	bool bLeftInited = false;
	float fLeftDist = 0.0f;
	float fLeftDepth = 0.0f;
	Vector vLeftSph;
	CalcMinDistanceToPlane(bLeftInited, vLeftSph, fLeftDepth, fLeftDist, Frustum[0], targets[activeShipIdx], view, ShipFightCamera::LR_LEFT);
	CalcMinDistanceToPlane(bLeftInited, vLeftSph, fLeftDepth, fLeftDist, Frustum[0], targets[0], view, ShipFightCamera::LR_LEFT);


	 
	bool bRightInited = false;
	float fRightDist = 0.0f;
	float fRightDepth = 0.0f;
	Vector vRightSph;
	CalcMinDistanceToPlane(bRightInited, vRightSph, fRightDepth, fRightDist, Frustum[1], targets[activeShipIdx], view, ShipFightCamera::LR_RIGHT);
	CalcMinDistanceToPlane(bRightInited, vRightSph, fRightDepth, fRightDist, Frustum[1], targets[0], view, ShipFightCamera::LR_RIGHT);

	fLeftDist -= 1.0f;
	fRightDist -= 1.0f;

	if (bShowDebug)
	{
		Render().Print(800, 32, 0xFFFFFFFF, "min: %f, max: %f", fLeftDist, fRightDist);
	}


	fLeftDist = 1.0f - fabsf(fLeftDist);
	fRightDist = 1.0f - fabsf(fRightDist);

	if (bShowDebug)
	{

	Render().Print(0, 16, 0xFFFFFFFF, "l:%f, f:%f     d [%f, %f]", fLeftDist, fRightDist, fLeftDepth, fRightDepth);
	Render().Print(0, 32, 0xFFFFFFFF, "pan: %f", fPanOffsetReal);
	}

	float kkk = fPanOffsetReal;

	float fPanDist = (fRightDist - fLeftDist);

	float lerpK = Clampf(fPanSpeed * dltTime);

	if (fPanDist > 0.1f)
	{
		if (bShowDebug)
		{
		Render().Print(0, 48, 0xFFFFFFFF, "pan <----- (from %f to %f) speed:%f",fPanOffsetReal, fPanOffsetReal-fPanDist, dltTime);
		}

		fPanOffsetReal = Lerp(fPanOffsetReal, fPanOffsetReal-fPanDist, lerpK);


		//fPanOffsetReal = floatMoveByStep(fPanOffsetReal, fPanOffsetReal-0.2f, fPanSpeed * dltTime);

	}

	fPanDist = (fLeftDist - fRightDist);
	if (fPanDist > 0.1f)
	{
		if (bShowDebug)
		{
		Render().Print(0, 48, 0xFFFFFFFF, "pan -----> (from %f to %f) speed:%f",fPanOffsetReal, fPanOffsetReal+fPanDist, dltTime);
		}

		fPanOffsetReal = Lerp(fPanOffsetReal, fPanOffsetReal+fPanDist, lerpK);

		//fPanOffsetReal = floatMoveByStep(fPanOffsetReal, fPanOffsetReal+0.2f, fPanSpeed * dltTime);


	}



	if (fPanOffsetReal < -30.0f)
	{
		fPanOffsetReal = -30.0f;
	}

	if (fPanOffsetReal > 30.0f)
	{
		fPanOffsetReal = 30.0f;
	}

	


	
	

	
	
	if (bShowDebug)
	{
		Render().DrawSphere(vLeftSph, 5.0f, 0xFFFFFFFF);
		Render().DrawSphere(vRightSph, 5.0f, 0xFFFFFFFF);

		Render().DrawSphere(camPos, 3.0f, 0xFFFFFF00);
		Render().DrawVector(camPos, camTarget, 0xFFFFFF00);
		Render().Print(camPos, 100000.0f, 0.0f, 0xFFFFFFFF, "camera here !");


		Render().DrawVector(camPos, camPos+mIV.vx, 0xFFFFFF00);
		Render().DrawVector(camPos, camPos-mIV.vx, 0xFFFFFF00);
		


		Render().DrawSphere(camPos+_panVector, 3.0f, 0xFFFFFF80);
		Render().DrawVector(camPos+_panVector, camTarget+_panVector, 0xFFFFFF80);
		Render().DrawLine(camPos+_panVector, 0xFFFFFFFF, camPos, 0xFFFFFF00);
		Render().Print(camPos+_panVector, 100000.0f, 1.0f, 0xFFFFFF80, "pan camera here !");

	}


	bool bZoomEnabled = false;

	//float zoomLerpK = Clampf(fZoomSpeed * dltTime);
		if (fLeftDist < 0.2f && fRightDist < 0.2f)
		{
			//Надо сделать zoomOut
			//float fZoomOutDist = fabsf (coremin (fLeftDist, fRightDist));

			//if (zoomLerpK > 0.00001f)
			{
			fZoom = Lerp(fZoom, 1.6f, dltTime*0.4f*fZoomSpeed);
			}

			if (bShowDebug)
			{
			Render().Print(0, 64, 0xFFFFFFFF, "zoom out : %f", fZoom);
			}

			bZoomEnabled = true;

			fTimeWithoutZoom = 0.0f;
		}

/*
		if (fLeftDist > 0.15f && fRightDist > 0.15f)
		{
			float fZoomInDist = coremax (fLeftDist, fRightDist);

			if (zoomLerpK > 0.00001f)
			{
			fZoom = Lerp(fZoom, fZoom-(fZoomInDist * 10.0f), lerpK);
			}

			Render().Print(0, 64, 0xFFFFFFFF, "zoom in : %f", fZoom);

			bZoomEnabled = true;

			fTimeWithoutZoom = 0.0f;
		}
*/


		//Надо вернуть zoom на место...
		if (bZoomEnabled == false)
		{
			fTimeWithoutZoom+= dltTime;

			if (fTimeWithoutZoom > 4.0f)
			{
				fZoom = Lerp(fZoom, 0.0f, dltTime*0.4f*fZoomSpeed);

				if (bShowDebug)
				{
				Render().Print(0, 64, 0xFFFFFFFF, "zoom restore : %f", fZoom);
				}
			} else
			{
				if (bShowDebug)
				{
				Render().Print(0, 64, 0xFFFFFFFF, "time without zoom : %f, zoom : %f", fTimeWithoutZoom, fZoom);
				}
			}

		}

	//Если обе дистанции <0 то надо ZoomOut
	//Если обе дистанции >0 то надо ZoomIn
	



/*
	if (bShowDebug)
	{
		Render().DrawSphere(sph.pos, sph.radius);
		float y  = 10;
		for(dword i = 0; i < targets.Size(); i++)
		{
			Target & t = targets[i];

			Render().Print(0, y, 0xFFFFFFFF, "[%d] '%s' = 0x%08X", i, t.targetID.c_str(), t.target.Ptr());
			y+= Render().GetSystemFont()->GetHeight();
		}
	}


	//Считаем ближайшие точки на сфере для каждого корабля
	for(dword i = 0; i < targets.Size(); i++)
	{
		Target & trg = targets[i];

		Vector dir = (trg.world.pos - sph.pos);
		dir.y = 0.0f;
		dir.Normalize();

		trg.pointOnSphereDir = dir;
		trg.pointOnSphere = sph.pos + (dir * sph.r);
	}


	//Считаем коэфиценты блендинга для каждого корабля (чем больше угол от игрока, тем меньше блендинга К)
	for(dword i = 0; i < targets.Size(); i++)
	{
		targets[i].blendK = (((targets[0].pointOnSphereDir | targets[i].pointOnSphereDir) * 0.3f) + 0.5f) + 0.2f;
		//targets[i].blendK = targets[i].blendK * targets[i].blendK;
	}



	//Переводим матрицы камеры в кватернионы, для каждого корабля - что бы slerp потом делать
	Matrix mCamera;
	for(dword i = 0; i < targets.Size(); i++)
	{
		camPos = (sph.pos + (targets[i].pointOnSphereDir * (sph.r + fDistToFitFrustum)));
		camPos.y += fCamHeight;

		if (bLookToSphereCenter)
		{
			//targets[0].targetPnt; надо поменять на константу - пока влом
			camTarget = sph.pos + targets[0].targetPnt;
		} else
		{
			camTarget = targets[i].world.pos + targets[i].targetPnt;
		}

		mCamera.BuildView(camPos, camTarget, Vector (0.0, 1.0f, 0.0f));
		mCamera.Inverse();
		targets[i].q = Quaternion(mCamera);
	}




	// Нормализуем коэфиценты ------------------------------------------


	
	if (bRenormalizeRotK)
	{
		float k = 0.0f;
		for(dword i = 0; i < targets.Size(); i++)
		{
			k += targets[i].blendK;
		}

		k = 1.0f / k;

		
		for(dword i = 0; i < targets.Size(); i++)
		{
			targets[i].blendK *= k;
		}
	}




	//Находим второй по важности коарбль в композиции...
	//-------------------------------------------------------------------------------
	float fMax = targets[1].blendK;
	long idx = 1;
	for(dword i = 2; i < targets.Size(); i++)
	{
		if (targets[i].blendK > fMax)
		{
			fMax = targets[i].blendK;
			idx = i;
		}
	}






	//Интерполяцию делаем между двумя центральными кораблями сцены, наш делая немного более к камере
	//-------------------------------------------------------------------------------
	Quaternion rot;
	rot.SLerp(targets[0].q, targets[idx].q, 0.4f);




	//Из кватерниона восстанавливаем camPos и camTarget считаем
	//-------------------------------------------------------------------------------
	if (bLookToSphereCenter)
	{
		camTarget = sph.pos + targets[0].targetPnt;
	} else
	{
		camTarget = targets[0].world.pos + targets[0].targetPnt;
	}

	Matrix mCam;
	rot.GetMatrix(mCam);
	camPos = camTarget - mCam.vz * (sph.r + fDistToFitFrustum);



	//------------------------------------------


	if (bShowDebug)
	{
		for(dword i = 0; i < targets.Size(); i++)
		{
			Render().DrawSphere(targets[i].pointOnSphere, 5.0f, 0xFF00FF00);

			Render().Print (targets[i].pointOnSphere + Vector(0.0f, 5.5f, 0.0f), 10000.0f, 0.0f, 0xFF00FF00, "%f", targets[i].blendK);
		}

		

		
		Render().DrawSphere(targets[0].world.pos, 15.0f, 0xFF00FFFF);


		Render().DrawSphere(camPos, 5.0f, 0xFFFFFF00);
		Render().DrawSphere(camTarget, 5.0f, 0xFFFFFF00);
		Render().DrawVector(camPos, camTarget, 0xFFFFFF00);
	}

*/


}

//Нарисовать дополнительную информацию нри селекте
void ShipFightCamera::SelectedDraw(float dltTime)
{
	BaseCamera::SelectedDraw(dltTime);

}

//Получить позицию камеры
void ShipFightCamera::GetCameraPosition(Vector & position)
{
	position = (camPosReal+_panVector);
}

//Получить точку наблюдения камеры
bool ShipFightCamera::GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up)
{
	target_pos = (camTargetReal+_panVector);
	up = Vector (0.0, 1.0f, 0.0f);
	return true;

}


MOP_BEGINLISTCG(ShipFightCamera, "ShipFight Camera", '1.00', 0, CAMERA_COMMENT("Camera for battle on the ships\n\n"), "Cameras")

	MOP_FLOAT("Height", 20.0)
	MOP_FLOAT("HeightNear", 15.0)
	//MOP_FLOAT("DebugDist", 25.0)
	MOP_BOOL("DebugDraw", false)
	MOP_FLOAT("_DistanceToSwitch", 150.0)
	MOP_FLOATEX("OffsetToPlayer", 0.0, 0.0f, 100.0f)
	MOP_FLOATEX("MoveSpeed", 130.0, 0.0f, 1000000000.0f)
	MOP_FLOATEX("PanSpeed", 20.0, 0.0f, 100000.0f)
	MOP_FLOATEX("ZoomSpeed", 1.0, 0.0f, 1000000000.0f)


	MOP_ARRAYBEG("Targets", 3, 3)
		MOP_CAM_TARGET
	MOP_ARRAYEND
	// стандартные параметры базового класса (за исключением первого MOP_CAM_TARGET,
	// его нужно брать в качестве последних данных с предыдущего массива целей)
	MOP_CAM_FOV
	MOP_CAM_BLENDER
	MOP_CAM_LEVEL
	MOP_CAM_ACTIVATE
	MOP_CAM_PREVIEW
	MOP_CAM_DRAWCAMERA
MOP_ENDLIST(ShipFightCamera)
