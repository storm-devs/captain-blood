//////////////////////////////////////////////////////////////////////////
// Modenov Ivan 2006
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "..\common_h\math3d.h"
#include "..\common_h\defines.h"

class IRender;

class Ballistics
{
	static const float gravity_;	// гравитация

	bool isPhysicsFly_;				// физический полет ядра
	Vector v0_, v1_;				// начальная и конечная точки траектории
	float curTime_;					// время прошедшее с начала полета
	Vector curPos_;					// текущее положение
	bool bValid_;					// флаг правильности внутреннего состояния

	float startSpeed_;				// начальная скорость
	float angle_;					// начальный угол
	float g_;						// ускорение падения

	float height_;					// высота полета ядра
	float fTotalTime_;				// предрасчетное время полета

public:

	// конструктор/деструктор
	Ballistics();
	~Ballistics();

	// задает новую траекторию
	bool SetTrajectoryParams(const Vector& v0, const Vector& v1, float startSpeed, float minAngle = 0.0f, float maxAngle = PI/2);
	// задает новую траекторию по минимальной высоте и времени полета
	bool SetTrajectoryParamsByTime(const Vector& v0, const Vector& v1, float fMinHeight, float fTime);
	// задать общие парметры
	void SetCommonTrajectoryParams(const Vector& v0, const Vector& v1)
	{
		curTime_ = 0.0f;
		v0_ = v0;
		v1_ = v1;
		curPos_ = v0_;
	}

	// расчитывает новое положение снаряда
	void Step(float deltaTime);

	// возвращает текущее положение снаряда
	const Vector& GetPosition() const { return curPos_; }
	const Vector& GetEndPosition() const { return v1_; }

	// возвращает текущее время от начала полета
	float GetTime() const { return curTime_; }

	// возвращает полное время полета
	float GetTotalFlightTime() const;

	// считает положение снаряда в любой момент времени
	Vector GetPosition(float time) const;

	// возвращает угол под которым был выпущен снаряд
	float GetAngle() const { return angle_; }

	float GetPrecalculateTotalTime() const { return fTotalTime_; }



	//////////////////////////////////////////////////////////////////////////
	// статические расчетные функции

	// расчитывает угол, под которым надо
	// запустить снаряд, чтобы он попал в заданную точку
	static bool ComputeShootAngle(const Vector& v0, const Vector& v1, float startSpeed, float & angle, float minAngle = 0.0f, float maxAngle = PI/2);

	// расчитывает время полета снаряда между точками
	static bool ComputeFlightTime(const Vector& v0, const Vector& v1, float startSpeed, float & time);

	// расчитывает время полета снаряда между точками
	static float ComputeFlightTime(float distanceXZ, float angle, float startSpeed);
	
	// расчитывает положение снаряда в любой точке траектории
	static bool ComputePosition(const Vector& start, const Vector& end, float g, float startSpeed, float angle, float time, Vector& pos);

	void DebugDraw(IRender & render, dword color);
};
