//============================================================================================
// Spirenkov Maxim
//============================================================================================
// BaseCamera
//============================================================================================


#include "BaseCamera.h"
#include "..\CameraController\CameraController.h"

BaseCamera::BaseCamera()
{
	//debugCamFov = 0.0f;
	//debugInputFov = 0.0f;
	//debugFovToPrint = 0.0f;
	fov = 1.0f + PI*0.5f;
	model = null;
	isPreview = false;
	attachedobjIDhash = 0;

	fNeededDist=2.5f;
	fMinAngle=10;
	fMaxAngle=25;

	fMinSpeedAngle=80;
	fMaxSpeedAngle=120;

	MissionObject::Activate(false);

	AutoZoom = false;

	bDrawCamera = true;

	debugdraw_enbled = false;

	model = null;


	fovChgTime = -1.0f;
	fovChgCurTime = -1.0f;
	fovChgTarget = fov;

	fovLast = -1.0f;

	init_fov = fov;
}

BaseCamera::~BaseCamera()
{
	MissionObject::Activate(false);

	MGIterator* iter;

	iter = &Mission().GroupIterator(CAMERA_CONTROLLER_GROUP, _FL_);

	if (!iter->IsDone())
	{		
		CameraController* CamController = (CameraController*)iter->Get();			

		CamController->FindHighestActiveCam();
	}

	iter->Release();

	if (model)
	{
		model->Release();
		model = null;
	}
}

void BaseCamera::InitParams()
{
	target.Reset();
	fov = 1.0f + PI*0.5f;
	isPreview = false;


	fNeededDist=2.5f;
	fMinAngle=10;
	fMaxAngle=25;

	fMinSpeedAngle=80;
	fMaxSpeedAngle=120;

	MissionObject::Activate(false);

	AutoZoom = false;

	bDrawCamera = true;	

	attachedobj.Reset();
	cam_offset = 0.0f;

	attach_mode = 0;

	init_fov = fov;
}

//Получить точку наблюдения камеры
bool BaseCamera::GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up)
{
	if(targetID.NotEmpty())
	{
		if(!target.Validate())
		{
			FindObject(targetID, target);
		}
	}
	if(!target.Validate())
	{
		return false;
	}
	Matrix mtx;
	target.Ptr()->GetMatrix(mtx);
	target_pos = mtx*targetPos;
	return true;
}

//Получить FoV камеры
float BaseCamera::GetCameraFoV()
{
	return fCurFOV;
}

//Инициализировать объект
bool BaseCamera::Create(MOPReader & reader)
{
	InitParams();

	CameraController * CamController = GetCameraController();
	if(!CameraPreCreate(reader))
	{
		return false;
	}
	
	targetID = reader.String();
	target.Reset();
	targetPos = reader.Position();
	fov = reader.Float()*PI/180.0f;
	init_fov = fov;
	
	float btime = reader.Float();
	blender.Init(btime);
	
	level = ML_CAMERAMOVE + reader.Long();
	
	bool isAct = reader.Bool();
	
	isPreview = reader.Bool();
	
	bDrawCamera = reader.Bool();

	if(!CameraPostCreate(reader))
	{
		return false;
	}
	
	//Activate(!isAct);

	if(IsActive())
	{
		MGIterator* iter;

		iter = &Mission().GroupIterator(CAMERA_CONTROLLER_GROUP, _FL_);

		if (!iter->IsDone())
		{		
			CameraController* CamController = (CameraController*)iter->Get();			

			if (CamController->GetActiveCameraLevel()<level)
			{
				CamController->SetActiveCamera(this);
			}
		}

		iter->Release();
	}

	Activate(isAct);	

	Registry(CAMERAS_GROUP);

	AutoZoom = false;
	fCurFOV = fov;


	//SetUpdate(&BaseCamera::CameraDebugDraw, ML_GUI1 + 1);

	return true;
}

//Пересоздать объект
void BaseCamera::Restart()
{
	DelUpdate(&BaseCamera::Work);

	Init();

	ReCreate();
}

//Получить матрицу объекта
Matrix & BaseCamera::GetMatrix(Matrix & mtx)
{
	//Matrix offset_matrix;

	if (attach_mode==1)
	{
		offset_matrix = cam_offset;		
	}
	else
	if (attach_mode==2 && attachedobjID.Len())
	{		
		if(!attachedobj.Validate())
		{
			FindObject(ConstString(attachedobjID.c_str(), attachedobjIDhash, attachedobjID.Len()), attachedobj);
		}

		if (attachedobj.Validate())
		{
			attachedobj.Ptr()->GetMatrix(offset_matrix);
		}		
	}	

	Vector pos, trg, up(0.0f, 1.0f, 0.0f);
	GetCameraPosition(pos);

	pos = pos * offset_matrix;

	if(!GetCameraTarget(pos, trg, up))
	{
		trg = pos + Vector(0.0f, 0.0f, 1.0f);

		trg = trg * offset_matrix;
	}	
	
	if(!mtx.BuildView(pos, trg, up))
	{
		if(!mtx.BuildView(pos, trg, Vector(0.0f, 1.0f, 0.0f)))
		{
			if(!mtx.BuildView(pos, trg, Vector(0.0f, 0.0f, -1.0f)))
			{
				if(!mtx.BuildView(pos, trg, Vector(1.0f, 0.0f, 0.0f)))
				{
					mtx.SetIdentity();
				}
			}
		}
	}
	
	mtx.Inverse();
	
	return blender.GetMatrix(mtx,fCurFOV);
}

//Обработчик команд для объекта
void BaseCamera::Command(const char * id, dword numParams, const char ** params)
{
	if(!id) return;
	
	if(string::IsEqual(id, "Target"))
	{
		target.Reset();
		
		if(numParams > 0)
		{
			targetID.Set(params[0]);
			LogicDebug("Target: new target is \"%s\"", params[0]);
		}
		else
		{
			targetID.Empty();
			LogicDebug("Target: new target is \"\"");
		}

		if( targetID.NotEmpty() )
			FindObject(targetID,target);
	}
	else
	if(string::IsEqual(id, "Attach"))
	{
		attachedobj.Reset();
						
		if(numParams > 0)
		{
			attach_mode = 2;
			attachedobjID = params[0];
			attachedobjIDhash = string::HashNoCase(attachedobjID.c_str());
			LogicDebug("Attach: camera attached to \"%s\"", params[0]);
		}
		else
		{
			attach_mode = 0;
			attachedobjID.Empty();
			LogicDebug("Target: camera deattched \"\"");
		}
	}
	else
	if(string::IsEqual(id, "Teleport"))
	{
		attachedobj.Reset();

		attach_mode = 1;

		if(numParams > 0)
		{
			attach_mode = 1;			

			MOSafePointer mo;
			FindObject(ConstString(params[0]), mo);

			if (mo.Ptr())
			{
				Matrix mat;

				mo.Ptr()->GetMatrix(mat);

				//cam_offset.SetIdentity();

				cam_offset = mat;
				//cam_offset.Inverse();
				//cam_offset.pos = mat.pos;

				LogicDebug("Attach: camera teleported to \"%s\"", params[0]);
			}
			else
			{
				attach_mode = 0;
				cam_offset = 0.0f;			
				LogicDebug("Teleport: offset reseted \"\"");
			}
		}
		else
		{
			attach_mode = 0;
			cam_offset = 0.0f;			
			LogicDebug("Teleport: offset reseted \"\"");
		}
	}
	else
	if(string::IsEqual(id, "Reset"))
	{
		target.Reset();
		Reset();
		LogicDebug("Reset");
	}
	else
	if(string::IsEqual(id, "shockRND"))
	{
		if(numParams >= 3)
		{
			char * tmp = null;
			float amp = (float)strtod(params[0], &tmp);
			float time = (float)strtod(params[1], &tmp);
			float itencity = (float)strtod(params[2], &tmp);
			//LogicDebug("ShockRND: amplitude = %f, time = %f, itencity = %f", amp, time, itencity);
			blender.Shock(MissionCamerasBlender::SH_ShockRND,amp, time, itencity);
		}
		else
		{
			LogicDebugError("ShockRND: invalidate params");
		}
	}
	else
	if(string::IsEqual(id, "shockRND1"))
	{
		//LogicDebug("ShockRND1: amplitude = 0.1, time = 0.2, itencity = 40");
		blender.Shock(MissionCamerasBlender::SH_ShockRND,0.1f, 0.2f, 40.0f);
	}
	else
	if(string::IsEqual(id, "shockRND2"))
	{
		//LogicDebug("ShockRND2: amplitude = 0.1, time = 0.4, itencity = 45");
		blender.Shock(MissionCamerasBlender::SH_ShockRND,0.1f, 0.4f, 45.0f);
	}
	else
	if(string::IsEqual(id, "shockRND3"))
	{
		//LogicDebug("ShockRND3: amplitude = 0.1, time = 0.6, itencity = 50");
		blender.Shock(MissionCamerasBlender::SH_ShockRND,0.1f, 0.6f, 50.0f);
	}
	else
	if(string::IsEqual(id, "shockRND4"))
	{
		//LogicDebug("ShockRND4: amplitude = 0.1, time = 0.75, itencity = 55");
		blender.Shock(MissionCamerasBlender::SH_ShockRND,0.1f, 0.75f, 55.0f);
	}
	else
	if(string::IsEqual(id, "shockRND5"))
	{
		//LogicDebug("ShockRND5: amplitude = 0.15, time = 0.85, itencity = 70");
		blender.Shock(MissionCamerasBlender::SH_ShockRND,0.15f, 0.85f, 70.0f);
	}
	else
	if(string::IsEqual(id, "shockRND_H"))
	{
		if(numParams >= 3)
		{
			char * tmp = null;
			float amp = (float)strtod(params[0], &tmp);
			float time = (float)strtod(params[1], &tmp);
			float itencity = (float)strtod(params[2], &tmp);
			//LogicDebug("ShockRND_H: amplitude = %f, time = %f, itencity = %f", amp, time, itencity);
			blender.Shock(MissionCamerasBlender::SH_ShockRND_H,amp, time, itencity);
		}
		else
		{
			LogicDebugError("ShockRND_H: invalidate params");
		}
	}
	else
	if(string::IsEqual(id, "shockRND_V"))
	{
		if(numParams >= 3)
		{
			char * tmp = null;
			float amp = (float)strtod(params[0], &tmp);
			float time = (float)strtod(params[1], &tmp);
			float itencity = (float)strtod(params[2], &tmp);
			//LogicDebug("ShockRND_V: amplitude = %f, time = %f, itencity = %f", amp, time, itencity);
			blender.Shock(MissionCamerasBlender::SH_ShockRND_V,amp, time, itencity);
		}
		else
		{
			LogicDebugError("ShockRND_V: invalidate params");
		}
	}
	else		
	if(string::IsEqual(id, "shockPolar1"))
	{
		//LogicDebug("ShockPolar1: amplitude = 0.125, time = 0.5, itencity = 80");
		blender.Shock(MissionCamerasBlender::SH_ShockPolar,0.125f, 0.5f, 80.0f);
	}
	else
	if(string::IsEqual(id, "shockPolar2"))
	{
		//LogicDebug("ShockPolar2: amplitude = 0.15, time = 0.5, itencity = 100");
		blender.Shock(MissionCamerasBlender::SH_ShockPolar,0.15f, 0.5f, 100.0f);
	}
	else
	if(string::IsEqual(id, "shockPolar3"))
	{
		//LogicDebug("ShockPolar3: amplitude = 0.15, time = 0.75, itencity = 100");
		blender.Shock(MissionCamerasBlender::SH_ShockPolar,0.15f, 0.75f, 100.0f);
	}
	else
	if(string::IsEqual(id, "shockPolar4"))
	{
		//LogicDebug("ShockPolar4: amplitude = 0.2, time = 1.0, itencity = 100");
		blender.Shock(MissionCamerasBlender::SH_ShockPolar,0.2f, 1.0f, 100.0f);
	}
	else
	if(string::IsEqual(id, "shockPolar5"))
	{
		//LogicDebug("ShockPolar5: amplitude = 0.2, time = 1.25, itencity = 120");
		blender.Shock(MissionCamerasBlender::SH_ShockPolar,0.2f, 1.25f, 120.0f);
	}
	else
	if(string::IsEqual(id, "shockPolar"))
	{
		if(numParams >= 3)
		{
			char * tmp = null;
			float amp = (float)strtod(params[0], &tmp);
			float time = (float)strtod(params[1], &tmp);
			float itencity = (float)strtod(params[2], &tmp);
			LogicDebug("ShockPolar: amplitude = %f, time = %f, itencity = %f", amp, time, itencity);
			blender.Shock(MissionCamerasBlender::SH_ShockPolar,amp, time, itencity);
		}
		else
		{
			LogicDebugError("ShockPolar: invalidate params");
		}
	}
	else
	if(string::IsEqual(id, "shockPolar_V"))
	{
		if(numParams >= 3)
		{
			char * tmp = null;
			float amp = (float)strtod(params[0], &tmp);
			float time = (float)strtod(params[1], &tmp);
			float itencity = (float)strtod(params[2], &tmp);
			LogicDebug("ShockPolar: amplitude = %f, time = %f, itencity = %f", amp, time, itencity);
			blender.Shock(MissionCamerasBlender::SH_ShockPolar_V,amp, time, itencity);
		}
		else
		{
			LogicDebugError("ShockPolar_V: invalidate params");
		}
	}
	else
	if(string::IsEqual(id, "shockPolar_H"))
	{
		if(numParams >= 3)
		{
			char * tmp = null;
			float amp = (float)strtod(params[0], &tmp);
			float time = (float)strtod(params[1], &tmp);
			float itencity = (float)strtod(params[2], &tmp);
			LogicDebug("ShockPolar_H: amplitude = %f, time = %f, itencity = %f", amp, time, itencity);
			blender.Shock(MissionCamerasBlender::SH_ShockPolar_H,amp, time, itencity);
		}
		else
		{
			LogicDebugError("ShockPolar_H: invalidate params");
		}
	}
	else	
	if(string::IsEqual(id, "resetShock"))
	{		
		blender.ResetShock();
	}
	else	
	if(string::IsEqual(id, "blendtime"))
	{
		if(numParams >= 1)
		{
			char * tmp = null;

			float fBlendTime = (float)strtod(params[0], &tmp);
			
			LogicDebug("blendtime = %f", fBlendTime);

			blender.Init(fBlendTime);
		}
		else
		{
			LogicDebugError("blendtime: invalidate params");
		}
	}	
	else
	if(string::IsEqual(id, "setFov"))
	{
		if(numParams >= 2)
		{
			char * tmp = null;
			
			fovChgTarget = (float)strtod(params[0], &tmp) * PI/180.0f;
			fovChgTime = fovChgCurTime = (float)strtod(params[1], &tmp);
			fovLast = fov;			

			if (fovChgCurTime<0.01f)
			{
				fovChgCurTime = -1.0f;
				fovChgTime = 0.001f;
				fov = fovChgTarget;
				fCurFOV = fov;
			}
			
			LogicDebug("setFov = %f", fovChgTarget);			
		}
		else
		{
			LogicDebugError("setFov: invalidate params");
		}
	}	
	else
	if(string::IsEqual(id, "AutoFocusParams"))
	{
		if(numParams >= 5)
		{
			char * tmp = null;
			
			fNeededDist = (float)strtod(params[0], &tmp);
			fMinAngle = (float)strtod(params[1], &tmp);
			fMaxAngle = (float)strtod(params[2], &tmp);
			fMinSpeedAngle = (float)strtod(params[3], &tmp);
			fMaxSpeedAngle = (float)strtod(params[4], &tmp);
			
			LogicDebug("AutoFocusParams: Dist = %f, MinAngle = %f, MaxAngle = %f, MinSpeedAngle = %f, MaxSpeedAngle = %f",
					   fNeededDist, fMinAngle, fMaxAngle, fMinSpeedAngle, fMaxSpeedAngle);
		}
		else
		{
			LogicDebugError("AutoFocusParams: invalidate params");
		}
	}
	else
	if(string::IsEqual(id, "AutoFocusOn"))
	{
		AutoZoom = true;

		LogicDebug("AutoFocusOn");
	}
	else	
	if(string::IsEqual(id, "AutoFocusOff"))
	{
		AutoZoom = false;

		LogicDebug("AutoFocusOff");
	}
	else
	if(string::IsEqual(id, "CrazyOn"))
	{
		blender.ActiveCrazy(true);

		LogicDebug("CrazyOn");
	}
	else	
	if(string::IsEqual(id, "CrazyOff"))
	{
		blender.ActiveCrazy(false);

		LogicDebug("CrazyOff");
	}
	else	
	if(string::IsEqual(id, "CrazyParams"))
	{
		if(numParams >= 2)
		{
			char * tmp = null;

			float waveMAXAmp = (float)strtod(params[0], &tmp);
			float waveSpeed = (float)strtod(params[1], &tmp);			

			blender.SetCrazyParams(waveMAXAmp,waveSpeed);

			LogicDebug("CrazyParams: Dist = %f, MinAngle = %f",
					   waveMAXAmp, waveSpeed);
		}
		else
		{
			LogicDebugError("CrazyParams: invalidate params");
		}
	}
	else
	{
		if (GetCameraController())
		{
			if (GetCameraController()->CheckCommand(id))
			{			
				GetCameraController()->Command(id, numParams, params);
			}
		}
	}
}

bool BaseCamera::CheckCommand(const char * id)
{
	if(!id) return false;

	if(string::IsEqual(id, "Target"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "Reset"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "Attach"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "Teleport"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockRND"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockRND1"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockRND2"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockRND3"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockRND4"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockRND5"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockRND_H"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockRND_V"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockPolar"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockPolar1"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockPolar2"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockPolar3"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockPolar4"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockPolar5"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockPolar_V"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "shockPolar_H"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "resetShock"))
	{
		return true;
	}
	else	
	if(string::IsEqual(id, "blendtime"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "setFov"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "AutoFocusParams"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "AutoFocusOn"))
	{
		return true;
	}
	else	
	if(string::IsEqual(id, "AutoFocusOff"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "CrazyOn"))
	{
		return true;
	}
	else	
	if(string::IsEqual(id, "CrazyOff"))
	{
		return true;
	}
	else	
	if(string::IsEqual(id, "CrazyParams"))
	{
		return true;
	}

	return false;
}

//Инициализировать объект
bool BaseCamera::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&BaseCamera::EditModeWork, ML_CAMERAMOVE_FREE + 1);
	//Создаём модельку камеры
	if(!model)
	{
		IGMXService * geom = (IGMXService *)api->GetService ("GMXService");
		if(!geom) return false;
		geom->SetTexturePath("editmode");
		model = geom->CreateGMX("editor\\CameraModel.gmx", &Mission().Animation(), &Mission().Particles(), &Mission().Sound());
		geom->SetTexturePath("");
	}
	
	Create(reader);

	return true;
}

//Обновить параметры
bool BaseCamera::EditMode_Update(MOPReader & reader)
{
	Create(reader);

	return true;
}

//Получить размеры описывающего ящика
void BaseCamera::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	if(model)
	{
		min = model->GetLocalBound().vMin - 0.1f;
		max = model->GetLocalBound().vMax + 0.1f;
		return;
	}
	
	min = -0.3f;
	max = 0.3f;
}

#ifndef MIS_STOP_EDIT_FUNCS
//Выделить объект
void BaseCamera::EditMode_Select(bool isSelect)
{
	MissionObject::EditMode_Select(isSelect);
	if(!EditMode_IsOn()) return;
	
	if(isSelect)
	{
		Reset();
		SetUpdate(&BaseCamera::EditModeDrawSelected, ML_ALPHA5 + 10);
	}
	else
	{
		DelUpdate(&BaseCamera::EditModeDrawSelected);
	}
}
#endif

//Активировать/деактивировать объект
void BaseCamera::Activate(bool isActive)
{
	blender.ResetShock();
	// Vano: добавил отключение crazy cam, а то оно могло остаться навсегда, если между Crazy(true)..Crazy(false) было переключение камер
	blender.ResetCrazy();

	if(IsActive() == isActive) return;

	MissionObject::Activate(isActive);
	    
	// говорим контроллеру камер о своей активации
	if(isActive)
	{
		MGIterator* iter;

		iter = &Mission().GroupIterator(CAMERA_CONTROLLER_GROUP, _FL_);

		bool NeedCamBlend = true;

		if (!iter->IsDone())
		{		
			CameraController* CamController = (CameraController*)iter->Get();			

			NeedCamBlend = !CamController->IsHasentActiveCam();

			CamController->SetActiveCamera(this);
		}

		iter->Release();

		LogicDebug("Activate");

		LogicDebug("Reset");
		Reset();

		if(!EditMode_IsOn())
		{			
			if (NeedCamBlend)
			{
				Matrix viewmatrix = Render().GetView();
				viewmatrix.Inverse();
				
				Matrix mat = Mission().GetSwingMatrix();
				mat.Inverse();

				viewmatrix = mat * viewmatrix;
				viewmatrix.Inverse();

	
				blender.Activate(viewmatrix, Render().GetProjection(),
					(float)Render().GetViewport().Width,
					(float)Render().GetViewport().Height,
					Render().GetWideScreenAspectWidthMultipler());
			}

			SetUpdate(&BaseCamera::Work, level);
			//Work(0, level);
		}		

		/*if(!EditMode_IsOn())
		{
			SetUpdate(&BaseCamera::EditModeDrawSelected, ML_ALPHA5 + 10);
		}*/
	}
	else
	{
		MGIterator* iter;

		iter = &Mission().GroupIterator(CAMERA_CONTROLLER_GROUP, _FL_);

		if (!iter->IsDone())
		{		
			CameraController* CamController = (CameraController*)iter->Get();			

			CamController->FindHighestActiveCam();
		}

		iter->Release();

		LogicDebug("Deactivate");
		
		if(!EditMode_IsOn())
		{
			DelUpdate(&BaseCamera::Work);
		}

		debugdraw_enbled = false;
		/*if(!EditMode_IsOn())
		{
			DelUpdate(&BaseCamera::EditModeDrawSelected);
		}*/
	}
}

//Автоматический расчет FOV
void BaseCamera::AutoFocusCam(float dltTime)
{
	float fNeededFOV = fov;

	if (AutoZoom)
	{	
		Vector cam_pos;

		GetCameraPosition(cam_pos);

		{
			if(targetID.NotEmpty())
			{
				if(!target.Validate())
				{
					FindObject(targetID, target);
				}
			}
			
			if(target.Validate())
			{			
				Matrix mtx;
				target.Ptr()->GetMatrix(mtx);
				Vector target_pos = mtx*targetPos;

				float fDist=(cam_pos-target_pos).GetLength();

				fNeededFOV = fNeededDist / fDist * 2;

				if (fNeededFOV>PI * 0.55f) fNeededFOV = PI * 0.55f;
	
				if (fNeededFOV>fCurFOV)
				{
					fNeededFOV=fCurFOV;
				}
			}	
		}
	}

	float tmp = (fNeededFOV - fCurFOV);

	float delta = tmp;
	float k = SIGN(delta);

	delta = fabs(delta);

	if (delta< fMinAngle*PI/180) delta = fMinAngle*PI/180;
	if (delta> fMaxAngle*PI/180) delta = fMaxAngle*PI/180;

	float fAngleSpeed = (delta - fMinAngle*PI/180)/(fMaxAngle*PI/180 - fMinAngle*PI/180);

	fAngleSpeed = (fMinSpeedAngle + (fMaxSpeedAngle - fMinSpeedAngle) * fAngleSpeed) * PI/180;
	
	if (dltTime * fAngleSpeed > tmp * k)
	{
		fCurFOV = fNeededFOV;
	}
	else
	{
		fCurFOV += dltTime * fAngleSpeed * k;
	}
}

/*
void _cdecl BaseCamera::CameraDebugDraw(float dltTime, long level)
{
	if (IsActive() == false)
	{
		return;
	}

	Render().Print(100, 100, 0xFFFFFFFF, "%s, fov %f, original fov from designers %f (%f)", GetObjectID().c_str(), (debugFovToPrint/PI)*180.0f, (debugInputFov/PI)*180.0f, (debugCamFov/PI)*180.0f);
}
*/

//Работа
void _cdecl BaseCamera::Work(float dltTime, long level)
{
	if (fovChgCurTime>0.0f)
	{
		fovChgCurTime -= dltTime;

		if (fovChgCurTime<0.0f)
		{
			fovChgCurTime = -1.0f;
			fov = fovChgTarget;
		}
		else
		{
			fov = fovLast + (fovChgTarget - fovLast)*(1.0f - fovChgCurTime/fovChgTime);
		}
	}

	WorkUpdate(dltTime);
	blender.Update(dltTime);
	
	Matrix mat(true);

	mat = Mission().GetSwingMatrix()*GetMatrix(Matrix());

	Sound().SetListenerMatrix(mat);

	mat.Inverse();

	Render().SetView(mat);
	//Render().SetView(mat2) ;//(Mission().GetSwingMatrix()*GetMatrix(Matrix())).BuildViewFromObject());		
	
	AutoFocusCam(dltTime);
	

	//float wideConst = Render().GetWideScreenAspectFovMultipler();

	float cameraFov16to9 = GetCameraFoV();

	//1.225f это коэфицент что бы не сломались старые fov он не нужен, просто что бы заново все fov
	//дизайнерам не настраивать, без него fov просто можно брать из Maya/Max при камере 16:9
	cameraFov16to9 = cameraFov16to9 * 1.225f;

	//debugCamFov = cameraFov16to9;
	float fPerspective = blender.GetFov(cameraFov16to9);

	//fPerspective *= wideConst;

	//float fHeight = (float)Render().GetViewport().Width * ((3.0f / 4.0f) / wideConst);
	//float fWidth = (float)Render().GetViewport().Width;


	//Max: математика для корректировки ФОВ для текущего соотношения сторон
	//картинки совпадали 16/9 и 4.3 пиксель в пиксель. Только обрезались поля.

	//debugInputFov = fPerspective;

	float fWidth = (float)Render().GetViewport().Width;
	float fHeight = (float)Render().GetViewport().Height;
	fPerspective = recalculateFov(fWidth, fHeight, Render().GetWideScreenAspectWidthMultipler(), fPerspective);

/*
	float _fWidth = (float)Render().GetViewport().Width;
	float _fHeight = (float)Render().GetViewport().Height;


	float testFov = invRecalculateFov(_fWidth, _fHeight, Render().GetWideScreenAspectWidthMultipler(), fPerspective);

	assert(fabsf(testFov - debugInputFov) < 0.0001f);
*/

/*
	float fWidth = (float)Render().GetViewport().Width;
	float fHeight = (float)Render().GetViewport().Height;

	//float aspect = fWidth/fHeight;//wideConst;//4.0f/3.0f;
	float aspect = (1.0f/Render().GetWideScreenAspectWidthMultipler()) * (fWidth/fHeight);
	
	float kCorrection = Clampf(aspect*9.0f/16.0f);
	float tg = tan(fPerspective*0.5f);
	float ktg = tg*kCorrection;
	float ang = atan(ktg)*2.0f;	
	fHeight = fWidth*(9.0f/16.0f)*fPerspective/ang;
	fPerspective = ang;
*/

	//debugFovToPrint = fPerspective;
	//Render().Print(100, 100, 0xffffffff, "Current FOV = %f", (fPerspective/PI)*180.0f);
	Render().SetPerspective(fPerspective, fWidth, fHeight, 0.1f, 4000.0f);

	
/*
	Matrix prj = Render().GetProjection();
	float fovFromProjMatrix = float(fabs(atan(1.0f/prj.m[0][0])*2.0));	
	float actFOV = invRecalculateFov(_fWidth, _fHeight, Render().GetWideScreenAspectWidthMultipler(), fovFromProjMatrix);
	assert(fabsf(actFOV - debugInputFov) < 0.0001f);
*/



	if (!EditMode_IsOn())
	{		
		if (api->DebugKeyState('Z','X'))
		{
			debugdraw_enbled = !debugdraw_enbled;

			Sleep(200);

			if (debugdraw_enbled)
			{
				SetUpdate(&BaseCamera::DebugDraw,ML_ALPHA5 + 10);
			}
			else
			{		
				DelUpdate(&BaseCamera::DebugDraw);
			}		
		}		
	}	
}

//Работа в режиме редактирования
void _cdecl BaseCamera::EditModeWork(float dltTime, long level)
{
	
	if(!EditMode_IsVisible() && !EditMode_IsSelect()) return;
	
	if(EditMode_IsSelect() && isPreview)
	{
		Work(dltTime, level);
	}
	else
	{
		if (Mission().EditMode_IsAdditionalDraw() && bDrawCamera)
		{
			Matrix mtx;
			GetMatrix(mtx);

			if(model)
			{	
				model->SetTransform(mtx);
				model->Draw();
			}
			else
			{
				Render().DrawSphere(mtx.pos, 0.3f);
			}
		}
	}
}

//Отрисовка в выделеном режиме
void _cdecl BaseCamera::EditModeDrawSelected(float dltTime, long level)
{
	//if (debugdraw_enbled) return;

	if(!isPreview)
	{
		SelectedDraw(dltTime);
	}

	SelectedUpdate(dltTime);
}

//Обновить состояние для предпросмотра
void BaseCamera::SelectedUpdate(float dltTime)
{
}

//Нарисовать дополнительную информацию нри селекте
void BaseCamera::SelectedDraw(float dltTime)
{
	if (debugdraw_enbled) return;

	if (!bDrawCamera) return;

	Vector v[12];
	float r = 30.0f;
	float xy = r*sinf(GetCameraFoV()*0.5f);
	float z = r*cosf(GetCameraFoV()*0.5f);
	v[0] = Vector(0.0f, 0.0f, 0.0f);
	v[1] = Vector(xy, -xy, z);
	v[2] = Vector(-xy, -xy, z);
	v[3] = Vector(0.0f, 0.0f, 0.0f);
	v[4] = Vector(xy, -xy, z);
	v[5] = Vector(xy, xy, z);
	v[6] = Vector(0.0f, 0.0f, 0.0f);
	v[7] = Vector(xy, xy, z);
	v[8] = Vector(-xy, xy, z);
	v[9] = Vector(0.0f, 0.0f, 0.0f);
	v[10] = Vector(-xy, xy, z);
	v[11] = Vector(-xy, -xy, z);
	Matrix mtx;
	GetMatrix(mtx);
	Render().SetWorld(mtx);

	ShaderId ShowCameraFrustum_id;
	Render().GetShaderId("ShowCameraFrustum", ShowCameraFrustum_id);

	Render().DrawPrimitiveUP(ShowCameraFrustum_id, PT_TRIANGLELIST, 4, v, sizeof(Vector));
	Render().DrawMatrix(mtx);
}


//Получить указатель на контролер камеры
CameraController * BaseCamera::GetCameraController()
{
	MOSafePointerType<CameraController> CamController;
	static const ConstString objectId("Camera Controller");
	FindObject(objectId, CamController.GetSPObject());
	
	if(!CamController.Ptr() && !EditMode_IsOn())
	{
		MOPWriter writer('0.00', "Camera Controller");
		Mission().CreateObject(CamController.GetSPObject(), "CameraController", objectId, EditMode_IsOn());
		
		if(!CamController.Ptr())
		{
			api->Trace("Character can't created. CameraController not be found or created");			
		}
	}
	
	return CamController.Ptr();
}

void _cdecl BaseCamera::DebugDraw(float dltTime, long level)
{
	EditModeDrawSelected(dltTime, level);	

	RS_SPRITE spr[4];

	float fPosX = -1.0f;
	float fPosY = 1.0f-0.075f;

	spr[0].vPos = Vector (fPosX,      fPosY+0.075f, 0.0f);
	spr[1].vPos = Vector (fPosX+1.0f, fPosY+0.075f, 0.0f);
	spr[2].vPos = Vector (fPosX+1.0f, fPosY, 0.0f);
	spr[3].vPos = Vector (fPosX,      fPosY, 0.0f);

	spr[0].tv = 0.0f;
	spr[0].tu = 0.0f; 
	spr[0].dwColor = 0xAA000000;

	spr[1].tv = 0.0f;
	spr[1].tu = 1.0f; 
	spr[1].dwColor = 0xAA000000;

	spr[2].tv = 1.0f;
	spr[2].tu = 1.0f;  
	spr[2].dwColor = 0xAA000000;

	spr[3].tv = 1.0f;
	spr[3].tu = 0.0f;  	
	spr[3].dwColor = 0xAA000000;

	Render().DrawSprites(NULL,spr, 1);

	Render().Print(0,0,0xffff00ff,"%s fov - %4.3f cur_fov - %4.3f",GetObjectID().c_str(),Rad2Deg(fov),Rad2Deg(fCurFOV));
}