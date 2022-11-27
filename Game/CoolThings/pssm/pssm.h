// ===========================================================
// Vano
// Parallel-Split Shadow Maps - попытка 
// ===========================================================

#pragma once

#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\ishadows.h"

struct RecvVertex
{
	Vector4 pos;
	Vector dir;
	float u, v;
};

class Frustum
{
public:
	Frustum();

	void SetView(const Matrix & view);
	void SetProjection(const Matrix & proj);
	void SetAspect(float aspect);
	void SetNearFarZ(float near, float far);

	const Plane * GetFrustum() const;
	const Vector4 * GetPoints()  const;

private:
	// m_points[].w = -1.0f for vmx dot product
	mutable __declspec(align(16)) Vector4 m_points[8];
	mutable __declspec(align(16)) Plane m_planes[6];
	Matrix m_view, m_proj;
	mutable Matrix m_invView;
	float m_aspect;
	float m_near, m_far;
	mutable bool needUpdate;

	void Update() const;
};

class PSSMHolder
{
private:
	void			Init();
	IRender			& Render() const { return *pRS; };

public:
	struct SCOData
	{
		Matrix			projectedMtx;			// 64
		//Vector4	projectedMin, projectedMax;	// 32
		MissionObject	* obj;					// 4
		MOF_EVENT		func;					// 4
		float			radius;
		float			distance2Light;			

		Matrix	mtx;							// 64
		Vector	vMin, vMax;						// 24
		float	fProjZMin, fProjZMax;			// 8

		bool	isVisible;						
		bool	isCharacter;
		bool	isInCamera;						// true if radius inside camera
		
		SCOData() : projectedMtx(true), mtx(true)
		{
		}

		static bool Sorter(const SCOData & s1, const SCOData & s2)
		{
			// если s1 ближе чем s2
			if (s1.distance2Light - s1.radius < s2.distance2Light - s2.radius)
				return false;

			// если s1 находится внутри s2 и камера находится в s2
			if (s2.isInCamera && (~(s2.mtx.pos - s1.mtx.pos)) + Sqr(s1.radius) <= Sqr(s2.radius))
				return false;

			return true;
		}
	};

	PSSMHolder();
	~PSSMHolder();

	bool	IsInited() const;
	void	AddRef();
	void	Release();

	IRender				* pRS;
	IGMXService			* pGeo;
	long				iRef;

	//IVariable	* pIViewProj;
	IVariable	* pVarTexParams;
	IVariable	* pIShadowMapMatrix;
	IVariable	* pIShadowMapTexture;
	IVariable	* pIZBias;
	IVariable	* pISplits;
	IVariable	* m_varCamPos;
	IVariable	* pIShadowColor;
	IVariable	* pIShading;
	IVariable	* pIShdK;
	IVariable	* pILightDir;
	IVariable	* pITexOffsets;
	IVariable	* pIShadowPower;
	IVariable	* varLinearZTex;
	//IVariable	* pILightView;
	IVariable	* pILightViewOnlyZ;
	//IVariable	* pITestUShift;

	IGMXService::RenderModeShaderId		PSSM_MT_CAST_id;

	IGMXService::RenderModeShaderId		PSSM_RECV_SOLID_id;

	IGMXService::RenderModeShaderId		PSSM_PROJ_CAST_id;

	IGMXService::RenderModeShaderId		PSSM_ST_CAST_id;
	IGMXService::RenderModeShaderId		PSSM_ST_CASTNPS_id;
	IGMXService::RenderModeShaderId		PSSM_ST_RECV_id;

	IGMXService::RenderModeShaderId		PSSM_ZPass_Recv_id;
	IGMXService::RenderModeShaderId		PSSM_ZPass_Recv_Alpha_id;

	ShaderId	zpass_id;
	ShaderId	zpasswire_id;
	ShaderId	defShade_id;

	int			m_projNum;
	char		* m_projBuffer;
	Vector4		* m_projMinMax;

	array<SCOData>	* aSCObjects;

	IShadowsService	* pShdServ;
};

class PSShadowMap : public MissionShadowCaster
{
private:
	enum 
	{
		vperm0167 = 80,
		vabs,
		vfrustum1,
		vfrustum2,
		vfrustum3,
		vfrustum4,
		vforce_dword = 0x7FFFFFFF
	};

	struct PntLight
	{
		ConstString		cName;
		float			fSize;	
		long			iPointLightHash;
		MOSafePointer	light; 
	};

	// статический референсный объект который держит все общие параметры теней
	static PSSMHolder PH;

	long m_numZPassPlanes;
	IVBuffer * m_zPassVB;
	IIBuffer * m_zPassIB;

	MGIterator	* pIterSRecv;

	float		fBlur;
	float		fBlurDistance, fBlurAngle;
	float		fShadowDistance;
	bool		bShowDebug;

	long			statDrawSplit[3];
	ProfileTimer	enumTimer, castTimer, recvTimer, castSplitTimer[3];

	Vector		vCamPos, vCamTarget, vCamUp;
	float		Splits[5];

	array<PntLight>		aPntLights;

	int			iNumRecvObjects;
	float		fMinReceiversY, fMaxReceiversY;
	string		sMinReceiver, sMaxReceiver;

	float		fNearZ;
	float		fMinVolumeY, fMaxVolumeY;
	float		fOldMinVolumeY, fOldMaxVolumeY;

	float		fSplitSchemeLambda;
	dword		shadowMapResolution;

	Color		cShadowColor;
	float		fShdPower, fCharacterShdPower;
	float		fShdK1;
	float		fShdK2;

	Vector		m_lightFarPosition;

	Vector		m_frustumTarget;
	float		m_frustumDiameter;

	Matrix		mLightView, mLightProj;
	Matrix		mCameraView, mCameraProj;
	Matrix		mCropView;

	// TEMPO
	Matrix		m_View, m_invView, m_Proj;
	// TEMPO

	float		fCameraFOV, fCameraAspect;
	float		fCameraNear;
	float		fCameraFar; 
	float		fCameraFarMax;

	Matrix		lightMtx;
	Vector		lightDir, lightSource, lightTarget;
	float		fLightNear;
	float		fLightFar; 
	float		fLightFarMax;
	float		fLightFOV;

	Vector		vp[8];
	Vector		vLightPos;
	float		fZBias, fCharacterZBias;

	long		iNumSplits;

	bool		bPointLight;
	bool		bErrorCreation;

	struct ClipPlane
	{
		Vector	pos;
		Vector	angles;
		Plane	plane;
	};

	array<ClipPlane>	m_clipPlanes;

	long		iNumRecv;
	dword		dwNumCharactersCasted;

	float		m_radius[16];

	Plane frustum[5];
	RENDERVIEWPORT	normalViewport;
	RENDERSCREEN	renderInfo;

	IS_XBOX(IRenderTargetDepth *, IRenderTarget *) GetZRenderTarget();

	// Вычисляем View и Projection матрицы для сплитов (и сразу записываем в шейдерные константы)
	void CalcViewProjForSplit(long iIndex, float fNear, float fFar);

	// Создаются стандартные View/Projection матрицы
	void CalculateViewProj(	Matrix & mView, Matrix & mProj, const Vector & vSource, const Vector & vTarget, 
							const Vector & vUpVector, float fFOV, float fNear, float fFar, float fAspect);

	// Computes corner points of a frustum
	void CalculateFrustumCorners(	Vector * pVPoints,
									const Vector & vCamSource, const Vector & vCamTarget, const Vector & vCamUp,
									float fNear, float fFar, float fFOV, float fAspect, float fScale);

	void CalculateLightForFrustum(long iIndex, Vector * pVCorners);
	void AdjustCameraPlanes();
	void CalculateSplitDistances();

	__forceinline bool SphereIsVisibleSquare(const Plane * frustum, const Vector4 & SphereCenter, float SphereRadius);
	//__forceinline bool SphereIsVisibleSquare(const Plane * frustum, const Vector & SphereCenter, float SphereRadius);
	bool BoxIsVisible(const Plane * frustum, const Vector & vMin, const Vector & vMax);

	void CalcProjection(const Matrix & mShadowView, Matrix & mShadowProjection, const Vector & min, const Vector & max);

	void UpdateVariables();

	void SetupXBOXFrustumRegisters();
	void CalculateCastObjectParameters();
	void CalculateMinMaxReceiversY();
	void ShadowMap_Cast(int split);
	void Recv_ShadowPass(int split);
	void Recv_ZPrePass();
	void ShadowMap_Recv_Clear();

protected:
	virtual void AddShadowCastObject(MissionObject * obj, MOF_EVENT func, const Vector & min, const Vector & max);

public:
	//Конструктор - деструктор
	PSShadowMap();
	virtual ~PSShadowMap();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);
	//Активирование/деактивирование объекта
	virtual void Activate(bool isActive);

	MO_IS_FUNCTION(PSShadowMap, MissionShadowCaster);

	void _cdecl Realize(float fDeltaTime, long level);
	void _cdecl ClearScreen(float fDeltaTime, long level);
#ifndef STOP_DEBUG
	void _cdecl DebugRealize(float fDeltaTime, long level);
#endif	

#ifndef NO_CONSOLE
	void _cdecl Console_Parameters(const ConsoleStack & params);
#endif

	dword dwId0;
	dword dwId1;
	dword dwId2;
	dword dwId3;
	dword dwId4;
	dword dwId5;
	dword dwId6;


	void _cdecl Begin0(float fDeltaTime, long level);
	void _cdecl End0(float fDeltaTime, long level);

	void _cdecl Begin1(float fDeltaTime, long level);
	void _cdecl End1(float fDeltaTime, long level);

	void _cdecl Begin2(float fDeltaTime, long level);
	void _cdecl End2(float fDeltaTime, long level);

	void _cdecl Begin3(float fDeltaTime, long level);
	void _cdecl End3(float fDeltaTime, long level);

	void _cdecl Begin4(float fDeltaTime, long level);
	void _cdecl End4(float fDeltaTime, long level);

	void RegisterUnregisterPixEvents ();

	Matrix tmpMtx;
	Vector tmpMin, tmpMax;
	Frustum tmpFrustum;

	//void OverlapTest();
};