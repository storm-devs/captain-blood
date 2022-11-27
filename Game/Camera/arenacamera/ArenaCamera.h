
#ifndef _ArenaCamera_h_
#define _ArenaCamera_h_

#include "..\..\..\Common_h\Mission\Mission.h"
#include "..\camera_inc.h"


class ShipFightCamera : public BaseCamera
{
	struct Target
	{
		MOSafePointer target;
		ConstString targetID;				//Идентификатор цели
		Vector targetPnt;
		Matrix world;
		bool active;

	};

public:
	ShipFightCamera();
	~ShipFightCamera();
	virtual bool AllowZooming() { return false; }

	//--------------------------------------------------------------------------------------------
private:
	//Прочитать параметры идущие после стандартных
	virtual bool CameraPreCreate(MOPReader & reader);
	//Прочитать параметры идущие после стандартных
	virtual bool CameraPostCreate(MOPReader & reader);
	//Сбросить состояние камеры в исходное
	virtual void Reset();
	//Обновить состояние камеры
	virtual void WorkUpdate(float dltTime);
	//Нарисовать дополнительную информацию нри селекте
	virtual void SelectedDraw(float dltTime);
	//Получить позицию камеры
	virtual void GetCameraPosition(Vector & position);
	//Получить точку наблюдения камеры
	virtual bool GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up);
	//Инициализация параметров
	virtual void InitParams();

	//Обновить цели
	void UpdateTargets();

	virtual void _cdecl DebugDraw(float dltTime, long level);

	//--------------------------------------------------------------------------------------------
private:

	float fCamHeight;
	float fCamHeightNear;
	//float fDistToFitFrustum;
	bool bShowDebug;
	float fDistToSwitch;
	float fOffsetToPlayer;
	float fPanSpeed;
	float fZoomSpeed;

	float fZoom;

	float fTimeWithoutZoom;

	enum LRFlag
	{
		LR_LEFT = 0,
		LR_RIGHT = 1,

		LR_FORCE_DWORD = 0x7fffffff
	};


	void AddObjectToSphere(bool & bSphereInited, Sphere & sph, /*const*/ Target &trg);
	void CalcMinDistanceToPlane(bool & bSphereInited, Vector& minV, float & fMinDepth, float & fMin, const Plane & p, /*const */Target &trg, const Matrix & mtxView, LRFlag flg);


	//Выбирает второй по значимости в композиции корабль (1-й это корабль игрока)
	long SelectMainShip (long currentMainShip);


	Vector CalcCamPosFromQuat (const Quaternion & q, const Vector & lookTo, float fDist);


	float moveSpeed;


	Vector camPos;
	Vector camTarget;


	bool bRealNotInited;
	Vector camPosReal;
	Vector camTargetReal;

	float fPanOffset;
	float fPanOffsetReal;
	Vector _panVector;

	long activeShipIdx;
	
	array<Target> targets;				// Цели, которые могут быть у камеры + 1 цель в базовой камере


	__forceinline float GetDistanceToPlane (const Vector & v , const Plane & p)
	{
		return ((v | p.n) - p.d);
	}


};

#endif