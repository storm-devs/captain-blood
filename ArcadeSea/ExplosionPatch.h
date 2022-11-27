#ifndef _ExplosionPatch_h_
#define _ExplosionPatch_h_

#include "..\Common_h\IExplosionPatch.h"

class ExplosionPatchCollider : public MissionObject
{
public:
	ExplosionPatchCollider();
	~ExplosionPatchCollider();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();
	//Изменение параметров в редакторе
	virtual bool EditMode_Update(MOPReader & reader);
	//Активировать
	virtual void Activate(bool isActive);

protected:
	void ReadMOPs(MOPReader & reader);

	//Работа в режиме редактора
	void _cdecl WorkEdit(float dltTime, long level);

	IGMXScene* m_pPatchGMX;
	IPhysRigidBody* m_pPatchCollider;
	Matrix m_mtxPatchTransform;

	bool m_bActivateByCreate;
};

class ExplosionPatch : public IExplosionPatch
{
	struct HitParams
	{
		dword explosionMaterial;

		ConstString modelName;
		ConstString modelAni;
		float modelTimeout;
		float modelScale;
		bool isDeletedByTimeout;
		bool isDynamicLight;
		bool isShadowReceive;
		bool isShadowCast;
	};

	struct ReadyExplosion
	{
		Vector pos;
		long group;
		long material;
	};

	struct HitGroup
	{
		ConstString type;
		array<HitParams> hits;

		HitGroup() : hits(_FL_) {}
	};

	enum ExplosionState
	{
		est_free,
		est_active,
		est_ready_to_delete,
		est_fadeout
	};

	// набор геометрий для взрыва
	struct GeometryData
	{
		ExplosionState state;
		float timer;

		IGMXScene* gmxModel;
		Matrix mtxPos;
		bool isShadowCast;
		bool isDeletedByQuantityLimit;
	};

//--------------------------------------------------------------------------------------------
public:
	ExplosionPatch();
	virtual ~ExplosionPatch();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();
	//Изменение параметров в редакторе
	virtual bool EditMode_Update(MOPReader & reader);

	//Обработчик команд для объекта
	virtual void Command(const char * id, dword numParams, const char ** params);

	// найти группу взрывов по заданому имени
	virtual long GetHitGroup(const char* groupName);
	// создать взрыв в точке (группа задана через ее индекс)
	virtual void MakeExplosion(const Vector& pos, long nHitGroupIndex);
	// создать взрыв в точке (группа задана через ее имя)
	virtual void MakeExplosion(const Vector& pos, const char* groupName);
	// создать взрыв в точке (группа задана через ее имя)
	virtual void MakeExplosion(const Vector& pos, const ConstString & groupName);
	// очистить все эффекты
	virtual void ClearExplosions();

//--------------------------------------------------------------------------------------------
private:
	//Работа в режиме игры
	void _cdecl WorkGame(float dltTime, long level);

	// рисование теней
	void _cdecl ShadowDraw ();

	// рисование переломления
	void _cdecl SeaRefraction ();

	//Работа в режиме редактора
	void _cdecl WorkEdit(float dltTime, long level);

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

	//Зачитать параметры
	void ReadMOPs(MOPReader & reader);

	// работа с массивом геометрии
	long BorrowModel( const Vector& pos, const HitParams& hitParams );
	void FreeModel( long idxModel );
	void GoToNextState( long idxModel );
	void RenderModels( bool shadowCast );
	void UpdateModels( float dltTime );
	void PreloadModel( ConstString& gmxName, ConstString& aniName );
	void ReleaseModels();
	void ReleasePreloadModels();

	// найти параметры взрыва по материалу попадания
	long FindHitParams(const HitGroup& hitGroup, dword dwExplosionMaterial);

	// родить взрыв с заданными параметрами
	void MakeExplosion(const Vector& pos, long nHitGroupIndex, dword dwExplosionMaterial);

	// получить материал по его ID
	dword GetExplodeMaterial( ConstString& matID );

//--------------------------------------------------------------------------------------------
protected:
	// параметры попадания
	array<HitGroup> m_hitParams;

	// набор моделей для отрисовки
	array<GeometryData> m_models;

	// список взрывов для рождения
	array<ReadyExplosion> m_aReadyExplosions;

	// набор предзагруженных геометрий для взрыва
	array<IGMXScene*> m_aGmxPreload;

	// орбитр персонажей, для рождения взрыва
	MOSafePointer m_arbiter;

	// флаг того, что какая то из моджелей отбрасывает тень
	bool m_bIsShadowCast;

	// лимит на количество одновременных взрывов
	long m_nExplosionLimit;
	// лимит, после которого, надо удалять старые взрывы
	long m_nExplosionDeleteLimit;
	// время фейда
	float m_fModelFadeTime;
	// мощность фейда
	float m_fModelFadePower;
	// дистанция до нижней точки трейса через патч
	float m_fTraceBottom;
	// дистанция до верхней точки трейса через патч
	float m_fTraceTop;

	bool m_bTurnOnDebugTrace;

	long m_nLevelDraw;

	bool m_bRefraction;

	CritSection m_critSection;
};

#endif
