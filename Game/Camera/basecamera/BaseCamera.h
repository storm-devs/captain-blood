//============================================================================================
// Spirenkov Maxim
//============================================================================================
// BaseCamera
//============================================================================================

#ifndef _BaseCamera_h_
#define _BaseCamera_h_

#include "BaseCamera.h"
#include "..\..\..\common_h\mission\Mission.h"
#include "CamerasBlender.h"


#define MOP_CAM_TARGET		MOP_STRINGC("Target", "Player", "Mission object ID for use it as target")	\
							MOP_POSITIONC("Target point", Vector(0.0f, 1.5f, 0.0f), "Relative local target point in object system")
#define MOP_CAM_FOV			MOP_FLOATEXC("Camera FOV", 1.25f*180.0f/PI, 10.0f, 140.0f, "Camera frustum angle in degrees")
#define MOP_CAM_BLENDER		MOP_FLOATEXC("Blend time", 0.0f, 0.0f, 10.0f, "Blend time from last camera position to current when activate it")
#define MOP_CAM_LEVEL		MOP_LONGEXC("Level", 0, 0, 10, "Set current priority level for camera")
#define MOP_CAM_ACTIVATE	MOP_BOOLC("Active", true, "Active camera in start mission time")
#define MOP_CAM_PREVIEW		MOP_BOOLC("Edit preview", false, "Preview camera from editor viewport")
#define MOP_CAM_DRAWCAMERA	MOP_BOOLC("Draw Camera", true, "Draw Camera in Edit Mode")

#define MOP_CAM_STD			MOP_CAM_TARGET		\
							MOP_CAM_FOV			\
							MOP_CAM_BLENDER		\
							MOP_CAM_LEVEL		\
							MOP_CAM_ACTIVATE	\
							MOP_CAM_PREVIEW     \
							MOP_CAM_DRAWCAMERA


#define CAMERA_COMMENT(a) a##"  Aviable commands list:\n    Target targetID - set new target\n    Reset - reset camera\n  Blendtime Blendtime - set camera blending time\n   ShockRND amplitude time itencity - shock camera \n   ShockRND1\n   ShockRND2\n   ShockRND3\n   ShockRND4\n   ShockRND5\n   ShockRND_H amplitude time itencity - shock camera \n    ShockRND_V amplitude time itencity - shock camera \n    ShockPolar amplitude time itencity - shock camera \n   ShockPolar1\n   ShockPolar2\n   ShockPolar3\n   ShockPolar4\n   ShockPolar5\n  resetShock - reset camera shock\n  Attach mo_name - attach camera to object\n  Teleport mo_name - teleport camera to object\n  setFov fov time - set new fov\n  AutoFocusParams Dist MinAngle MaxAngle MinSpeedAngle MaxSpeedAngle - AutoFocus Parameters\n    AutoFocusOn - Turn On AutoFocus \n    AutoFocusOff - Turn Off AutoFocus \n    CrazyOn - Turn On Crazy \n    CrazyOff - Turn Off Crazy \n    CrazyParams Amp Speed - Crazy Parameters\n  TimeScale fTimeScale\n   AccelTimeScale fTimeScaleAccel\n   BloodPuff\n EnableMotionBlur\n  DisableMotionBlur\n  MotionBlurChgSpeed: fMB_delta\n  MotionBlurParams fImageK, fAfterImageK\n  ChangeDist - change RotatedCam distance\n  ChangeMinVAngle - change RotatedCam MinVertAngle\n  ChangeMaxVAngle - change RotatedCam MaxVertAngle\n  LockTarget - change RotatedCam LockTarget\n  "

#define CAMERAS_GROUP MG_CAMERA

class CameraController;

class BaseCamera : public MissionObject
{	
//--------------------------------------------------------------------------------------------
public:

	bool debugdraw_enbled;

	BaseCamera();
	virtual ~BaseCamera();

//--------------------------------------------------------------------------------------------
//Generic
//--------------------------------------------------------------------------------------------
protected:
	//--------------------------------------------------------------------------------------------
	//Параметры камеры
	//--------------------------------------------------------------------------------------------
	//Получить позицию камеры
	virtual void GetCameraPosition(Vector & position) = null;
	//Получить точку наблюдения камеры
	virtual bool GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up);	

	//--------------------------------------------------------------------------------------------
	//Работа с камерой
	//--------------------------------------------------------------------------------------------
	//Прочитать параметры идущие перед стандартным
	virtual bool CameraPreCreate(MOPReader & reader){ return true; };
	//Прочитать параметры идущие после стандартных
	virtual bool CameraPostCreate(MOPReader & reader){ return true; };
	//Сбросить состояние камеры в исходное
	virtual void Reset() { blender.ResetShock(); fovChgCurTime = -1.0f; fov = init_fov; fCurFOV = init_fov; fovLast = fov; };
	//Обновить состояние камеры
	virtual void WorkUpdate(float dltTime){};
	//Обновить состояние для предпросмотра
	virtual void SelectedUpdate(float dltTime);
	//Нарисовать дополнительную информацию нри селекте
	virtual void SelectedDraw(float dltTime);

//--------------------------------------------------------------------------------------------
protected:
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);
	
public:

	virtual void InitParams();

	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);
	virtual long GetCameraLevel() { return level; };

	//Получить FoV камеры
	virtual float GetCameraFoV();

	//Пересоздать объект
	virtual void Restart();

	virtual bool CheckCommand(const char * id);

	//Отрисовка в выделеном режиме
	virtual void _cdecl DebugDraw(float dltTime, long level);

	void GetTargetPos(Vector& pos) { pos = targetPos; };
	void GetBlenderMatrix(Matrix& mat, float fov) { blender.GetMatrix(mat,fov,true); };
	virtual bool AllowZooming() { return true; };	

protected:
	//Инициализировать объект
	virtual bool EditMode_Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//Получить размеры описывающего ящика
	virtual void EditMode_GetSelectBox(Vector & min, Vector & max);
#ifndef MIS_STOP_EDIT_FUNCS
	//Выделить объект
	virtual void EditMode_Select(bool isSelect);
#endif
	//Активировать/деактивировать объект
	virtual void Activate(bool isActive);

	//Автоматический расчет FOV
	virtual void AutoFocusCam(float dltTime);

	//Работа
	virtual void _cdecl Work(float dltTime, long level);
	//Работа в режиме редактирования
	virtual void _cdecl EditModeWork(float dltTime, long level);

	//virtual void _cdecl CameraDebugDraw(float dltTime, long level);
	//Отрисовка в выделеном режиме
	virtual void _cdecl EditModeDrawSelected(float dltTime, long level);	
	
protected:
	//Получить указатель на контролер камеры
	CameraController * GetCameraController();

protected:
	
/*
	float debugCamFov;
	float debugInputFov;
	float debugFovToPrint;
*/

	MOSafePointer target;				//Наблюдение за целью
	ConstString targetID;				//Идентификатор цели
	Vector targetPos;					//Локальная позиция
	float fov;							//Угол зрения
	long level;							//Уровень исполнения камеры
	MissionCamerasBlender blender;		//Блендер позиций
	IGMXScene * model;					//Моделька для редактирования
	bool isPreview;						//Вид из камеры

	bool bDrawCamera;

	MOSafePointer attachedobj;	
	string attachedobjID;
	dword attachedobjIDhash;

	Matrix cam_offset;
	Matrix offset_matrix;

	int   attach_mode;

	bool  AutoZoom;
	float fCurFOV;

	float fNeededDist;
	float fMinAngle;
	float fMaxAngle;

	float fMinSpeedAngle;
	float fMaxSpeedAngle;
	
	float fovChgTime;
	float fovChgCurTime;
	float fovChgTarget;
	float fovLast;

	float init_fov;
};


#endif
