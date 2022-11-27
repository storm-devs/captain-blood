#ifndef TRAP_THORN_H
#define TRAP_THORN_H

#include "..\trapbase.h"
#include "thornpattern.h"

class TrapThorn : public TrapBase
{
	enum State
	{
		state_inactive,
		state_warning,
		state_active
	};

public:
	TrapThorn();
	~TrapThorn();

	virtual void Show(bool isShow);
	virtual void Activate(bool isActive);

	MO_IS_FUNCTION(TrapThorn, TrapBase);

	// Инициализировать объект
	virtual bool Create(MOPReader & reader);
	// Инициализировать в режиме редактора
	virtual bool EditMode_Create(MOPReader & reader);
	//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
	virtual void PostCreate();
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);

	// реализация дамага от конкретной ловушки
	virtual void InflictDamage(DamageReceiver* pObj, TrapPatternBase::CharReactionData& reactions);
	// обработка на кадре
	void Frame(float fDeltaTime);
	void EditorDraw();
	void UpdatePattern();

private:
	void SwitchState(State st);
	void ReadMOPs(MOPReader & reader);
	Matrix GetCurrentMatrix();
	void StartMoving(float fStartHeight, float fEndHeight, float speed, float accelerate);
	void StopMove() {m_fCurHeight = m_fMaxHeight; m_bMoving = false;}

protected:
	// Паттерн
	ConstString m_pcPatternName;
	MOSafePointer m_PatternObj;

	// позиция
	Matrix m_mtxBaseTransform;

	// Логика
	float m_fActiveTime;						// время которое ловушка находится в активном состоянии	
	ThornPattern::ComlexityData m_complexData;	// параметры зависящие от сложности
	// текущий режим
	State m_curState;
	float m_fStateTimer;

	// высоты
	ThornPattern::Heights m_heights;

	// звуки
	ThornPattern::Sounds m_sounds;

	// движение шипов
	bool m_bMoving;
	float m_fHeightSpeed;
	float m_fHeightAccelerate;
	float m_fCurHeight;
	float m_fMaxHeight;
};

#endif
