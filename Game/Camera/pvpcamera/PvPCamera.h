
#ifndef _MissionPvPCamera_h_
#define _MissionPvPCamera_h_

#include "..\..\..\Common_h\Mission\Mission.h"
#include "..\camera_inc.h"

class MissionPvPCamera : public BaseCamera
{

	struct DebugInfo
	{
		bool bApproach, bAway;
		void Reset();
	} dbginf;

	struct Target
	{
		MOSafePointer target;				//Наблюдение за целью
		Vector pos;							//Текущая позиция
		ConstString targetID;				//Идентификатор цели
		Vector targetPos;					//Локальная позиция
		bool includeIntoCamera;				//Включать объект в камеру
	};

	struct FloatFactor
	{
		float fMinValue;
		float fMaxValue;
		float fCurValue;
		float fSpeed;
		bool bDecrease;

		void Init(float min,float max) {fMinValue=min; fMaxValue=max; fCurValue=0.f; fSpeed=0.1f; bDecrease=true;}
		float ChangeValue(float fNewVal) {fCurValue = fCurValue + (fNewVal-fCurValue)*fSpeed; bDecrease=false; return fCurValue;}
		void Decrease() {if(bDecrease) fCurValue *= (1.f-fSpeed); bDecrease=true;}
	};

public:
	enum PvPCConstants
	{
		max_players = 4
	};
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
	//Найти косинус угола охватывающий цели
	float CalcCosFoV(const Vector & from, const Vector & to);

	//Расчитать косинусы целей
	void CalcTargsCos();
	void WorkMoveAway();
	void WorkApproach();
	//Пододвинуть камеру вперед (отрицательное значение - движение назад)
	bool MoveCamForward(float dist);
	//Пододвинуть камеру вправо (отрицательное значение - движение влево)
	bool MoveCamRight(float dist);
	//Обновить цели
	long UpdateTargets();
	//Обновить нормаль зоны камеры
	void UpdateZoneNormal();
	//проверить камеру на необходимость отката
	bool VerifyCamera();
	bool VerifyCamera09();
	void DebugShowVerifyData();
	// получить смещения камеры исходя из позиции
	void GetOffsetsFromPosition(const Vector & pos);

private:
	static Vector CalculatePos(const Vector& lt, const Vector& rt, const Vector &lb, const Vector &rb, float u, float v);

	//--------------------------------------------------------------------------------------------
private:
	Vector line[2];						//Отрезок на котором может быть камера
	long targetsCount;					//Количество целей
	Target targets[max_players];		//Цели, которые могут быть у камеры + 1 цель в базовой камере

	float m_fCamTargetSpeed;			//скорость движения прицела камеры
	float m_fCamTargetSpeedMax;			//максимальная скорость движения прицела камеры
	Vector targetcenter;				//Центр целей - туда тянется смотреть камера
	Vector realcenter;					//Точка, куда реально смотрит камера

	// зона камеры
	Vector m_nearLine[2];				//Ближняя линия камеры
	Vector m_farLine[2];				//Дальняя линия камеры
	float m_fNearestDistance;			//Минимальное разрешенное расстояние до ближайшей цели
	float m_fMaxAngleCos;				//Косинус максимального угола поворота камеры

	// задержка на приближение камеры (для того что бы избежать дрожания)
	float m_fApproachPause;				// текущее время задержки
	float m_fApproachPauseTime;			// максимальное время в течении которого нет приближения камеры

	// положение целей:
	float m_fNearDist;					// расстояние до ближайшей цели
	float m_fNearPlane;					// наименьшее расстояние от цели до плоскости сечения зоны камерой
	float m_fLeftCos;					// косинус от направления камеры до крайней левой цели
	float m_fRightCos;					// косинус от направления камеры до крайней правой цели
	float m_fCam2ZoneCos;				// косинус от направления камеры до нормали зоны камеры
	float m_fCam2ZoneLimit;				// косинус от направления камеры до нормали зоны камеры (лимит в котором попорот осуществляется смещение по горизонтали)

	// положение камеры
	Vector m_vRealCamFrom;				//Положение камеры куда надо ей стремиться
	Vector m_vSpeedCamFrom;				//Текущее скорость смены положения камеры
	float m_fMaxCamSpeed;				//Скорость смены положения камеры
	//
	Vector m_vCurCamFrom;				//Текущее положение камеры
	Vector m_vZoneNorm;					//Текущая нормаль зоны
	Vector m_vGreedDir;					//Направление камеры в зональной сетке
	float m_fApproachFactor;			//Коеффициент приближения камеры (0- мы на дальней линии, 1- мы на ближней линии)
	float m_fOffsetFactor;				//Коеффициент расположения камеры влево/вправо (0- крайняя левая граница зоны, 1- крайняя правая граница зоны)

	float m_fCamCos;					//Косинус камеры (ширина охвата)
	FloatFactor m_offsetStep;			//шаг приближения/удаления камеры в зоне
	FloatFactor m_distanceStep;			//шаг смещения камеры в зоне

	float m_fCurDeltaTime;

	Vector m_vStartPos;
};

#endif