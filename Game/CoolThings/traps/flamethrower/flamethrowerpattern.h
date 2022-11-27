#ifndef FLAMETHROWERPATTERN_H
#define FLAMETHROWERPATTERN_H

#include "..\trapbase.h"


class FlametrapPattern : public TrapPatternBase
{
public:
	struct FlameSector
	{
		Vector moveAngle;	// ширина сектора качания огнемета
		float damageAngle;	// ширина сектора где огнемет наносит повреждение
		float distance;		// дальность действия огнемета
		float startAng;		// позиция начала действия огнемета
		float rotateSpeed;  // скорость качания огнемета (знак указывает начальное направление движения)
	};
	struct ComplexityData
	{
		float prepareTime;
		float activeTime;
		float damage;
		float immuneTime;
	};
	struct ParticlesDescr
	{
		struct sfxdescr
		{
			const char* sfx;
			float scale;
			float timescale;
		};
		sfxdescr prepare;
		sfxdescr active;
		sfxdescr track;
		float trackStartTime;
		float trackUpdateTime;
		sfxdescr trackPrepare;
	};
	struct SoundsDescr
	{
		const char* pcPrepare;		// звук приготовления (разгорания)
		const char* pcActive;		// звук во время активности огнемета (горение)
		const char* pcDeactivate;	// звук на деактивацию (тушение) огнемета
	};

public:
	FlametrapPattern();
	~FlametrapPattern();

	MO_IS_FUNCTION(FlametrapPattern, MissionObject);

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);

	// получение параметров
	IGMXScene* GetModel() {return m_pModel;}
	const FlameSector& GetFlameParams() {return m_flame;}
	bool GetComplexityData(ComplexityData& dat);
	const ParticlesDescr& GetParticlesData() {return m_particles;}
	const SoundsDescr& GetSoundsData() {return m_sounds;}

private:
	// описание уровня сложности
	struct Complexity
	{
		const char* name;
		ComplexityData dat;
	};

	// модель ловушки
	IGMXScene* m_pModel;

	FlameSector m_flame;
	array<Complexity> m_aComplexity;
	ParticlesDescr m_particles;
	SoundsDescr m_sounds;

private:
	void ReadMOPs(MOPReader & reader);
};

#endif
