#pragma once
#include "..\..\common_h\Mission.h"

//////////////////////////////////////////////////////////////////////////
// Менеджер фаталити для морских миссий
//////////////////////////////////////////////////////////////////////////
class BaseCamera;
class MissionTime;
class SeaFatalityManager : public MissionObject
{
	enum Mode { Normal, SlomoIn, Slomo, SlomoOut };
	struct Params
	{
		Params() : cams(__FILE__, __LINE__) {}
		struct CamParams
		{
			float		weight;			// вес выбора данной камеры
			ConstString	camera;			// имя камеры
			float		workTime;		// время работы камеры
			float		slomoInTime;	// время перехода в slowmo
			float		slomoTime;		// время нахождения в slowmo
			float		slomoOutTime;	// время выхода из slowmo
			float		slomoFactor;	// коэфициент slowmo
			float		clearRadius;	// радиус в котором не должно быть других кораблей
		};

		float	nearCameraModeRadius;	// радиус режима работы ближняя-дальняя камера
		ConstString	missionTime;		// имя объекта Mission time
		
		float	farProbability;			// вероятность срабатывания дальних камер 0-100
		array<CamParams>	cams;		// массив камер для показа дальних кораблей, которые выбираются случайно
	};

	struct FatalityTarget
	{
		FatalityTarget() : ship(NULL), isProcessed(false) {}
		FatalityTarget(MissionObject* shp) : ship(shp), isProcessed(false) {}
		MissionObject*	ship;
		bool			isProcessed;
	};

	Params					m_params;
	MissionObject*			m_playerCam;
	MissionObject*			m_missionTime;
	array<FatalityTarget>	m_ships;
	bool					m_isFatality;
	float					m_fatalityTime;
	float					m_cameraTime;
	unsigned int			m_curCam;
	Mode					m_mode;
public:
	SeaFatalityManager(void);
	virtual ~SeaFatalityManager(void);

	void _cdecl Work(float dTime, long level);

	void FindFatalityTarget();
	
	void DoFatality( float dTime );
	
	void _cdecl InitHandler( float dTime, long level );

	//////////////////////////////////////////////////////////////////////////
	// MissionObject
	//////////////////////////////////////////////////////////////////////////

	virtual bool Create(MOPReader& reader);

	virtual void Activate(bool isActive);
	
};
