
#include "FreeCamera.h"

//============================================================================================
//FreeCameraModule
//============================================================================================

#define SENSITIVITY		0.1f
#define FOV				1.285f

INTERFACE_FUNCTION
CREATE_CLASS(FreeCamera)


FreeCameraModule::FreeCameraModule()
{
	render = null;
	ctr = null;
	pos = Vector(0.0f, 9.0f, -5.0f);
	ang = Vector(0.0f, 0.0f, 0.0f);
	fov = FOV;	
}

FreeCameraModule::~FreeCameraModule()
{
	RELEASE(ctr);
}

bool FreeCameraModule::Init()
{
	render = (IRender *)api->GetService("dx9render");
	Assert(render);	
	IControlsService * ctrlsrv = (IControlsService *)api->GetService("ControlsService");
	Assert(ctrlsrv);
	ctr = ctrlsrv->CreateInstance(_FL_);
	Assert(ctr);	
	return true;
}

//Перемещение камеры
void FreeCameraModule::Move(float dltTime)
{
	if(!ctr)
	{
		return;
	}
	//Ограничим временной отрезок
	if(dltTime > 0.1f) dltTime = 0.1f;
	//Углы
	ctrl = pref; ctrl += "FreeCamera_Turn_H";
	ang.y += ctr->GetControlStateFloat(ctrl) * 0.2f;
	ctrl = pref; ctrl += "FreeCamera_Turn_V";
	ang.x -= ctr->GetControlStateFloat(ctrl) * 0.2f;
	if(ang.x < -PI*0.5f) ang.x = -PI*0.5f;
	if(ang.x > PI*0.5f) ang.x = PI*0.5f;
	//Матрица направления
	Matrix mtx(ang);	
	//Перемещение	
	float speed = 5.0f*dltTime;
	ctrl = pref; ctrl += "FreeCamera_SpeedUp4x";
	if(ctr->GetControlStateType(ctrl) == CST_ACTIVE) speed *= 4.0f;
	ctrl = pref; ctrl += "FreeCamera_SpeedUp10x";
	if(ctr->GetControlStateType(ctrl) == CST_ACTIVE) speed *= 10.0f;
	ctrl = pref; ctrl += "FreeCamera_Forward";
	if(ctr->GetControlStateType(ctrl) == CST_ACTIVE)
	{
		pos += speed*mtx.vz;
	}
	ctrl = pref; ctrl += "FreeCamera_Back";
	if(ctr->GetControlStateType(ctrl) == CST_ACTIVE)
	{
		pos -= speed*mtx.vz;
	}
	ctrl = pref; ctrl += "FreeCamera_Left";
	if(ctr->GetControlStateType(ctrl) == CST_ACTIVE)
	{
		pos -= speed*mtx.vx;
	}
	ctrl = pref; ctrl += "FreeCamera_Right";
	if(ctr->GetControlStateType(ctrl) == CST_ACTIVE)
	{
		pos += speed*mtx.vx;
	}
	mtx.pos = pos;
	mtx.BuildViewFromObject();
	render->SetView(swing*mtx);
	render->SetPerspective(fov);
}

//Установить углы по точке
bool FreeCameraModule::SetTarget(float x, float y, float z)
{
	if(~(pos - Vector(x, y, z)) < 1e-10f) return false;
	Vector xz = Vector(x - pos.x, 0.0f, z - pos.z);
	ang.y = xz.GetAY();
	Vector yxz = Vector(pos.y - y, 0.0f, xz.GetLengthXZ());
	ang.x = yxz.GetAY();
	return true;
}

//============================================================================================
//FreeCamera
//============================================================================================

FreeCamera::FreeCamera()
{
}

FreeCamera::~FreeCamera()
{
	camera.ctr->EnableControlGroup("FreeCamera", false);
	RELEASE(camera.ctr);
}

//Инициализация entity
bool FreeCamera::Init()
{	
	if(!camera.Init())
	{
		return false;
	}
	camera.ctr->EnableControlGroup("FreeCamera", true);
	api->SetObjectExecution(this, "free camera", Core_DefaultExecuteLevel, &FreeCamera::Execute);
	api->SetGroupLevel("free camera", Core_DefaultExecuteLevel + 0x100);
	return true;
}

//Исполнение из слоя
void __fastcall FreeCamera::Execute(float dltTime)
{
	IMission * mis = (IMission *)api->FindObject("Mission");
	if(mis)
	{
		camera.swing = mis->GetSwingMatrix();
	}else{
		camera.swing.SetIdentity();
	}
	camera.Move(dltTime);
}

//Установить новую позицию камеры
void FreeCamera::SetPosition(const Vector & pos)
{
	camera.pos = pos;
}

//Получить позицию камеры
Vector FreeCamera::GetPosition()
{
	return camera.pos;
}

//Установить углы
void FreeCamera::SetAngles(const Vector & ang)
{
	camera.ang = ang;
}

//Получить углы
Vector FreeCamera::GetAngles()
{
	return camera.ang;
}

//Соориентировать камеру на точку
void FreeCamera::SetTarget(const Vector & pos)
{
	camera.SetTarget(pos.x, pos.y, pos.z);
}

//Установить угол обзора
void FreeCamera::SetFOV(float fov)
{
	camera.fov = fov;
}

//Получить угол обзора
float FreeCamera::GetFOV()
{
	return camera.fov;
}

//Приостановить работу камеры
void FreeCamera::Pause(bool isPause)
{
	if(!isPause)
	{
		api->SetObjectExecution(this, "free camera", Core_DefaultExecuteLevel, &FreeCamera::Execute);
	}else{
		api->DelObjectExecutions(this, "free camera");
	}
}

//============================================================================================
//MissionFreeCamera
//============================================================================================

MissionFreeCamera::MissionFreeCamera()
{
	model = null;
}

MissionFreeCamera::~MissionFreeCamera()
{
	if(model) model->Release(); model = null;
//	camera.ctr->EnableControlGroup("mission.FreeCamera", false);
	Controls().EnableControlGroup("mission.FreeCamera", false);
}

//Инициализировать объект
bool MissionFreeCamera::Create(MOPReader & reader)
{
	if(!camera.Init()) return false;
	//api->Controls->EnableControlGroup("mission.FreeCamera", true);
	camera.ctr->EnableControlGroup("mission");
	camera.ctr->EnableControlGroup("mission.FreeCamera");
	camera.pref = "Mission";
	
	SetUpdate((MOF_UPDATE)&MissionFreeCamera::Work, ML_CAMERAMOVE_FREE);
	return EditMode_Update(reader);
}

//Получить матрицу объекта
Matrix & MissionFreeCamera::GetMatrix(Matrix & mtx)
{
	return mtx.Build(camera.ang, camera.pos);
}

//Инициализировать объект
bool MissionFreeCamera::EditMode_Create(MOPReader & reader)
{
	//Создаём модельку камеры
	IGMXService * geom = (IGMXService *)api->GetService ("GMXService");
	if(!geom) return false;
	model = geom->CreateGMX("editor\\CameraModel.gmx", &Mission().Animation(), &Mission().Particles(), &Mission().Sound());
	SetUpdate((MOF_UPDATE)&MissionFreeCamera::EditModeWork, ML_GEOMETRY1);
	return EditMode_Update(reader);
}

//Обновить параметры
bool MissionFreeCamera::EditMode_Update(MOPReader & reader)
{
	camera.pos = reader.Position();
	camera.ang = reader.Angles();
	camera.fov = reader.Float()*(PI/180.0f);
	Activate(reader.Bool());
	return true;
}

//Получить размеры описывающего ящика
void MissionFreeCamera::EditMode_GetSelectBox(Vector & min, Vector & max)
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

//Работа
void _cdecl MissionFreeCamera::Work(float dltTime, long level)
{
	if(!IsActive()) return;
	dltTime = api->GetNoScaleDeltaTime();
	camera.swing = Mission().GetSwingMatrix();
	camera.Move(dltTime);
}

//Работа в режиме редактирования
void _cdecl MissionFreeCamera::EditModeWork(float dltTime, long level)
{
	if (!Mission().EditMode_IsAdditionalDraw()) return;
	if(model) 
	{
		model->SetTransform(Matrix(camera.ang, camera.pos));
		model->Draw();
	}
}

MOP_BEGINLISTG(MissionFreeCamera, "Free camera", '1.00', 0, "Cameras")
MOP_POSITIONC("Start position", Vector(0.0f, 5.0f, -5.0f), "Position in start mission moment")
MOP_ANGLESEXC("Start angles", Vector(0.6f, 0.0f, 0.0f), Vector(-0.49f*PI, -6*PI, 0.0f), Vector(0.49f*PI, 6*PI, 0.0f), "Orientation in start mission moment")
MOP_FLOATEXC("Camera FOV", 1.25f*180.0f/PI, 10.0f, 140.0f, "Camera frustum angle in degrees")
MOP_BOOLC("Active", true, "Active camera in start mission time")
MOP_ENDLIST(MissionFreeCamera)

