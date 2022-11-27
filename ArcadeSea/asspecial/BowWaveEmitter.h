//////////////////////////////////////////////////////////////////////////
// Modenov Ivan 2006
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common_h/Mission.h"
#include "../../Common_h/gmx.h"
#include "bowparticles.h"
#include "bowgeometry.h"

// эмиттер партиклов буруна у носа корабля
class BowWaveEmitter :
	public MissionObject
{
public:
	// параметры буруна
	struct Params
	{
		ConstString		hostName;				//	имя хостового объекта
		const char*		texName;				//	текстура партиклов
		Vector			pos;					//	относительная позиция эмиттера
		unsigned int	maxPartCount;			//	максимальное число частиц
		float			curveHeight[2];			//	высота ограничительных парабол
		float			curveWidth[2];			//	ширина ограничительных парабол

		float			endLine;				//	расстояние до конца парабол
		float			parabolaTrackCoef;		//	соотношение параболы и лежания на воде

		float			maxHeight;				//	максимальная высота вылета частиц
		float			minHeight;				//	минимальная высота вылета частиц
		
		float			maxParticleSize;		//	макс. размер частицы
		float			minParticleSize;		//	мин. размер частицы

		float			maxParticleLifeTime;	//	максимальное время жизни частицы
		float			minParticleLifeTime;	//	минимальное время жизни частицы

		float			maxRotateSpeed;			//	макс. скорость вращения
		float			minRotateSpeed;			//	мин. скорость вращения

		bool			isEmissionStopped_;		//	флаг остановки эмиттера
	};

private:
	//////////////////////////////////////////////////////////////////////////
		

	Params			params_;					//	
	MOSafePointer	host_;						//	
	Vector			prevHostPos_;				//	
	Vector			prevHostDir_;				//	

	Vector			shipVelocity_;				//	линейная скорость корабля
	float			shipAngularVel_;			//	угловая скорость корабля

	BowParticles	particles_;
	BowGeometry		geometry_;
	
	void ReadMOPs(MOPReader & reader);
	void DrawEditorFeatures();
	
public:
	BowWaveEmitter(void);
	virtual ~BowWaveEmitter(void);

	MissionObject* GetHost() { return host_.Ptr(); }
	const Vector& GetHostLinearSpeed() const { return shipVelocity_; }
	float GetHostAngularSpeed() const { return shipAngularVel_; }
	Params& GetParams() { return params_; }

	void _cdecl PartWork(float deltaTime, long level);
	void _cdecl GeoWork(float deltaTime, long level);


	// Создание объекта
	virtual bool Create(MOPReader & reader);
	// Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);
	// Инициализация 
	virtual bool Init();

	MO_IS_FUNCTION(BowWaveEmitter, MissionObject);

};
