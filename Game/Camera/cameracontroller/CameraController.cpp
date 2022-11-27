#include "..\..\..\common_h/mission/Mission.h"
//#include "..\..\..\Common_h\ICharactersArbiter.h"

#include "CameraController.h"

//Конструктор
CameraController::CameraController()
{		
	ActiveCamera = NULL;
	IsAnotherCamController=false;

	PostProcessService* pPostProcess = (PostProcessService*)api->GetService("PostProcessService");	

	if (pPostProcess)
	{		
		splash = pPostProcess->GetSplashFilter();
	}
	else
	{
		splash = NULL;
	}

	HasentActiveCam = true;
}

//Деструктор
CameraController::~CameraController()
{			
	cam_iter->Release();

	Console().UnregisterCommand("showcameras");	
}


//Создание объекта
bool CameraController::Create(MOPReader & reader)
{
	cam_iter = &Mission().GroupIterator(CAMERAS_GROUP, _FL_);
	
	EditMode_Update (reader);

	// не создан ли до этого другой контролллер камеры
	{	
		MGIterator* iter;

		iter = &Mission().GroupIterator(CAMERA_CONTROLLER_GROUP, _FL_);

		if (!iter->IsDone())
		{		
			api->Trace ("Warning : Another Camera_Controller was created before");
			IsAnotherCamController = true;
		}

		iter->Release();
	}

	Registry(CAMERA_CONTROLLER_GROUP);

	if (!IsAnotherCamController)
	{	
		FindHighestActiveCam();
	}

	if (!EditMode_IsOn())
	{
		Console().RegisterCommand("showcameras", "show active cameras", this, (CONSOLE_COMMAND)&CameraController::Console_ShowActiveCameras);
	}		
	
	return true;
}

void CameraController::FindHighestActiveCam()
{
	ActiveCamera=NULL;
	
	if (cam_iter)
	{	
		cam_iter->Reset();

		for (;!cam_iter->IsDone();cam_iter->Next())
		{	
			BaseCamera* Camera = (BaseCamera*)cam_iter->Get();

			if (Camera->GetCameraLevel()>GetActiveCameraLevel()&&Camera->IsActive())
			{
				SetActiveCamera(Camera);
			}
		}
	}	
}

//Обновление параметров
bool CameraController::EditMode_Update(MOPReader & reader)
{
	SetUpdate((MOF_UPDATE)&CameraController::Realize, ML_GUI1);

	return true;
}

void CameraController::AddBloodPuff()
{
	if( splash )
	{
		splash->AddBloodPuff(this);
	}
}

void _cdecl CameraController::Realize(float fDeltaTime, long level)
{
	if( splash )
	{
		splash->Update(this,fDeltaTime);
	}

	if (IsAnotherCamController) return;	

	if (ActiveCamera==NULL)
	{
		FindHighestActiveCam();
		if (ActiveCamera==NULL) return;
	}

	if (!ActiveCamera->IsActive())
	{
		FindHighestActiveCam();
	}	

		

	/*
	if (GetAsyncKeyState('L')<0)
	{
		MissionObject* pChrPlayer;

		pChrPlayer = Mission().Player();

		if (pChrPlayer)
		{	
			pChrPlayer->Restart();
		}

		Sleep(200);
	}


	/*
	if (GetAsyncKeyState('L')<0)
	{
		MissionObject* pChrPlayer;
	
		pChrPlayer = Mission().Player();

		if (pChrPlayer)
		{		
			const char* params[2];

			//params[0] = "20 Salor";
			//params[0] = "21 Boatswain";
			params[0] = "22 Captain";
			pChrPlayer->Command("Achievement",1,params);			
		}

		//Sleep(200);
	}

	/*if (GetAsyncKeyState('L')<0)
	{
		MissionObject* pChrPlayer;
	
		pChrPlayer = Mission().FindObject("FatGuy");
	
		if (pChrPlayer)
		{		
			const char* params[2];

			pChrPlayer->Command("respawn",0,params);

		}

		Sleep(200);
	}

	if (GetAsyncKeyState('L')<0)
	{
		MissionObject* pChrPlayer;

		pChrPlayer = Mission().Player();

		if (pChrPlayer)
		{	
			pChrPlayer->Show(!pChrPlayer->IsShow());
		}

		Sleep(200);
	}

	if (GetAsyncKeyState('N')<0)
	{
		MissionObject* pChrPlayer;

		pChrPlayer = Mission().Player();

		if (pChrPlayer)
		{	
			pChrPlayer->Command("enableactor",0,null);
		}

		Sleep(200);
	}	

	if (GetAsyncKeyState('M')<0)
	{
		MissionObject* pChrPlayer;

		pChrPlayer = Mission().Player();

		if (pChrPlayer)
		{	
			pChrPlayer->Command("disableactor",0,null);
		}

		Sleep(200);
	}

	if (GetAsyncKeyState('N')<0)
	{
		MissionObject* pChrPlayer;

		pChrPlayer = Mission().FindObject("Chainman0");

		if (pChrPlayer)
		{		
			pChrPlayer->Restart();
		}		

		Sleep(200);
	}

	if (GetAsyncKeyState('N')<0)
	{
		MissionObject* pChrPlayer;

		pChrPlayer = Mission().FindObject("Chainman0");

		if (pChrPlayer)
		{	
			pChrPlayer->Command("enableactor",0,null);
		}

		Sleep(200);
	}

	if (GetAsyncKeyState('M')<0)
	{
		MissionObject* pChrPlayer;

		pChrPlayer = Mission().FindObject("Chainman2");

		if (pChrPlayer)
		{	
			pChrPlayer->Restart();
		}

		Sleep(200);
	}

	if (GetAsyncKeyState('L')<0)
	{			
		MOSafePointer ptr;
		Mission().FindObject("Grenadier_1",ptr);
	
		if (ptr.Ptr())
		{					
			ptr.Ptr()->Show(false);

		}

		Sleep(200);
	}

	if (GetAsyncKeyState('N')<0)
	{
		MOSafePointer ptr;
		Mission().FindObject("Player",ptr);
	
		if (ptr.Ptr())
		{	
			ptr.Ptr()->Command("enableactor",0,null);
		}

		Sleep(200);
	}*/	
}


//Обработчик команд для объекта
void CameraController::Command(const char * id, dword numParams, const char ** params)
{	
	if(!id) return;
	
	if(string::IsEqual(id, "BloodPuff"))
	{
		int num = 1;

		if (numParams>0)
		{
			char * pos = null;
			num = (int)strtod(params[0], &pos);

			if (num<1) num = 1;
		}

		for (int i=0;i<num;i++)
		{
			AddBloodPuff();
		}		

		LogicDebug("BloodPuff");
	}
	else	
	if(string::IsEqual(id, "BloodRemove"))
	{
		if (splash)
		{
			splash->Release(this);
		}
		
		LogicDebug("BloodPuff");
	}
	else	
	{
		if (cam_iter)
		{		
			cam_iter->Reset();

			for (;!cam_iter->IsDone();cam_iter->Next())			
			{	
				BaseCamera* camera = (BaseCamera*)cam_iter->Get();

				if (!camera->IsActive()) continue;

				if (camera->CheckCommand(id))
				{			
					camera->Command(id, numParams, params);
				}							
			}		
		}
	}	
}

bool CameraController::CheckCommand(const char * id)
{
	if(!id) return false;
	
	if(string::IsEqual(id, "BloodPuff"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "EnableMotionBlur"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "DisableMotionBlur"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "MotionBlurChgSpeed"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "MotionBlurParams"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "BloodRemove"))
	{
		return true;
	}	

	return false;

}

void CameraController::SetActiveCamera(BaseCamera* NewActiveCamera)
{
	if (IsAnotherCamController) return;

	if (HasentActiveCam)
	{
		HasentActiveCam = false;		
	}

	ActiveCamera = NewActiveCamera;
}

long CameraController::GetActiveCameraLevel()
{
	if (ActiveCamera==NULL) return -99999999;

	return ActiveCamera->GetCameraLevel();
}

void _cdecl CameraController::Console_ShowActiveCameras(const ConsoleStack & params)
{	
	if (!cam_iter) return;

	cam_iter->Reset();

	bool isAnyActiveCam = false;
	
	for (;!cam_iter->IsDone();cam_iter->Next())
	if (cam_iter->Get()->IsActive())
	{
		if (!isAnyActiveCam)
		{
			Console().Trace(COL_CMD_OUTPUT, "  Follow Cameras is Active:");
		}

		isAnyActiveCam = true;
			
		BaseCamera* camera = (BaseCamera*)cam_iter->Get();

		if (isAnyActiveCam)
		{
			Console().Trace(COL_CMD_OUTPUT, "    Camera \"%s\" is active",camera->GetObjectID().c_str());
		}

	}

	if (!isAnyActiveCam)
	{
		Console().Trace(COL_CMD_OUTPUT, "  There is no any Active Camera");
	}
}

void CameraController::GetTargetPoint(Vector& pt)
{
	if (ActiveCamera)
	{
		ActiveCamera->GetTargetPos(pt);
	}
}

void CameraController::GetBlenderMatrix(Matrix& shock, float fov)
{
	if (ActiveCamera)
	{
		ActiveCamera->GetBlenderMatrix(shock,fov);
	}
}

bool CameraController::AllowZooming()
{
	if (ActiveCamera)
	{
		return ActiveCamera->AllowZooming();
	}

	return false;
}

const char * CameraController::comment = 
"Control Last Active camera";



MOP_BEGINLISTCG(CameraController, "", '1.00', 100, CameraController::comment, "Cameras")	

MOP_ENDLIST(CameraController)



