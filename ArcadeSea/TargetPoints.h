#ifndef _TargetPoints_h_
#define _TargetPoints_h_

#include "..\Common_h\Mission\Mission.h"

class IExplosionPatch;

class TargetPoints : public MissionObject
{
	enum TargetPointState
	{
		tps_disable,
		tps_prepare,
		tps_showdelay
	};

	struct TargetPointVertex
	{
		Vector pos;
		dword col;
		float u,v;
		float u2,v2;
	};

	struct RenderParams
	{
		// readable params
		ConstString texturename;
		float pointradius;
		float scalePeriod;
		float scaleAmplitude;
		dword colPrepare;
		float startSize;
		dword startColor;
		dword finishColor;

		// created params
		IBaseTexture* pTexture;
		IVBuffer* pVertex;
		IIBuffer* pIndex;
		ShaderId idShader;
		IVariable* pShVarTexture;
		IVariable* pShVarCol;

		dword pointQMax;
		dword pointQPrepare;
	};

	struct LogicParams
	{
		// readable params
		float minShowDelay;
		float maxShowDelay;
		float activeRadius;
		ConstString watchObjectID;
		float cooldawn;
		long pointsLimit;
		float minFlyTime;
		float maxFlyTime;
		float minFlyHeight;
		float maxFlyHeight;

		// created params
		MOSafePointer watchObject;
	};

	struct HitParams
	{
		// readable params
		float explodeRadius;
		float explodeDamage;
		float explodePower;
		ConstString hitSFX;
		float hitSFXScale;
		ConstString hitSound;
	};

	struct PointData
	{
		// позиция
		Vector pos;
		// состояние
		TargetPointState state;
		// время
		float maxFlyTime;
		float curFlyTime;
	};

	// набор геометрий для взрыва
	struct GeometryData
	{
		IGMXScene* gmxModel;
		float modelLeftTime;
		Matrix mtxPos;
	};

	struct EditorParams
	{
		bool isShowPoints;
	};

//--------------------------------------------------------------------------------------------
public:
	TargetPoints();
	virtual ~TargetPoints();

	//Инициализировать объект
	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();
	//Изменение параметров в редакторе
	virtual bool EditMode_Update(MOPReader & reader);
	//Активировать
	virtual void Activate(bool isActive);

	// получить массив точек для попадания (возвращает количество точек)
	long GetPointsArray(long shootQuantity, array<long>& points);
	const Vector & GetPointPos(long n);
	void ResetPoint(long n);
	void BorrowPoint(long n, float flyTime);

	float GetCooldawn() {return m_logicParams.cooldawn;}

	static TargetPoints* GetNextTargetPoint(IMission* pMission);
	static void BorrowTargetPoints(TargetPoints* pTargPoints, MissionObject* pObj);

	void GetTimeHeightLimit(float & fTimeLimit, float & fHeightLimit);

	void MakePointDamage(long n);

	void SetSrcPosition(const Vector & pos) { m_vSrcPosition = pos; }

//--------------------------------------------------------------------------------------------
private:
	//Работа в режиме игры
	void _cdecl WorkGame(float dltTime, long level);

	//Работа в режиме редактора
	void _cdecl WorkEdit(float dltTime, long level);

	//Пересоздать объект
	virtual void Restart()
	{
		ReCreate();
	};

	//Зачитать параметры
	void ReadMOPs(MOPReader & reader);

	// обновить точки
	void UpdatePointsVertex(float dltTime);
	// нарисовать точки
	void DrawPoints();

	// установить задержку на рисование
	void InitShowDelay();

	// работа с массивом геометрии
	void RenderModels(float fDeltaTime);

//--------------------------------------------------------------------------------------------
protected:
	RenderParams m_renderParams;

	LogicParams m_logicParams;

	HitParams m_hitParams;

	EditorParams m_editorParams;

	// target points data
	array<PointData> m_points;

	static array<TargetPoints*> g_targObjectsList;
	static long g_nTargObjIndex;
	static float g_fCooldawn;
	static TargetPoints* g_pLastTargetPoints;

	MOSafePointer m_arbiter;
	MOSafePointer m_seaMisParams;

	IExplosionPatch* m_ExplosionPatch;
	long m_HitGroupIndex;

	MOSafePointer m_ShooterKickOut;
	bool m_bWaitKickOutPrepair;
	bool m_bWaitKickOut;
	Vector m_vSrcPosition;
};

#endif
