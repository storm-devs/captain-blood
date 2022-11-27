//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// Character
//===========================================================================================================================
// SwordTrail
//===========================================================================================================================

#ifndef _SwordTrail_h_
#define _SwordTrail_h_

#include "..\..\..\Common_h\mission.h"
#include "..\..\auxiliary objects\EffectTable\EffectTable.h"

class Character;

class SwordTrail
{
	struct Element
	{
		Vector s;
		Vector e;
		float t;
		float a;
	};
	
	struct Vertex
	{
		Vector p;
		dword c;
		float u, v;
	};

	Element last_elem;	

//--------------------------------------------------------------------------------------------
//Управление следом
//--------------------------------------------------------------------------------------------
public:
	EffectTable* effectTable;

	SwordTrail();
	virtual ~SwordTrail();

	//Инициализировать
	bool Init(Character * _chr, IGMXScene * model, dword color, const char * uniqTexture, const char * technique, float trailPower);
	//Освободить ресурсы
	void Release();
	//Начать слежение за локаторами
	void StartTrace();
	//Закончить слежение за локаторами
	void StopTrace();
	//Рисование, обновление следа
	void Draw(const Matrix & toWorld, const Matrix & itemPosition, float dltTime, bool lostPosition);
	//Обновление следа без рисования
	void Update(const Matrix & itemPosition, float dltTime, bool lostPosition);

	bool IsActive() { return isStartTrace; };

	bool IsTrailNotEmpty() { return trail > 1; } 
//--------------------------------------------------------------------------------------------
//Имплементация
//--------------------------------------------------------------------------------------------
protected:
	//Обновить след
	void UpdateTrail(float dltTime);
	//Разделить первый элемент, если требуется
	void SplitTrail(bool isForceSplit);
	//Вставить элемент между 0 и 1 в указанную относительную позицию
	void InsertElement(float pos);
	//Установить модельку
	bool SetModel(IGMXScene * model);

//--------------------------------------------------------------------------------------------
protected:
	array<Element> trail;			// Массив элементов следа
	IGMXScene * scene;
	GMXHANDLE lstart, lend;			// Локаторы, за которыми надо следить
	float splitTime;				// Время с последнего разбивания
	float releaseTime;				// Время с отключения следа
	bool isStartTrace;				// Разрешён ли процес трейса позиций
	Character * chr;				// Шлейф был инициирован
	IBaseTexture * localTexture;	// Собственная текстура следа
	ShaderId shader_id;				// Текущий шейдер для данного трейла
	float trailPower;				// Множитель цвета

	// Цвет следа
	dword color;

	//Счётчик шлейфов
	static dword counter;
	//Общая текстура
	static IBaseTexture * texture;
	//Общий буфер вершин
	static IVBuffer * vbuffer;

	static ShaderId techBlend_id, techAdd_id, techMultiply_id;
};

#endif
