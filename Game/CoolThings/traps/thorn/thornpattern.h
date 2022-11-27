#ifndef THORNPATTERN_H
#define THORNPATTERN_H

#include "..\trapbase.h"


class ThornPattern : public TrapPatternBase
{
public:
	// звуки
	struct Sounds
	{
		ConstString pcStartMove;	// звук начала движения (поднятие до предупреждения)
		ConstString pcWarning;		// звук предупреждения
		ConstString pcActivate;		// звук на активацию шипа (поднятие до активного сотояния)
		ConstString pcDeactivate;	// звук на деактивацию (падение) шипа
	};
	// высоты
	struct Heights
	{
		float fDisable;				// базовая высота шипов
		float fWarning;				// высота на которой шипы предупреждают о скорой активации
		float fActive;				// высота на которой шипы активны и наносят дамаг

		float fDisableAccelerate;
		float fWarningAccelerate;
		float fActiveAccelerate;
	};
	// данные зависимые от сложности
	struct ComlexityData
	{
		float warningTime;			// время которое ловушка находится в предупреждающем режиме
		float damage;				// повреждение которое наносит ловушка
		float m_fImmuneTime;		// время в течении которого не наносится повторный дамаг
	};

public:
	ThornPattern();
	~ThornPattern();

	MO_IS_FUNCTION(ThornPattern, MissionObject);

	// Инициализировать объект
	virtual bool Create(MOPReader & reader);
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);

	// Параметры
	IGMXScene* GetModel() {return m_pModel;}
	const Vector& GetDamageBoxSize() {return m_vDamageBoxSize;}
	const Vector& GetDamageBoxCenter() {return m_vDamageBoxCenter;}
	const Heights& GetHeights() {return m_heights;}
	const Sounds& GetSounds() {return m_sounds;}
	bool GetComplexityData(ComlexityData& dat);
	float GetActiveTime() {return m_fActiveTime;}	

protected:
	// описание уровня сложности
	struct Complexity
	{
		const char* name;
		ComlexityData dat;
	};

	// модель ловушки
	IGMXScene* m_pModel;

	// описание логики действия ловушки
	Heights m_heights;					// высоты положения ловушки
	array<Complexity> m_aComplexity;	// массив с уровнями сложности
	float m_fActiveTime;				// время которое ловушка находится в активном состоянии	
	Vector m_vDamageBoxSize;			// размер ящика наносящего дамаг
	Vector m_vDamageBoxCenter;			// центр ящика наносящего дамаг
	Sounds m_sounds;					// описание звуков

protected:
	void ReadMOPs(MOPReader & reader);
};

#endif
