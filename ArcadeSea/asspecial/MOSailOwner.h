#pragma once

#include "..\..\Common_h\Mission.h"
#include "..\ships\ShipPart.h"
#include "..\ships\Sail.h"

// корабль, лодка

class IClothBurns;
class MissionSailOwner;

// хозяин паруса
class MOSailOwner : public SailOwner
{
	MissionSailOwner* m_pMO;
public:
	MOSailOwner();

	void SetMO(MissionSailOwner* pMO);

	virtual IPhysCombined* GetPhysView();
	virtual IPhysCloth* CreateCloth(const char* file, long line, IClothRenderInfo & renderInfo, IClothMeshBuilder & builder, IPhysCloth::SimulationData & simData);
	virtual bool EditMode_IsOn();
	virtual Matrix& GetMatrix(Matrix& mtx);
	virtual float GetAlpha() {return 1.f;}
	virtual float GetLastDeltaTime();
	virtual IClothBurns* GetClothBurns();
	virtual void PartDestroyFlagSet(DWORD dwFlag) {}
	virtual const char* GetSoundNameForEvent(dword dwIdx);
	virtual void AcceptVisitor(IPartVisitor& clsf);
	virtual bool TieToPhysicBody(Sail* pSail,bool bTie,bool bTop) {return false;}
};

// описание парусов
class SailsPattern : public MissionObject
{
public:
	// описание паруса
	struct SailPattern
	{
		Sail::Params param;

		// готовый парус
		IClothMeshBuilder* cooked;
	};

protected:
	array<SailPattern> m_aSailPattern;

public:
	SailsPattern() : m_aSailPattern(_FL_) {}
	~SailsPattern();

	// Создание объекта
	virtual bool Create(MOPReader & reader);
	// Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);
	// Отрисовка в режиме редактора
	void _cdecl EditModeDraw(float fDeltaTime, long level);

	// получить паттерн для паруса по его индексу
	const SailPattern* GetPattern(dword n) { if( n<m_aSailPattern.Size() ) return &m_aSailPattern[n]; return null; }

	MO_IS_FUNCTION(SailsPattern, MissionObject);


private:
	// построить парус
	void BuildSail(dword index);
	// читать параметры
	void ReadMOPs(MOPReader & reader);
	// убить паруса
	void ReleaseAllSails();
};

// миссионный объект - хозяин парусов
class MissionSailOwner : public DamageReceiver
{
	struct SailDescribe
	{
		Sail* sail;			// парус
		dword idx;			// номер паруса в паттерне
		long nBoxTop;		// физический бокс (верх) - к нему крепится парус
		long nBoxBottom;	// физический бокс (низ) - к нему крепится парус
		Matrix mtxSail;		// матрица текущего положения паруса (при входе в спящий режим)
	};

protected:
	// хозяин парусов
	MOSailOwner		m_MOSailOwner;

	// список парусов
	array<SailDescribe>	m_aSails;

	// объект горения парусов
	MOSafePointerType<IClothBurns>	m_pCBurns;

	// паттерн парусов
	ConstString		m_pcSailPatternName;
	SailsPattern*	m_pSailPattern;

	// объект к которому привязаны паруса
	ConstString		m_pcConnectToName;
	MOSafePointer	m_pConnectTo;

	// положение группы парусов на этом кадре
	Matrix			m_mtxTransform;

	float			m_fLastDeltaTime;	// дельта тайм на этом кадре
	float			m_fAlpha;			// непрозрачность
	ConstString		m_pcTearSound;		// имя звука разрываемого паруса

	// объект для QT поиска
	IMissionQTObject*	m_pFinder;
	Vector m_vFinderMin;
	Vector m_vFinderMax;

	// физический объект, к которому привязываются все паруса
	IPhysCombined*	m_pRigidBody;

	bool	m_bNoSwing;
	bool	m_bReflection;
	bool	m_bRefraction;
	bool	m_bFakeSails;
	bool	m_bVisible;
	bool	m_bActive;

public:
	//////////////////////////////////////////////////////////////////////////
	// Конструктор - деструктор
	//////////////////////////////////////////////////////////////////////////
	MissionSailOwner();
	virtual ~MissionSailOwner();

	//////////////////////////////////////////////////////////////////////////
	// Работа на кадре
	//////////////////////////////////////////////////////////////////////////
	void _cdecl Work(float fDeltaTime, long level);
	void _cdecl DrawRefl(const char*, MissionObject*);
	void _cdecl DrawRefr(const char*, MissionObject*);

	//////////////////////////////////////////////////////////////////////////
	// Методы для функционирования хозяина паруса
	//////////////////////////////////////////////////////////////////////////
	// непрозрачность паруса
	float GetAlpha() const { return m_fAlpha; }
	// дельта тайм для этого кадра
	float GetLastDeltaTime() {return m_fLastDeltaTime;}
	// получить имя звука для звукового евента
	const ConstString & GetSoundNameForEvent(dword dwIdx) {return m_pcTearSound;}
	// объект управляющий горением парусов
	IClothBurns* GetClothBurns() {return m_pCBurns.Ptr();}
	// посещение визитера (для получения списка парусов)
	void Accept(IPartVisitor& visitor);
	// получить физическое тело
	IPhysCombined* GetPhysBody() {return m_pRigidBody;}

	//////////////////////////////////////////////////////////////////////////
	// DamageReceiver
	//////////////////////////////////////////////////////////////////////////
	//Воздействовать на объект сферой
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius);
	//Воздействовать на объект линией
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to);
	//Воздействовать на объект выпуклым чехырёхугольником
	virtual bool Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4]);

	//////////////////////////////////////////////////////////////////////////
	// MissionObject
	//////////////////////////////////////////////////////////////////////////
	// Создание объекта
	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();
	// Рестарт объекта
	virtual void Restart();
	// Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);
	// Получение матрицы трансформации
	virtual Matrix & GetMatrix(Matrix & mtx);
	// размеры бокса
	virtual void GetBox(Vector & min, Vector & max);
	// Включить показ объекта
	virtual void Show(bool isShow);
	// Включить активность объект
	virtual void Activate(bool isActive);
	// Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);
	// Проверка типа
	MO_IS_FUNCTION(MissionSailOwner, MissionObject);


private:
	// чтение параметров
	void ReadMOPs(MOPReader & reader);
	// убить внутренние ресурсы
	void ReleaseAll();
	// сделать парус фейковым или физическим
	void SetFake(long idx,bool bFake);
};
