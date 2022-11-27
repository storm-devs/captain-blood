
#ifndef _MissionFixedCamera_h_
#define _MissionFixedCamera_h_

#include "..\..\..\Common_h\Mission\Mission.h"
#include "..\camera_inc.h"

class MissionRotatedCamera : public BaseCamera
{
//--------------------------------------------------------------------------------------------
public:

	Vector  ang;
	float   dist;
	
	float   fRotSpeedH;
	float   fMaxRotSpeedH;
	float   fRotAccelerateH;

	float   fRotSpeedV;
	float   fMaxRotSpeedV;
	float   fRotAccelerateV;

	float   fMinRotAngleV;
	float   fMaxRotAngleV;

	float   trg_dist;
	float   trg_fMinRotAngleV;
	float   trg_fMaxRotAngleV;
	
	MOSafePointer aim_target;	//Наблюдение за целью
	ConstString aim_targetID;		//Идентификатор цели
	float   new_ay;
	float   autoturn_spd;
	float   autoturn_time;

	int     locktrg_index;

	struct TLockTarget
	{
		ConstString name;
		float  dist;
		float  ay;

		MOSafePointer mo;

		struct TLockTarget()
		{
			dist = 0;
		}
	};

	array<TLockTarget> LockTargets;

	MissionRotatedCamera():LockTargets(_FL_)
	{		
	};

	bool bOneObjectattach;
	virtual void InitParams();

	virtual bool CheckCommand(const char * id);
//--------------------------------------------------------------------------------------------
private:
	//Прочитать параметры идущие после стандартных
	virtual bool CameraPreCreate(MOPReader & reader);
	//Получить позицию камеры
	virtual void GetCameraPosition(Vector & position);

	virtual void WorkUpdate(float dltTime);

	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	void ChangeValue(float &value, float targetValue, float dltTime, float accel);

	void ValidateMO(MOSafePointer & mo,const ConstString & name);

	void SortLockTargetList();

	void FindFarLeft();
	void FindFarRight();

	bool CheckMOCondition(MOSafePointer & mo);

	bool CheckLockTargetList();

	void NextLockIndex(bool forward);
	void FindNextLockTarget(bool forward);

	void CalcAutoTurnSpeed();

	inline void SetLockTarget(const ConstString & target);
	inline void UnlockTarget();
	inline void ShowAim(bool show);
//--------------------------------------------------------------------------------------------
private:
	Vector dir;							//Направление смещения камеры
};

#endif