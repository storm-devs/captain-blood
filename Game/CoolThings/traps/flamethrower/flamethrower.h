#ifndef TRAP_FLAMETHROWER_H
#define TRAP_FLAMETHROWER_H

#include "..\trapbase.h"
#include "flamethrowerpattern.h"


class TrapFlamethrower : public TrapBase
{
	enum State
	{
		state_inactive,
		state_warning,
		state_active
	};

public:
	TrapFlamethrower();
	~TrapFlamethrower();

	virtual void Show(bool isShow);
	virtual void Activate(bool isActive);

	MO_IS_FUNCTION(TrapFlamethrower, TrapBase);

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	// Инициализировать в режиме редактора
	virtual bool EditMode_Create(MOPReader & reader);
	//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
	virtual void PostCreate();
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);
	//команды
	virtual void Command(const char * id, dword numParams, const char ** params);

	// реализация дамага от конкретной ловушки
	virtual void InflictDamage(DamageReceiver* pObj, TrapPatternBase::CharReactionData& reactions);
	// обработка на кадре
	void Frame(float fDeltaTime);
	void EditorDraw();
	void UpdatePattern();

private:
	void SwitchState(State st);
	void ReadMOPs(MOPReader & reader);
	void SetMove(bool bMove);
	Matrix CalculateMatrix();
	void LaunchParticle(long n, const char* sfx, float scale, float timescale, bool bLocal, const Vector& pos);
	bool GetPointOnCharacterPath(Vector& pos);
	void UpdateDamageZone();

protected:
	ConstString m_pcPattern;
	Matrix m_mtxBaseTransform;

	FlametrapPattern::FlameSector m_flame;
	FlametrapPattern::ComplexityData m_complexity;
	FlametrapPattern::ParticlesDescr m_particles;
	FlametrapPattern::SoundsDescr m_sounds;

	float m_fStateTimer;
	State m_curState;

	bool m_bMove;
	float m_fCurPosition;
	float m_fCurSpeed;
	bool m_bMoveForward;
	float m_fMakeTrackTime;

	ConstString m_pcTieObjectName;
	MOSafePointer m_TieObject;
};

#endif
