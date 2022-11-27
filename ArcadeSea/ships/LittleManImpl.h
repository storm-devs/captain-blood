#pragma once
#include "ShipPattern.h"
#include "..\Ballistics.h"

//
class LittleManPattern;
class LittleManImpl
{
	enum DieType { FromBomb, FromBall };
	IMission&			mission_;			//
	IGMXScene*			model_;				// моделька перса
	IParticleSystem*	deathParts_;		// партиклы смерти

	//LittleManPattern*	pattern_;
	//long				patternHash_;
	MOSafePointer		pattern_;
	
	Vector				localPos_;			// локальная позиции на корабле
	Vector				angles_;			// ориентация
	Vector				m_rotateSpeed;		// скорость вращения при полете
	Vector				worldPos_;			// мировая позиция перса
	float				dieFlyAngle_;		// дистанция вылета при смерти
	float				dieFlyDist_;		// угол вылета при смерти
	float				dieFlySpeed_;		// скорость вылета при смерти
	float				ballReactRadius_;	// дистанция реагирования на попадание снаряда

	unsigned int		animIndex_;
	unsigned int		behaviourIndex_;
	unsigned int		nodeIndex_;
	float				animTime_;
	bool				executeAnimation_;

	Ballistics			dieTrack_;			// траектория полета смерти
	bool				alive_;				// живой/неживой
	bool				show_;				// видимый изначально

	array<const char*>	groups_;			// группы, в которых состоит человечек
	bool				hide_;				// глобальный флажок сокрытия
	float				fade_;				// текущее значение фэйда
	float				hideShowFadeTime_;
	float				alpha_;

	void ChangeBehaviour(const char* behaviour);
public:
	void Work(const Matrix& parent, const Vector& vCamPos, float fDeltaTime);
	void AttackBall(const Vector& hitPoint);
	void AttackMine(const Vector& explCenter, float radius);
	void Kill();
	void SetBehaviour(const char* group, const char* behaviour);
	void Hide(const char* group, bool hide, bool withFade);
	const Vector& GetLocalPos() {return localPos_;}

	void SetUserAlpha(float alpha) { alpha_ = alpha; }
	LittleManImpl(MissionObject* mo, const ShipContent::LittleMan& params, IMission& mission);
	~LittleManImpl(void);
};

//////////////////////////////////////////////////////////////////////////

class LittleManPattern : public MissionObject
{
public:
	struct Params
	{
		Params() :	anims(__FILE__, __LINE__),
					dieParticles(__FILE__, __LINE__) {}

		struct Animation
		{
			Animation() : behaviours(__FILE__, __LINE__) {}
			struct Behaviour
			{
				struct Node
				{
					const char*	name;
					float	blendTime;
				};
				Behaviour() : nodes(__FILE__, __LINE__) {}

				const char*		name;
				array<Node>		nodes;
			};
			const char*			animFileName;
			array<Behaviour>	behaviours;
		};
		
		const char*			model;
		array<Animation>	anims;
		const char*			explosionBehaviour;
		array<const char*>	dieParticles;
		const char*			waterHitParticles;

		const char*			waterHitSound;
		const char*			throwOutSound;

		float				animOffDistance;
		float				animMaxTime;
		float				dieRotateSpeed;
	};
private:
	Params			params_;
	MOSafePointer	owningPattern_;
public:
	LittleManPattern() {}
	void SetOwningPattern(const MOSafePointer& owningPattern) {owningPattern_ = owningPattern;}
	virtual bool Create(MOPReader & reader);
	virtual bool EditMode_Update(MOPReader & reader);
	MO_IS_FUNCTION(LittleManPattern, MissionObject);

	const Params& GetParams() const { return params_; }
};