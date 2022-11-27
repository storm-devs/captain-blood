#ifndef CAMERA_CONTROLLER
#define CAMERA_CONTROLLER

#include "..\basecamera\BaseCamera.h"
#include "..\..\..\common_h\ICameraController.h"

#include "../splashmanager/splashmanager.h"
#include "../postprocess/PostProcess.h"

class CameraController : public ICameraController
{			
	Splash* splash;

	BaseCamera* ActiveCamera;	

	bool IsAnotherCamController;	
	bool HasentActiveCam;	
	MGIterator* cam_iter;	

public:
	
	//Конструктор - деструктор
	CameraController();
	virtual ~CameraController();
	
	BaseCamera* GetActiveCamera() { return ActiveCamera;};

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);

	MO_IS_FUNCTION(CameraController, MissionObject);

	void _cdecl Realize(float fDeltaTime, long level);

	void Command(const char * id, dword numParams, const char ** params);

	void SetActiveCamera(BaseCamera* NewActiveCamera);

	long GetActiveCameraLevel();

	void FindHighestActiveCam();

	void AddBloodPuff();

	bool CheckCommand(const char * id);

	bool IsHasentActiveCam() { return HasentActiveCam; };

	void _cdecl Console_ShowActiveCameras(const ConsoleStack & params);

	virtual void GetTargetPoint(Vector& pt);
	virtual void GetBlenderMatrix(Matrix& shock, float fov);

	virtual bool AllowZooming();	

	static const char * comment;
};

#endif